Looking at this code snippet, it appears to be part of a C/C++ compiler or parser's error handling code. Here's what I can tell:

## Context
This is likely from a compiler's source code (possibly GCC) where `RT_` constants represent different token types or expected constructs during parsing. When the parser encounters an unexpected token, it generates an error message indicating what was expected instead.

## Code Structure
- Each `case` handles a different expected token type
- `gmsgid` stores a translatable error message string
- `G_()` is a macro for marking strings for internationalization (gettext)
- `%<` and `%>` are special markers that likely format the enclosed text (e.g., as code or quotes)

## Token Types
- `RT_EXTERN`: Expected `extern` keyword
- `RT_STATIC_ASSERT`: Expected `static_assert` keyword  
- `RT_DECLTYPE`: Expected `decltype` keyword
- `RT_OPERATOR`: Expected `operator` keyword
- `RT_CLASS`: Expected `class` keyword
- `RT_TEMPLATE`: Expected `template` keyword
- `RT_NAMESPACE`: Expected `namespace` keyword
- `RT_USING`: Expected `using` keyword
- `RT_ASM`: Expected `asm` keyword
- `RT_TRY`: Expected `try` keyword

## Purpose
This code generates user-friendly error messages when the parser expects a specific C++ keyword but encounters something else. The messages will be displayed to developers when they have syntax errors in their code.

The `%<` and `%>` markers likely cause the enclosed text to be displayed in a distinctive way (like quotes or monospace font) to clearly indicate it's a language keyword rather than regular text.
