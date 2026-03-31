Looking at this code snippet, it appears to be part of a C/C++ compiler or parser error message generation system. Here's what I can tell:

## Context
This is likely from a C++ compiler codebase (possibly GCC) where `RT_` constants represent different types of tokens or expected constructs during parsing. The code is generating error messages when certain tokens are expected but not found.

## Structure
- Each `case` handles a different expected token type
- `gmsgid` stores a localized error message string
- `G_()` is likely a macro for internationalization/gettext support
- `%<` and `%>` are formatting markers for the keyword (probably for highlighting in error messages)

## Token Types
- `RT_EXTERN`: `extern` keyword
- `RT_STATIC_ASSERT`: `static_assert` keyword (C++11 feature)
- `RT_DECLTYPE`: `decltype` keyword (C++11 feature)
- `RT_OPERATOR`: `operator` keyword
- `RT_CLASS`: `class` keyword
- `RT_TEMPLATE`: `template` keyword
- `RT_NAMESPACE`: `namespace` keyword
- `RT_USING`: `using` keyword
- `RT_ASM`: `asm` keyword (inline assembly)
- `RT_TRY`: `try` keyword (exception handling)

## Purpose
This code generates user-friendly error messages like:
- "expected 'extern'"
- "expected 'static_assert'"
- "expected 'decltype'"
- etc.

When the parser encounters a syntax error and was expecting one of these keywords, it uses the appropriate message based on the `RT_` constant value passed to this switch statement.
