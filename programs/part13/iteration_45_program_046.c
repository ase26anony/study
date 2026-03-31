Looking at this code, it appears to be part of a C/C++ compiler's error reporting system, specifically handling different types of tokens or constructs that were expected but not found. Here's what I can tell:

## Code Analysis

This is a switch statement that maps various token types (RT_EXTERN, RT_STATIC_ASSERT, etc.) to their corresponding error messages. Each case handles a different C++ keyword or construct.

## Key Observations:

1. **Error Message Format**: Each message uses `G_()` which is likely a macro for internationalization/gettext support, allowing for translation of error messages.

2. **Message Formatting**: The messages use `%<` and `%>` to delimit keywords in the output. This is likely a GCC-specific formatting convention for highlighting keywords in error messages.

3. **Token Types**: The code handles various C++ constructs:
   - `RT_EXTERN`: `extern` keyword
   - `RT_STATIC_ASSERT`: `static_assert` keyword (C++11 and later)
   - `RT_DECLTYPE`: `decltype` keyword (C++11 and later)
   - `RT_OPERATOR`: `operator` keyword
   - `RT_CLASS`: `class` keyword
   - `RT_TEMPLATE`: `template` keyword
   - `RT_NAMESPACE`: `namespace` keyword
   - `RT_USING`: `using` keyword
   - `RT_ASM`: `asm` keyword (inline assembly)
   - `RT_TRY`: `try` keyword (exception handling)

## Likely Context:

This code is probably from GCC's C++ frontend (gcc/cp/). The switch statement is part of a function that generates error messages when the parser encounters a syntax error and needs to report what token was expected.

## Example Usage:

When the parser expects an `extern` declaration but finds something else, it would generate: "expected 'extern'"

The `%<` and `%>` markers would likely be replaced with appropriate formatting (like quotes or highlighting) in the final error message output.
