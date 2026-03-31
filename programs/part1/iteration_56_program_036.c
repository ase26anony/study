This program generates:

1. **Template-dependent dependency types** using `TaskGenerator` with different specializations
2. **`omp_depend_t` objects** with `depend(depobj: ...)` clauses
3. **Fold expressions** for multiple dependencies in `multi_depend_task`
4. **`this` pointer dependencies** in template class member functions
5. **OpenMP 5.0 `detach` clause** with dependencies
6. **`taskgroup` with `task_reduction`** and dependencies
7. **Combined constructs** (`target teams distribute parallel for`, `taskloop`, `parallel master taskloop`)
8. **`affinity` clause** combined with `depend`
9. **Iterator-based dependencies** in ordered loops
10. **Target data regions** with dependencies
11. **Optional unsupported usage** (guarded by `TEST_UNSUPPORTED`)

To compile and generate the internal representations that might trigger the uncovered pretty-printer path:
