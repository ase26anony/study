This test program comprehensively covers all the uncovered lines in the GCC C++ pretty-printer:

1. **All dependency types with `update` modifier**: The program includes tasks with `depend(update(in:))`, `depend(update(out:))`, `depend(update(inout:))`, `depend(update(mutexinoutset:))`, and `depend(update(inoutset:))` clauses.

2. **`destroy` dependency**: Includes `depend(destroy:)` clause (guarded by OpenMP 5.2+ version check).

3. **C++ specific features**: Uses C++ references (`int&`), class objects, and pointers to class objects as dependency arguments.

4. **Mixed OpenMP constructs**: Uses `task`, `target`, `taskgroup`, and nested parallelism within tasks.

5. **Valid dependency variables**: All variables are properly scoped as shared in the parallel region, and addresses are taken where needed.

6. **Execution flow**: Creates a dependency graph where tasks wait on their dependencies, and includes `taskwait` to ensure completion.

7. **Preprocessor guards**: Uses `#if _OPENMP >= 201811` for OpenMP 5.0+ features and `#if _OPENMP >= 202011` for OpenMP 5.2+ features.

To compile and test with GCC:
