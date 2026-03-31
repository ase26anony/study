This program systematically exercises various OpenMP dependency patterns:

1. **Template-dependent dependency types**: Uses template parameters to select between `in`, `out`, and `inout` dependencies
2. **`omp_depend_t` objects**: Creates dependency objects for `depend(depobj:)` clauses
3. **Fold expressions**: Generates multiple dependencies in a single clause
4. **`this` pointer in dependencies**: Uses member access in template class context
5. **Task reductions with dependencies**: Combines `task_reduction` with `depend` clauses
6. **OpenMP 5.0 `detach` clause**: Uses the newer detach feature with dependencies
7. **Set-based dependencies**: Exercises `mutexinoutset` and `inoutset` on array sections
8. **Combined constructs**: Uses dependencies with `target`, `taskloop`, and `parallel master` constructs
9. **Affinity clause**: Combines `affinity` with `depend`
10. **Iterator modifiers**: Uses array sections with dependencies in loops

The program is designed to generate a wide variety of internal `OMP_CLAUSE_DEPEND` representations, increasing the likelihood that some will fall into the uncovered `default:` case of the pretty-printer switch.

Compile with:
