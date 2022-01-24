# Attribute Grammars for Chef

Author: Justin Bassett<br> Date: 2022-01-23

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

# References

- Wikipedia: [Attribute grammar][WIKI].
- [Bison parser generator documentation][Bison].
- [Boost.Spirit X3 library documentation][Boost.Spirit.X3].
- [ANTLR documentation][ANTLR].

  [WIKI]: https://en.wikipedia.org/wiki/Attribute_grammar
  [Bison]: https://www.gnu.org/software/bison/manual/html_node/index.html
  [Boost.Spirit.X3]: https://www.boost.org/doc/libs/1_78_0/libs/spirit/doc/x3/html/index.html
  [ANTLR]: https://www.antlr.org/
