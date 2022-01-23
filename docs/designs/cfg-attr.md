# Attribute Grammars for Chef

Author: Justin Bassett<br>
Date: 2022-01-23

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

# References

- Wikipedia: [Attribute grammar][WIKI].

  [WIKI]: https://en.wikipedia.org/wiki/Attribute_grammar
