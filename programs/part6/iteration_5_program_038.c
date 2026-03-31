This program exercises the target code path through:

1. **Return statements** with dereferenced pointers, conditional expressions mixing lvalues/rvalues, and wrapped reference calls
2. **auto/decltype deduction** with dereferenced pointers and address-of operators
3. **Template argument deduction** with perfect forwarding and reference types
4. **Modern C++ features** like structured bindings, lambda captures, and if-with-initializer using dereferenced pointers
5. **Temporary materialization** scenarios with address-of operators on cast expressions
6. **Complex nested cases** with multiple value category transformations
7. **Qualified types** (const/volatile) to ensure full path coverage

Compile with:
