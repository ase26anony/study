Looking at this code snippet, it appears to be part of a C/C++ compiler's error handling code, likely from GCC or a similar compiler. This is a switch statement that maps different token types (`RT_EXTERN`, `RT_STATIC_ASSERT`, etc.) to corresponding error messages.

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

2. **Error Messages**: For each token type, it sets `gmsgid` to an error message string using the `G_()` macro (likely for internationalization/gettext support). The messages follow the pattern "expected `<keyword>`" where the keyword is wrapped in `%<` and `%>` markers (probably for proper quoting in error output).

3. **Purpose**: This code is likely used when the parser encounters a syntax error and needs to report what token was expected at that position in the code. For example, if the parser expected an `extern` declaration but found something else, it would use the message "expected `extern`".

The code structure suggests this is part of a larger error reporting mechanism in a compiler frontend, where different syntactic contexts require different expected token messages.
