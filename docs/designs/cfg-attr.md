# Attribute Grammars for Chef

Author: Justin Bassett<br> Date: 2022-02-02

# Summary

An exploration into how to turn the CFG implementation from a `bool` parse
function into a parser than can synthesize types (an AST).

# Background

Currently, the CFG specification in Chef is only a formal grammar, meaning it
allows us to determine whether a given string parses and is in the language the
grammar represents. But this is not sufficient. We want to be able to actually
parse an AST from the string. This is where [Attribute Grammars][WIKI] come in.
The idea is to annotate the grammar with attributes describing what to do when
a rule is taken. A good example of this is Bison, where the attributes can be
any code, allowing for the construction of an AST or even just performing
syntax directed translation (no AST, immediately evaluate based on the parse
rules). For the context of this document, that kind of flexibility is not a
requirement; it is only required to have some way to generate an AST from the
grammar. This AST can even be prescribed by Chef, at least for the first
iteration.

## Existing implementations

Many tools already exist for parsing, so there are many ways that this is
already accomplished. What are these ways?

### Recursive descent

Synthesizing an AST with a hand-rolled recursive descent parsing implementation
is natural and straightforward. Being hand-rolled, the AST and the parsing
functions are defined by the author. The grammar structure is translated into
the recursive descent implementation, and each function corresponds to a rule
in the grammar, possibly with some transformations to allow the parser to be
LL(1). Each function does not return `bool`, but returns something morally
equivalent to `std::optional<AstNode>`. Given this, it is straightforward to
define the AST synthesis for any given rule: the function simply combines the
nodes from its constituent rules into the new node combining all this
information.

Example:

```c++
// Grammar; no whitespace allowed:
// Add -> int '+' int
// int -> /* an integer */

struct AddNode {
	int first;
	int second;
};

std::optional<std::pair<int, std::string_view>> parse_int_literal(std::string_view input) {
	int result;
	const auto parse_result = std::from_chars(input.data(), input.data() + input.size(), &result);
	if (parse_result.ec != std::errc()) return std::nullopt;
	return std::pair{
		result,
		std::string_view(parse_result.ptr, input.data() + input.size()),
	};
}

std::optional<std::pair<AddNode, std::string_view>> parse_add_node(std::string_view input) {
	if (const auto first = parse_int_literal(input)) {
		input = first->second;
		if (input.starts_with('+')) {
			input.remove_prefix(1);
			if (const auto second = parse_int_literal(input)) {
				return std::pair{
					AddNode{first->first, second->first},
					second->second,
				};
			}
		}
	}
	return std::nullopt;
}
```

### Bison

[Bison][Bison] is a parser generator tool.

Example (psuedocode):

```flex
"+" return '+';
0|[1-9][0-9]* 	yylval.num = parse_int(yytext); return NUM;
```

```bison
Add: NUM '+' NUM	{ $$ = AddNode{ $1, $2 }; }
// Or it could evaluate immediately:
Add: NUM '+' NUM	{ $$ = $1 + $2; }
```

### Boost.Spirit

[Boost.Spirit X3][Boost.Spirit.X3] is a parser combinator library. This means
that each individual rule is a parser, and each parser has an attribute, which
is the type that the parser produces. The sequence combinator, `a >> b`, simply
combines the two attributes of the sub-parsers into a `tuple<A, B>`. Similarly
the alternative combinator, `a | b` combines the two attributes into a
`variant<A, B>`. There are some smarts for the `tuple<A, B>` case which means
that, through Boost.Fusion, any tuple-like thing can be used, including regular
structs exposed to Boost.Fusion. There are also some smarts for attribute-less
parsers, e.g. constant strings such as `"while"` would have no attribute.

However, Spirit also has something similar to Bison's attributes, which it
calls "Parser Semantic Actions." These actions allow performing almost any
action if the given parser successfully parses, via calling the supplied
lambda.

Example (psuedocode):

```c++
// Parsing to an AST:
struct AddNode {
	int first;
	int second;
};
BOOST_FUSION_ADAPT_STRUCT(::AddNode, first, second);

namespace parser {
	namespace x3 = boost::spirit::x3;
	using addnode_type = x3::rule<class addnode, ::AddNode>;
	// For declaring functions, etc. in header:
	BOOST_SPIRIT_DECLARE(addnode_type);

	// For the actual definition:
	const addnode_type addnode = "addnode";
	const auto addnode_def = x3::double_ >> '+' >> x3::double_;
	BOOST_SPIRIT_DEFINE(addnode);

	// Maybe a bit more to actually finish instantiating the parser.
}

template <typename Iter>
bool parse_add_node(Iter first, Iter last, AddNode& result) {
	n = 0.0;
	const bool r = x3::phrase_parse(first, last, parser::addnode, space);

	if (first != last) return false; // to require full consumption.
	return r;
}

// Immediate evaluation example:
// Adapted from https://www.boost.org/doc/libs/1_78_0/libs/spirit/doc/x3/html/spirit_x3/tutorials/sum___adding_numbers.html:
template <typename Iter>
bool adder(Iter first, Iter last, double& result) {
	n = 0.0;
	const bool r = x3::phrase_parse(first, last,
		// Grammar
		(
			// Even better than just x+y, this enables an arbitrary number of `+`s.
			double_[ [&](auto& ctx) { result += _attr(ctx); } ] % '+'
		)
		, space);

	if (first != last) return false; // to require full consumption.
	return r;
}
```

