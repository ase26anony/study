This program generates a comprehensive set of OpenMP dependency patterns:

1. **Template-based dependency selection** (`TaskGenerator<0..2>`) - Generates different dependency types via template instantiation
2. **`omp_depend_t` objects** (`test_depobj()`) - Uses dependency objects which may map to `OMP_CLAUSE_DEPEND_DEPOBJ`
3. **Detach clause with dependencies** (`test_detach()`) - OpenMP 5.0 feature that may generate unique dependency codes
4. **Task reduction with dependencies** (`test_task_reduction()`) - Combines reduction and dependency clauses
5. **`mutexinoutset` and `inoutset` on array sections** (`test_set_dependencies()`) - Less common dependency types
6. **Iterator-based dependencies** (`test_iterator_dependency()`) - Dependencies using loop indices
7. **Combined constructs** (`test_combined_constructs()`) - Dependencies on target, taskloop, and parallel master constructs
8. **Template class with `this` pointer** (`Container<T>`) - Dependencies involving member access through `this`
9. **Multiple dependencies in single clause** (`fold_deps`) - Fold expression style multiple arguments

The program is designed to force the GCC OpenMP implementation to generate various internal representations of dependency clauses, increasing the likelihood that some will fall into the uncovered `default:` case of the pretty-printer switch statement.

Compile with:
