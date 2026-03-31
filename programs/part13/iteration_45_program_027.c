Looking at this code snippet, it appears to be part of a C/C++ compiler or parser codebase (likely GCC based on the `G_()` macro for internationalization). This is a switch statement that maps various token types to their corresponding error message strings.

Here's what each part represents:

1. **Token Types**: The `case` statements are checking different token types:
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

2. **Error Messages**: Each case assigns an error message string to `gmsgid` using the `G_()` macro, which is for internationalization/translation support. The messages all follow the pattern "expected `<keyword>`" where the keyword is wrapped in `%<` and `%>` markers (likely for formatting/quoting in error output).

3. **Context**: This code is likely from an error handling routine where the parser expected a specific keyword but encountered something else. The `gmsgid` variable probably gets used later to display the appropriate error message to the user.

4. **Pattern**: The code shows a clear pattern of mapping token types to human-readable keyword names for error reporting purposes.

This is typical in compiler implementation where the parser needs to provide meaningful error messages when it encounters syntax errors, telling users what token was expected at a given position in the code.
