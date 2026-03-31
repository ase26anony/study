This program exercises multiple OpenMP dependency features:

1. **Template-based dependency selection** - Uses template parameters to choose between `in`, `out`, and `inout` dependencies
2. **`omp_depend_t` objects** - Creates and uses dependency objects with `depend(depobj: ...)` clauses
3. **Fold expressions** - Uses C++17 fold expressions to generate multiple dependencies
4. **`this` pointer in dependencies** - Uses member access in dependency clauses within template classes
5. **Taskgroup with task_reduction** - Combines dependencies with task reductions
6. **Combined constructs** - Uses `target teams distribute parallel for` and `taskloop` with dependencies
7. **Set dependencies** - Uses `mutexinoutset` and `inoutset` dependency types
8. **Conditional dependency types** - Uses ternary operator to select dependency type at compile time
9. **Detach clause** - Uses OpenMP 5.0 `detach` clause with dependencies

The program is designed to generate a wide variety of internal `OMP_CLAUSE_DEPEND` representations, increasing the likelihood that some will fall into the default case of the pretty-printer switch. The `#ifdef TEST_UNSUPPORTED` section shows how to test error-path dependencies if needed.

Compile with:
