Looking at this code, it appears to be part of a C/C++ compiler's error handling system, likely from GCC or a similar compiler. This is a switch statement that maps different token types (`RT_EXTERN`, `RT_STATIC_ASSERT`, etc.) to corresponding error messages.

Here's what each part does:

1. **Token Types**: Each `case` handles a specific reserved token type in C++:
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

2. **Error Messages**: For each token type, it sets `gmsgid` to an error message string that says "expected [keyword]". The `G_()` macro is likely for internationalization/translation support.

3. **Formatting**: The `%<` and `%>` in the strings are probably special markers that format the keyword appropriately in error messages (likely to display it as code or in quotes).

**Purpose**: This code would be used when the parser encounters a syntax error and needs to report what token was expected at that position. For example, if the parser expected an `extern` declaration but found something else, it would use the `RT_EXTERN` case to generate the error message "expected 'extern'".

The code structure suggests this is part of a larger error reporting mechanism where different parts of the parser can indicate what they were expecting when they encountered unexpected input.
