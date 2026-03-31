This program comprehensively exercises various OpenMP dependency features:

1. **Template-dependent tasks** (`TaskWithDep<0>`, `<1>`, `<2>`) - Uses conditional expressions in `depend` clauses based on template parameters.

2. **`omp_depend_t` objects** (`test_depobj()`) - Creates dependency objects which may map to internal `OMP_CLAUSE_DEPEND_DEPOBJ` codes.

3. **Detach clause with dependencies** (`test_detach()`) - Uses OpenMP 5.0 `detach` clause which creates unique dependency relationships.

4. **Task reduction with dependencies** (`test_task_reduction()`) - Combines `task_reduction` with `depend` clauses.

5. **Affinity with dependencies** (`test_affinity_depend()`) - Uses `affinity` clause alongside `depend`.

6. **Iterator-based dependencies** (`test_iterator_deps()`) - Uses `depend(source/sink:)` with iteration counters.

7. **Set dependencies** (`test_set_dependencies()`) - Uses `mutexinoutset` and `inoutset` with array sections.

8. **Combined constructs** (`test_combined_constructs()`) - Uses `target`, `taskloop`, and `parallel master taskloop` with dependencies.

9. **Container with `this` pointer** - Uses `this->data` in dependency expressions within a template class.

10. **SFINAE template** (`test_sfinae_dep`) - Creates dependency clauses in template context that may trigger special handling.

The program is designed to generate a wide variety of internal `OMP_CLAUSE_DEPEND` representations, increasing the likelihood that some will fall into the uncovered `default:` case in the pretty-printer. The `#ifdef TEST_UNSUPPORTED` section shows how to test non-standard usage if needed.

Compile with:
