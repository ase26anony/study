This comprehensive test program includes:

1. **TYPE_UNDEFINED**: Forward-declared `UndefinedType` and `NeverDefinedClass` with pointers to them
2. **TYPE_SCALAR**: All fundamental types in various contexts (global, local, parameters)
3. **TYPE_STRING**: String literals, character arrays, wide strings, UTF strings
4. **TYPE_STRUCT**: Multiple C structures with different features (nested, bit-fields, packed, flexible arrays)
5. **TYPE_USER_STRUCT**: C++ classes with inheritance, virtual functions, templates, complex members
6. **TYPE_UNION**: C-style unions, anonymous unions, unions in structures
7. **TYPE_POINTER**: All pointer variations (single, double, function, member, qualified)
8. **TYPE_ARRAY**: Multi-dimensional arrays, arrays of pointers, arrays of structs
9. **TYPE_CALLBACK**: Function pointers, std::function, lambdas, template callbacks
10. **TYPE_LANG_STRUCT**: Lambdas with captures, initializer_list, structured bindings, fold expressions, coroutines

The program uses `argc` to control execution flow, ensuring all code paths are potentially reachable. It takes addresses of variables and functions, creates instances of all types, and produces observable output to prevent dead code elimination.

**Recommended compilation commands:**