### ANTLR

[ANTLR][ANTLR] is a parser generator tool.

I don't have too strong of an understanding of ANTLR. But ANTLR works by
generating a parse tree which mirrors the input grammar. This parse tree is
walked by some tree walker, where a visitor can be used to make the conversion
to whatever final AST desired or by performing actions on the fly. I will note
that if the walker API is all that's being used, it would be unnecessary for
ANTLR to actually hold the whole parse tree in memory, but it can instead call
the visitor on the fly. That may or may not be desirable. ANTLR also has
attributes similarly to Bison where you can run arbitrary code in response to a
rule being parsed, but this method is discouraged.

So to summarize, ANTLR allows for an API such as Bison, but it also allows for
an API with a tree visitor that has `enterFooNode()` and `exitFooNode()`
functions for each parse tree node.

## Rule of Chiel

The [Rule of Chiel][Rule.Chiel] is important to understand so that it can be
kept in mind while pondering the design, otherwise we risk very slow to compile
code. That said, a lot of compile time programming cost analysis is based on an
assumption of amortization: if we use this thing many times, how expensive is
it? But in our case, we expect that there would be approximately one or two
grammars per translation unit if any at all, so there is nothing to amortize
over full grammar instantiations.

The Rule of Chiel states that these are in descending order of cost (the (~123)
numbers are cost estimates, but these are more order-of-magnitude rather than
actual values):

1. SFINAE. (~500)
2. Function template instantiation. (~200)
3. Class template instantiation. (~100)
4. Evaluating an alias. (~5)
5. Adding an additional parameter to a class template argument list. (~3)
6. Adding an additional parameter to an alias. (~2)
7. Memoized class template lookup. (~1)

# Requirements

Chef is intended to be fully `constexpr` at some point in its life. However, it
is also intended to have runtime ability, because runtime tools to aide in
grammar development is one very nice thing about a parser generator.
Additionally, Chef wants to be fast. The chosen direction should ideally not
inhibit compiler optimization for the `constexpr` variant. If Bison-style
actions are possible and these are encoded in a type-erased
`std::function`-like manner to allow for `constexpr` and runtime variants to
both work, that better not be the only method. But if Boost.Spirit style "rules
have attributes" works for most cases, with such a type-erased action for only
a small number of cases, that would probably be okay.

With that in mind, whatever solution that Chef chooses should meet these
requirements:

- (eventually) `constexpr` friendly, at least once the C++ standard and
  implementations catch up to a point where it can be implemented in such a
  manner.
- Runtime friendly. This means that the parser must be able to be generated at
  runtime, so no encoding everything in big templated expression templates.
  However, if the templated expression templates really is the best way for the
  `constexpr` version, it can be okay if the runtime version still works very
  well (e.g. uses opt-in type erasure through the expression templates).
- Possible to be fast. The initial design does not need to be fast, but there
  should be room in the design to make the parser very fast.
- Flexible in language. I think this requires both Bison-style actions and also
  some kind of conditional guards to enable context-sensitive languages such as
  C++ to be parsed. That said, this flexibility can require some pain to get
  working. LL(1) or LR(1) languages are definitely the priority.
- Flexible in output. If the parser can parse directly to many custom ASTs, I'm
  fond of that idea. I don't really like having a strict requirement that only
  some specific rigid tree is allowed. However, it's definitely simpler if the
  model is to generate a parse tree (less obvious punctuation), and only then
  translate it into the AST, so I'm happy either way. That is, whether it can
  parse directly to "any" user AST or whether it first has to go to a parse
  tree that can then be straightforwardly translated to "any" user AST, I'm
  happy with that.

# Design

At this point, based on the requirements and the existing implementations, it
seems that the nicest way is to give rules an "attribute" which can be used to
be combined into the parent rule's attribute. This is the technique used by
recursive descent parsing, Boost.Spirit, and ANTLR. For flexibility, it would
be nice if arbitrary actions can also be taken, Bison-style (but also supported
by Boost.Spirit and ANTLR). Precisely how these attributes get defined and how
they get translated to the user AST is what the rest of the design should
concern with.

## Constexpr

To support `constexpr`, that's not too difficult. To support `constexpr` and
custom types and all that, that's much more difficult. There are only a few
general ways I can think of to do this: never lose type information, tracing
type information back to the type-erased data tables, and code generation from
the type-erased data tables.

