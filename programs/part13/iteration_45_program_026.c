Looking at this code, it appears to be part of a C/C++ compiler's error message handling system. This is likely from GCC (GNU Compiler Collection) or a similar compiler. Here's what I can tell:

## Purpose
This code handles error messages for when the parser expects specific C++ keywords but encounters something else instead.

## Structure
- Each `case` corresponds to a different expected token type (RT_EXTERN, RT_STATIC_ASSERT, etc.)
- `gmsgid` stores the error message string
- `G_()` is likely a macro for internationalization/gettext support
- The `%<` and `%>` are special formatting markers that probably get replaced with appropriate quotes or formatting in the final error message

## Context
This is likely from a `switch` statement in a function that reports parsing errors. The function probably takes a token type as input and returns the appropriate error message.

## The tokens being handled:
- **RT_EXTERN**: `extern` keyword
- **RT_STATIC_ASSERT**: `static_assert` keyword (C++11/C11)
- **RT_DECLTYPE**: `decltype` keyword (C++11)
- **RT_OPERATOR**: `operator` keyword (for operator overloading)
- **RT_CLASS**: `class` keyword
- **RT_TEMPLATE**: `template` keyword
- **RT_NAMESPACE**: `namespace` keyword
- **RT_USING**: `using` keyword
- **RT_ASM**: `asm` keyword (inline assembly)
- **RT_TRY**: `try` keyword (exception handling)

This code would be used when the parser is in a context where one of these keywords is required but wasn't found, generating an error like "expected 'extern'" or "expected 'class'".
