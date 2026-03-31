Looking at this code snippet, it appears to be part of a C/C++ compiler or parser's error handling code. This is likely from GCC or a similar compiler codebase. Here's what I can tell:

## Purpose
This code handles error messages for expected tokens/keywords in C++ parsing. When the parser expects a specific keyword but doesn't find it, it generates an appropriate error message.

## Structure
- Each `case` corresponds to a token type (RT_EXTERN, RT_STATIC_ASSERT, etc.)
- `gmsgid` stores the error message string
- `G_()` is likely a macro for internationalization/gettext support

## Token Types
The tokens represent various C++ keywords and constructs:
- **RT_EXTERN**: `extern` keyword
- **RT_STATIC_ASSERT**: `static_assert` keyword (C++11 and later)
- **RT_DECLTYPE**: `decltype` keyword (C++11 and later)
- **RT_OPERATOR**: `operator` keyword (for operator overloading)
- **RT_CLASS**: `class` keyword
- **RT_TEMPLATE**: `template` keyword
- **RT_NAMESPACE**: `namespace` keyword
- **RT_USING**: `using` keyword
- **RT_ASM**: `asm` keyword (inline assembly)
- **RT_TRY**: `try` keyword (exception handling)

## Formatting
The `%<` and `%>` in the strings are likely formatting markers that will be replaced with appropriate formatting (like quotes or highlighting) in the final error message output.

## Context
This is probably from a `switch` statement in a function that handles "expected token" errors, where the parser knows what token should come next but encountered something else instead.
