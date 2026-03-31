This program includes:

1. **All `depend` clause modifiers**: `in`, `out`, `inout`, `mutexinoutset`, `inoutset`, and `destroy`
2. **Diagnostic triggers**: Uses `[[deprecated]]` variable to generate warnings
3. **Template contexts**: Template functions and classes with `if constexpr`
4. **Lambda expressions**: OpenMP tasks inside lambdas
5. **Multiple scopes**: Namespace, class methods, static methods
6. **Constructor attribute**: `__attribute__((constructor))` for early processing
7. **Complex expressions**: Array elements, pointer dereferences, member access
8. **Final synchronization**: Uses `depend(destroy:)` with `taskwait`
9. **Checksum calculation**: Prevents optimization removal

To compile and trigger the pretty-printer coverage:
