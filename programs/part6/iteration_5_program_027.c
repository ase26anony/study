This test program systematically exercises the uncovered lines by:

1. **Return Statements**: Multiple functions with different return expression patterns that should trigger the `force_rvalue` logic
2. **auto/decltype**: Various auto and decltype usages with dereferenced pointers
3. **Template Deduction**: Template functions with forwarding references and reference parameters
4. **Modern C++ Features**: Structured bindings, lambda captures, if-with-initializer
5. **Temporary Materialization**: Complex expressions requiring temporary address-taking
6. **Edge Cases**: Volatile qualifiers, multiple indirections, member pointers

To compile and run with the recommended options:
