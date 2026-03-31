This program includes:

1. **Template-based dependency selection** (`TaskGenerator` template with specializations)
2. **OpenMP 5.0+ features**: `detach` clause, `task_reduction`, combined constructs
3. **`omp_depend_t` objects** with `depend(depobj: ...)` clauses
4. **Variadic templates and fold expressions** for multiple dependencies
5. **Complex iterator expressions** in dependency subscripts
6. **`mutexinoutset` and `inoutset` dependency types**
7. **Conditional dependency generation** using `if constexpr`
8. **Combined constructs** (`target teams distribute parallel for`, `taskloop`, `parallel master taskloop`)
9. **Member function with `this` pointer** in dependencies
10. **Optional unsupported usage** guarded by `TEST_UNSUPPORTED`

To compile and generate the internal representations needed for coverage analysis:
