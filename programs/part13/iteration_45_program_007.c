Looking at this code, it appears to be part of a C/C++ compiler's error reporting system, specifically handling different token types for better error messages. Here's what I notice:

## Key Observations:

1. **Error Message Generation**: This code generates user-friendly error messages when the parser encounters unexpected tokens.

2. **Token Types**: Each `case` corresponds to a specific C++ keyword/token type:
   - `RT_EXTERN`: `extern` keyword
   - `RT_STATIC_ASSERT`: `static_assert` keyword
   - `RT_DECLTYPE`: `decltype` keyword
   - `RT_OPERATOR`: `operator` keyword
   - `RT_CLASS`: `class` keyword
   - `RT_TEMPLATE`: `template` keyword
   - `RT_NAMESPACE`: `namespace` keyword
   - `RT_USING`: `using` keyword
   - `RT_ASM`: `asm` keyword
   - `RT_TRY`: `try` keyword

3. **Internationalization**: The `G_()` macro is used for string localization/translation support.

4. **Formatting**: The `%<` and `%>` markers likely format the keyword for display (probably as code/highlighted text in error messages).

## Code Structure:
This is a `switch` statement where each case sets `gmsgid` (global message ID) to a localized error message indicating what token was expected.

## Potential Use Case:
This code would be used in parser error recovery when the compiler encounters syntax errors and needs to inform the user what token was expected at a particular position in the code.

## Example Error Message:
If the parser expected `extern` but got something else, it would generate: "expected 'extern'"

The code is clean, consistent, and follows good practices for internationalization in compiler development.