### Constexpr: keeping all type information

This has build performance issues. `constexpr`/`consteval` programming is quite
fast, but template instantiations are quite slow\*. Keeping all type
information and propagating that through the parsing algorithms would certainly
work. However, this would likely mean lots of template instantiations for each
rule or even subrule in the grammar. Furthermore, since each rule would have
type information on it, it would no longer be possible to do simple things such
as `for (const auto& rule : rules)`, meaning that the style would have to
change as well to being more TMP-like functional programming.

For these reasons, this approach is not the direction to go.

\*See the [Rule of Chiel][Rule.Chiel]. Of note is WG21's direction on
compile-time programming: `consteval` and `constexpr`. I've also done my own
performance testing, where I found that `constexpr` programming is much faster,
after accounting for a slight constant overhead.

### Constexpr: tracing to erased information

In this style, the basic `constexpr` support is most of what's needed. The
grammar and whatnot would be fed into the algorithms, and we get the data table
back. All that remains is to track the type information back to these data
tables. Thus, there would need to be some tracing information for the tables,
telling us what parts of the tables map to what rules of the grammar so that
the type information can kick in when executing the table.

Note that the actual evaluation of the parsing algorithm would be different
from the fully erased data table, because the evaluation needs this type
information. However, for the computation of the tables, the type information
can be erased.

### Constexpr: code generation

This is a very similar concept to _tracing to erased information_, except
instead of tracing, the type information would be injected as erased "strings"
into the data table information or around the data table information. Then, a
code generation step can take those strings and generate the actual parsing
code, complete with type information.

This has the same note as before: requires a unique parsing evaluation
algorithm.

This either requires strong reflection (definitely no earlier than C++26,
probably later than that), or that the parser generating happens before compile
time. Because of this, I do not think this approach is the direction to go.

## The Dream

The dream interface for Chef is to have the grammar as a raw string literal,
with hooks into C++ via reflection or calculated at instantiation time:

```c++
constexpr auto grammar = R"chef(
	Add -> int '+' int;
)chef"_grammar;

constexpr auto parser = chef::ll1_parser(grammar);

// Later:
struct AddNode {
	int first;
	int second;
};
AddNode add; // Or something to avoid requiring default construction.
if (parser(input, &add)) // use `add`.
```

In other words, the ideal is for all the type information to come from the
actual invocation of the parser, similar to Boost.Spirit. This might not be
feasible, because:

1. There can be ambiguity.
2. Thinking of all data types as tuples is problematic.
3. This constrains the parse tree / first AST to POD types.

It's probably necessary to annotate the rules with types to help resolve the
ambiguity.

In the parser side, rather than prescribing an AST, ideally it would be
possible to pass a visitor, such as ANTLR's method, without requiring
constructing an actual AST. This is probably more complicated, so this will be
deferred.

```c++
// As an alternative to the raw string literal, an interface such as this can
// be provided.
constexpr auto grammar = chef::grammar(
	chef::rule<"Add">("int '+' int"),
);

// Otherwise, the raw string grammar is a bit more readable, and it better
// enables interaction with runtime parser generator tools (copy-paste the CFG
// from the tools into the source code).
constexpr auto grammar = R"chef(
	Add -> Int '+' Int;
	Int -> int
)chef"_grammar;

// Annotate the type information when creating the parser().
constexpr auto parser = chef::ll1_parser(grammar,
	chef::bind<"Add", AddNode>,
	chef::bind<"Int", int>([](auto input) -> int { ... }); // Bison-style attributes.
```

## The easiest-to-implement starting design

Before doing anything complicated, let's start with the simplest possible
style. Given that everything is currently runtime-only, I'm going to go with a
runtime AST, and only a single kind of AST for every grammar. This should be
the most straightforward thing to implement on the current state of the code,
and it will also be necessary for any runtime parser generator tools.

```c++
// Something like:
struct ast_node;

struct sequence_node {
	std::vector<ast_node> children;
};

struct ast_node {
	std::string name;
	std::variant<sequence_node, /* other node types */> value;
};
```

# References

- Wikipedia: [Attribute grammar][WIKI].
- [Bison parser generator documentation][Bison].
- [Boost.Spirit X3 library documentation][Boost.Spirit.X3].
- [ANTLR documentation][ANTLR].
- [Rule of Chiel][Rule.Chiel].

  [WIKI]: https://en.wikipedia.org/wiki/Attribute_grammar
  [Bison]: https://www.gnu.org/software/bison/manual/html_node/index.html
  [Boost.Spirit.X3]: https://www.boost.org/doc/libs/1_78_0/libs/spirit/doc/x3/html/index.html
  [ANTLR]: https://www.antlr.org/
  [Rule.Chiel]: https://youtu.be/EtU4RDCCsiU?t=491
