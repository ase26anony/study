**Key Features of This Test Program:**

1. **Comprehensive `depend` Clause Coverage:**
   - `depend(in:)` on line 41
   - `depend(out:)` on line 48
   - `depend(inout:)` on lines 55 and 62 (two variants)
   - `depend(mutexinoutset:)` on line 73 (OpenMP 5.0+)
   - `depend(inoutset:)` on lines 80 and 87 (OpenMP 5.0+)
   - `depend(destroy:)` on line 101 (OpenMP 5.2+)

2. **C++ Specific Features:**
   - C++ references (`int& ref_dep`) used in dependency clause
   - Pointer to class object (`Data* obj_ptr`) used in dependency clause
   - Class member access in tasks

3. **Valid Dependency Variables:**
   - All variables are properly scoped and shared in the parallel region
   - `destroy` uses `omp_depend_t` object with proper initialization

4. **Mixed OpenMP Constructs:**
   - `parallel` region containing `single` construct
   - Multiple `task` constructs with different dependencies
   - `taskwait` for synchronization
   - Atomic operations to ensure data races are avoided

5. **Version Guards:**
   - `#if _OPENMP >= 201811` for OpenMP 5.0+ features
   - `#if _OPENMP >= 202011` for OpenMP 5.2+ `destroy` dependency

6. **Execution Flow:**
   - Tasks form a dependency graph ensuring proper execution order
   - Final computation verifies all tasks executed correctly
   - Deterministic output regardless of thread scheduling

**Compilation and Testing:**
