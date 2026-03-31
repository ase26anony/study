Looking at this code, it appears to be part of a C/C++ compiler or parser's error message generation system. This is likely from GCC or a similar compiler codebase. Here's what I can deduce:

## Purpose
This code generates error messages for expected tokens/keywords in C++ parsing. When the parser encounters a syntax error where a specific keyword was expected, it uses this switch statement to generate the appropriate error message.

## Structure
- `RT_` constants represent different token types (like `RT_EXTERN`, `RT_CLASS`, etc.)
- `gmsgid` is a variable that stores the message ID for internationalization
- `G_()` is a macro for marking strings for translation (gettext)

## Keywords Covered
The code handles these C++ keywords:
- `extern` - for external linkage declarations
- `static_assert` - for compile-time assertions (C++11+)
- `decltype` - for type deduction (C++11+)
- `operator` - for operator overloading
- `class` - for class definitions
- `template` - for template declarations
- `namespace` - for namespace definitions
- `using` - for using declarations/directives
- `asm` - for inline assembly
- `try` - for exception handling blocks

## Formatting
The `%<` and `%>` are likely formatting markers that will be replaced with appropriate quotation marks or highlighting in the final error message output.

## Context
This is probably from a function that generates error messages like "expected 'class' before '...'" or similar syntax error messages. The `RT_` prefix likely stands for "Required Token" or "Expected Token".
