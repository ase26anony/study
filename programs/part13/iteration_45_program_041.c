Looking at this code snippet, it appears to be part of a C/C++ compiler or parser implementation, likely from GCC or a similar compiler. Here's what I can deduce:

## Context
This is a switch statement handling different token types (RT_* constants) and setting appropriate error messages. The `G_()` macro is likely for internationalization/translation support.

## Token Types
The RT_* constants represent:
- **RT_EXTERN**: `extern` keyword
- **RT_STATIC_ASSERT**: `static_assert` keyword  
- **RT_DECLTYPE**: `decltype` keyword
- **RT_OPERATOR**: `operator` keyword
- **RT_CLASS**: `class` keyword
- **RT_TEMPLATE**: `template` keyword
- **RT_NAMESPACE**: `namespace` keyword
- **RT_USING**: `using` keyword
- **RT_ASM**: `asm` keyword
- **RT_TRY**: `try` keyword

## Purpose
This code generates error messages when the parser expects a specific keyword but encounters something else. The `%<` and `%>` in the strings are likely formatting markers that will be replaced with appropriate quotation or highlighting when the error message is displayed.

## Code Structure
Each case sets `gmsgid` (global message ID) to a translatable string indicating which keyword was expected. This pattern suggests the code is part of a larger error reporting mechanism in a compiler front-end.

The `RT_` prefix likely stands for "Required Token" or "Recovery Token", indicating these are keywords the parser was expecting to find at a particular point in the code being compiled.
