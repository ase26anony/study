Looking at this code snippet, it appears to be part of a C/C++ compiler or parser's error handling code. Here's what I can tell:

## Purpose
This is likely part of a switch statement that handles different "expected token" error messages in a compiler or parser. The code generates appropriate error messages when the parser encounters unexpected input.

## Structure
- `RT_` constants represent different token types or expected constructs
- `gmsgid` is a variable that stores the error message ID/string
- `G_()` is likely a macro for internationalization/gettext support

## Token Types
The code handles these expected tokens:
- `RT_EXTERN` - `extern` keyword
- `RT_STATIC_ASSERT` - `static_assert` keyword
- `RT_DECLTYPE` - `decltype` keyword  
- `RT_OPERATOR` - `operator` keyword
- `RT_CLASS` - `class` keyword
- `RT_TEMPLATE` - `template` keyword
- `RT_NAMESPACE` - `namespace` keyword
- `RT_USING` - `using` keyword
- `RT_ASM` - `asm` keyword
- `RT_TRY` - `try` keyword

## Formatting
The `%<` and `%>` in the strings are likely formatting markers that will be replaced with appropriate quotation marks or highlighting in the final error message output.

## Context
This is probably from GCC or a similar compiler's source code, specifically from the parser's error recovery or diagnostic reporting section. The code would be used when the parser expects a specific keyword but encounters something else instead.
