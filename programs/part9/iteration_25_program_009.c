**Key Features of this Test Program:**

1. **Comprehensive `depend` Clause Coverage:**
   - Basic types: `in`, `out`, `inout`
   - OpenMP 5.0+ with `update` modifier: `update(in)`, `update(out)`, `update(inout)`, `update(mutexinoutset)`, `update(inoutset)`
   - OpenMP 5.2+ `destroy` clause

2. **C++-Specific Features:**
   - Uses C++ references (`ref_a`)
   - Uses pointers to objects (`ptr_b`)
   - Uses class objects (`MyObj obj_c`)
   - Takes address of objects (`&obj_c`)

3. **Valid OpenMP Structure:**
   - All tasks within a `parallel` region with `single` construct
   - Proper shared variable declarations
   - `taskwait` for synchronization
   - Minimal but meaningful operations in tasks

4. **Version Compatibility:**
   - Uses `#if _OPENMP >= 201811` for OpenMP 5.0+ features
   - Uses `#if _OPENMP >= 202111` for OpenMP 5.2+ `destroy` clause
   - Falls back gracefully with older OpenMP implementations

5. **Execution Flow:**
   - Creates a dependency graph between tasks
   - Tasks perform actual computations to ensure clauses aren't optimized away
   - Produces deterministic output for verification

**Compilation and Testing:**
