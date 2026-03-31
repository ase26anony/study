Looking at this code, it appears to be part of a C/C++ compiler's error handling system. This is likely from GCC or a similar compiler. Here's what I can tell:

## Purpose
This code handles error messages for expected tokens in C++ parsing. When the parser encounters an unexpected token, it generates an appropriate error message based on what token was expected.

## Structure
- Each `case` corresponds to a different token type (like `RT_EXTERN`, `RT_STATIC_ASSERT`, etc.)
- `gmsgid` stores the error message string
- `G_()` is likely a macro for internationalization/gettext support

## Token Types
The code handles these C++ keywords/constructs:
- `extern` - for external linkage declarations
- `static_assert` - for compile-time assertions
- `decltype` - for type deduction
- `operator` - for operator overloading
- `class` - for class definitions
- `template` - for template declarations
- `namespace` - for namespace definitions
- `using` - for using declarations/directives
- `asm` - for inline assembly
- `try` - for exception handling blocks

## Formatting
The `%<` and `%>` in the strings are likely formatting markers that will be replaced with appropriate formatting (like quotes or highlighting) in the final error message output.

This is a clean, maintainable way to handle error messages for different expected tokens in a compiler's parser.
