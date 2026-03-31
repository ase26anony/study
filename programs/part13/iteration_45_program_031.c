Looking at this code, it appears to be part of a C/C++ compiler or parser that handles different token types and generates corresponding error messages. Here's what I can tell:

1. **Purpose**: This is likely from a parser's error handling code where it generates human-readable error messages for expected tokens.

2. **Structure**: Each `case` corresponds to a token type (RT_* constants) and sets `gmsgid` to a localized error message using the `G_()` macro (likely for internationalization/gettext).

3. **Token types covered**:
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

4. **Formatting**: The messages use `%<` and `%>` which are likely special markers to format the keyword (e.g., for syntax highlighting in error output).

5. **Context**: This is probably part of a larger `switch` statement handling various parsing errors where the parser expected a specific keyword but encountered something else.

The code is clean and follows a consistent pattern for error message generation in a multilingual compiler/parser system.
