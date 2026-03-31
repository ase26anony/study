This program systematically exercises various OpenMP dependency patterns:

1. **Template-dependent tasks** with conditional dependency types that may generate uncommon internal codes
2. **`omp_depend_t` objects** with `depend(depobj:)` clauses
3. **OpenMP 5.0 features** like `detach` and `task_reduction` with dependencies
4. **Combined constructs** (`target teams`, `taskloop`, `parallel master taskloop`) with dependencies
5. **Complex array sections** with `mutexinoutset` and `inoutset`
6. **Affinity clauses** combined with dependencies
7. **Template class member functions** using `this` pointer in dependencies

The `TaskGenerator<3>` instantiation uses `inoutset` which should be handled by the switch, but the template instantiation mechanism combined with complex array section expressions may generate internal representations that fall into the `default` case during pretty-printing.

Compile with:
