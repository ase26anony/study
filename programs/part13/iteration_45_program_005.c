Looking at this code, it appears to be part of a C/C++ compiler's error handling system, likely from GCC or a similar compiler. This is a switch statement that maps different token types (RT_* constants) to their corresponding error messages.

Here's what each case represents:

1. **RT_EXTERN** - For `extern` keyword
2. **RT_STATIC_ASSERT** - For `static_assert` keyword  
3. **RT_DECLTYPE** - For `decltype` keyword
4. **RT_OPERATOR** - For `operator` keyword
5. **RT_CLASS** - For `class` keyword
6. **RT_TEMPLATE** - For `template` keyword
7. **RT_NAMESPACE** - For `namespace` keyword
8. **RT_USING** - For `using` keyword
9. **RT_ASM** - For `asm` keyword
10. **RT_TRY** - For `try` keyword

The `G_()` macro is likely for internationalization/gettext support, marking these strings for translation. The `%<` and `%>` are probably formatting markers that will be replaced with appropriate quotes or formatting in the final error message.

This code would be used when the parser encounters a syntax error and needs to report what token was expected at that position in the code. For example, if the parser expected a `class` keyword but found something else, it would use the `RT_CLASS` case to generate the error message "expected 'class'".
