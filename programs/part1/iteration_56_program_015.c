This program comprehensively exercises various OpenMP dependency features:

1. **Template-dependent dependency types** (`TaskWithDependency<0|1|2>`)
2. **`omp_depend_t` objects** (`depend(depobj: ...)`)
3. **Template class with `this` pointer** in depend clause
4. **OpenMP 5.0 `detach` clause** with dependencies
5. **`taskgroup` with `task_reduction`** and dependencies
6. **`mutexinoutset` and `inoutset`** dependencies
7. **`affinity` clause** combined with `depend`
8. **Fold expressions** for multiple dependencies (C++17)
9. **Combined constructs** (`target teams distribute parallel for`) with dependencies
10. **`taskloop` with `depend` clause**
11. **`parallel master taskloop`** with dependencies

The program is designed to generate a wide variety of internal `OMP_CLAUSE_DEPEND` representations, increasing the likelihood that some will fall into the `default` case of the pretty-printer switch. The `#ifdef TEST_UNSUPPORTED` section is commented out by default but can be enabled for testing error-path handling.

**Compilation command for coverage analysis:**
