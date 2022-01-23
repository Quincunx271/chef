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

TODO: research ANTLR.

# References

- Wikipedia: [Attribute grammar][WIKI].
- [Bison parser generator documentation][Bison].
- [Boost.Spirit X3 library documentation][Boost.Spirit.X3].

  [WIKI]: https://en.wikipedia.org/wiki/Attribute_grammar
  [Bison]: https://www.gnu.org/software/bison/manual/html_node/index.html
  [Boost.Spirit.X3]: https://www.boost.org/doc/libs/1_78_0/libs/spirit/doc/x3/html/index.html
