This program comprehensively exercises various OpenMP dependency features:

1. **Template-dependent dependency types** (`TaskGenerator<0..3>`) - May generate specialized internal representations
2. **`omp_depend_t` objects** (`test_depobj()`) - Uses `depend(depobj:)` which may map to `OMP_CLAUSE_DEPEND_DEPOBJ`
3. **Fold expressions with multiple dependencies** (`multi_depend_task`) - Creates complex dependency lists
4. **Detach clause with dependencies** (`test_detach()`) - OpenMP 5.0 feature with potential unique internal codes
5. **Task reduction with dependencies** (`test_task_reduction()`) - Combines reduction and dependency semantics
6. **Affinity clause with dependencies** (`test_affinity()`) - May generate distinct dependency-related codes
7. **Combined constructs** (`test_combined()`) - Uses dependencies in `taskloop`, `target`, and `parallel master` contexts
8. **Template class with `this` pointer** (`Container::process()`) - Creates unique tree nodes for member access
9. **`mutexinoutset` dependency** - Explicitly tests one of the less common dependency types

The program is designed to force the GCC OpenMP implementation to generate a wide variety of internal `OMP_CLAUSE_DEPEND` representations, increasing the likelihood that some will fall into the `default:` case of the pretty-printer switch.

Compile with:
