Looking at this code snippet, it appears to be part of a C/C++ compiler or parser's error message handling system. Here's what I can deduce:

## Purpose
This code is handling different token types (RT_* constants) and assigning appropriate error messages when a specific token is expected but not found.

## Structure
- Each `case` corresponds to a different token type (RT_EXTERN, RT_STATIC_ASSERT, etc.)
- `gmsgid` is being set to a localized error message using the `G_()` macro (likely for internationalization)
- The `%<` and `%>` in the strings are probably formatting markers for highlighting keywords in error messages

## Token Types Covered
The code handles these C++ keywords/concepts:
- **RT_EXTERN**: `extern` keyword
- **RT_STATIC_ASSERT**: `static_assert` keyword (C++11 and later)
- **RT_DECLTYPE**: `decltype` keyword (C++11 and later)
- **RT_OPERATOR**: `operator` keyword
- **RT_CLASS**: `class` keyword
- **RT_TEMPLATE**: `template` keyword
- **RT_NAMESPACE**: `namespace` keyword
- **RT_USING**: `using` keyword
- **RT_ASM**: `asm` keyword (inline assembly)
- **RT_TRY**: `try` keyword (exception handling)

## Context
This is likely from the GCC compiler codebase or a similar C/C++ compiler/parser. The `RT_` prefix probably stands for "Required Token" or "Recovery Token" - these are likely used in error recovery when the parser encounters unexpected input and needs to report what it was expecting.

The code shows how compilers generate user-friendly error messages by mapping internal token types to human-readable keyword names with proper localization support.
