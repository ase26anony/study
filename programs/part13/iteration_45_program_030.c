Looking at this code snippet, it appears to be part of a C/C++ compiler or parser error handling system. Here's what I can tell:

## Context
This is likely from a compiler's source code (possibly GCC) where `RT_` constants represent different token types or expected constructs in the code being parsed.

## Structure
- Each `case` handles a different token type (`RT_EXTERN`, `RT_STATIC_ASSERT`, etc.)
- `gmsgid` stores an error message string
- `G_()` is likely a macro for internationalization/gettext support
- `%<` and `%>` are formatting markers that likely get replaced with proper quotation marks or highlighting in the final error message

## Purpose
This code generates human-readable error messages when the parser expects a specific keyword but encounters something else. For example:
- If the parser expects `extern` but finds something different, it will output: "expected 'extern'"
- If it expects `static_assert`, it outputs: "expected 'static_assert'"

## The `%<` and `%>` markers
These are special formatting sequences that likely:
- Get translated to appropriate quotation marks in the output
- May enable syntax highlighting in terminal output
- Could be replaced with HTML tags in IDE integration

This is a clean, maintainable way to handle error message generation for different C++ language constructs, with built-in support for internationalization.
