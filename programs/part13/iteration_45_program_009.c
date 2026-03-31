Looking at this code snippet, it appears to be part of a C/C++ compiler or parser's error message handling system. Here's what I can deduce:

## Context
This is likely from a C++ compiler's source code (possibly GCC) that handles parsing errors. The code is generating error messages for when certain tokens are expected but not found.

## Structure
- `RT_` constants represent different token types (like `RT_EXTERN`, `RT_STATIC_ASSERT`, etc.)
- `gmsgid` is a variable that stores the error message ID/string
- `G_()` is a macro for internationalization/gettext support
- `%<` and `%>` are special markers for formatting keywords in error messages

## Purpose
When the parser expects a specific keyword (like `extern`, `static_assert`, `decltype`, etc.) but encounters something else, it uses this switch statement to generate the appropriate error message.

## Example
If the parser is expecting the `extern` keyword but finds something else, it will generate the error: "expected 'extern'"

## The `%<` and `%>` markers
These are likely used to:
- Format the keyword specially in error output (possibly bold or quoted)
- Handle localization properly by marking the keyword as non-translatable content within a translatable message

This is a common pattern in compiler development where error messages need to be both user-friendly and properly internationalized.
