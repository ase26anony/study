**Key Features of This Test Program:**

1. **Comprehensive `depend` Clause Coverage:**
   - Standard dependencies: `in`, `out`, `inout`
   - OpenMP 5.0+ update modifiers: `update(in)`, `update(out)`, `update(inout)`, `update(mutexinoutset)`, `update(inoutset)`
   - OpenMP 5.2+ `destroy` dependency (conditionally compiled)

2. **C++ Specific Features:**
   - References: `int& ref_dep`
   - Class objects: `MyObj obj`
   - Object pointers: `&obj` in dependency clauses

3. **Valid OpenMP Constructs:**
   - Nested `parallel` → `single` → `task` structure
   - `taskgroup` for nested task synchronization
   - `taskwait` for barrier synchronization
   - Atomic operations for thread-safe updates

4. **OpenMP Version Guards:**
   - `#if _OPENMP >= 201811` for OpenMP 5.0+ features
   - `#if _OPENMP >= 202011` for OpenMP 5.2+ `destroy` clause

5. **Execution Flow:**
   - Creates a dependency graph ensuring tasks execute in correct order
   - Performs actual computations to prevent optimization removal
   - Produces deterministic output for verification

**Compilation and Testing:**
