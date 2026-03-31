This program includes:

1. **Template-based dependency selection** (`TaskWithDependency<0-3>`) that uses conditional expressions in the `depend` clause
2. **`omp_depend_t` objects** in `test_depobj()` which may generate `OMP_CLAUSE_DEPEND_DEPOBJ`
3. **Affinity clause** combined with `depend` in `test_affinity()`
4. **Task reduction with dependencies** in `test_task_reduction()`
5. **Detach clause** (OpenMP 5.0) in `test_detach()`
6. **Combined constructs** (`target teams distribute`, `taskloop`, `parallel master taskloop`) with dependencies
7. **Template with `this` pointer** in `Container::process()`
8. **Fold expressions** for multiple dependencies in `task_with_multiple_deps()`
9. **Optional unsupported usage** guarded by `TEST_UNSUPPORTED`

To compile and test:
