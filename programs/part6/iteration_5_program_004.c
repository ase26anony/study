This test program is designed to trigger the specific uncovered lines in `tree.cc` by:

1. **Multiple return statement patterns** that require address-taking and dereferencing
2. **Template type deduction** with reference types and perfect forwarding
3. **`auto` and `decltype` usage** that forces value category analysis
4. **Modern C++ features** (structured bindings, lambda captures, if-with-initializer)
5. **Temporary materialization** scenarios
6. **Complex nested expressions** that require multiple conversions

**Compilation recommendations:**
