This test program provides:

1. **All `depend` modifiers**: Each uncovered case (`in`, `inout`, `out`, `mutexinoutset`, `inoutset`, `destroy`) appears in valid OpenMP task directives.

2. **Complex data environment**: Uses global, static, extern, heap-allocated, reference, pointer, array element, and struct member variables in depend clauses.

3. **Nested constructs**: Tasks inside `parallel` regions, `single` sections, and `taskgroup` blocks with `taskwait`.

4. **Multiple scopes**: Class member functions, namespace functions, and template functions.

5. **Diagnostic trigger**: The `problematic_function()` contains a deliberate syntax error (`UndeclaredType`) that will cause the compiler to emit an error. When compiling with diagnostic flags, the compiler may pretty-print surrounding OpenMP constructs in error messages.

**Compilation commands to trigger coverage:**
