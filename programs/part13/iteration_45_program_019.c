Looking at this code, it appears to be part of a C/C++ compiler or parser that handles different token types and generates corresponding error messages. Here's what I can tell:

**Purpose**: This is a switch statement that maps various token types (RT_EXTERN, RT_STATIC_ASSERT, etc.) to error message strings when an expected token is not found.

**Structure**:
- Each `case` handles a specific token type
- `gmsgid` is assigned a localized error message using the `G_()` macro (likely for internationalization)
- The messages use `%<` and `%>` markers which probably denote keyword formatting in error output

**Token types covered**:
- `RT_EXTERN` - C/C++ `extern` keyword
- `RT_STATIC_ASSERT` - C++11 `static_assert`
- `RT_DECLTYPE` - C++11 `decltype`
- `RT_OPERATOR` - C++ operator overloading keyword
- `RT_CLASS` - C++ class keyword
- `RT_TEMPLATE` - C++ template keyword
- `RT_NAMESPACE` - C++ namespace keyword
- `RT_USING` - C++ using declaration/directive
- `RT_ASM` - Inline assembly keyword
- `RT_TRY` - C++ exception handling `try` block

**Context**: This is likely from the error recovery or parsing phase of a compiler like GCC, where when the parser expects a specific keyword but doesn't find it, it generates a helpful error message like "expected 'extern'" or "expected 'class'".

The code shows good practice with:
1. Internationalization support via `G_()`
2. Consistent formatting for keywords in error messages
3. Clear mapping between internal token representations and user-facing keywords
