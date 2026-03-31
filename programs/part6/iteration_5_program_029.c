This test program systematically exercises the target code path through:

1. **Multiple return statement patterns** that force lvalue-to-rvalue conversion with address-taking
2. **auto/decltype usage** that requires the compiler to evaluate expression value categories
3. **Template argument deduction** with reference types from dereferenced pointers
4. **Modern C++17 features** like structured bindings and if-with-initializer
5. **Temporary materialization** scenarios requiring address-of operations
6. **Complex nested patterns** combining multiple triggers

Compile with:
