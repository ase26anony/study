This program exercises multiple aspects of OpenMP dependency clauses:

1. **Template-based dependency selection** - Uses template parameters to select different dependency types
2. **`omp_depend_t` objects** - Creates dependency objects for `depend(depobj: ...)` clauses
3. **Fold expressions** - Uses C++17 fold expressions for multiple dependencies
4. **Taskgroup with reduction** - Combines `task_reduction` with dependencies
5. **Set dependencies** - Uses `mutexinoutset` and `inoutset` with array sections
6. **Detach clause** - OpenMP 5.0 `detach` with event dependencies
7. **Affinity with dependencies** - Combines `affinity` and `depend` clauses
8. **Iterator dependencies** - Uses dependencies in loop constructs
9. **Combined constructs** - Uses dependencies with `target teams distribute parallel for` and `taskloop`
10. **Questionable usage** - Includes (disabled) non-standard usage for edge cases

Compile with:
