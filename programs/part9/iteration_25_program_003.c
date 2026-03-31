**Key Features of This Test Program:**

1. **Comprehensive `depend` Clause Coverage:**
   - `depend(in: dep_in)` - Read dependency
   - `depend(out: dep_out)` - Write dependency  
   - `depend(inout: dep_inout)` - Read/write dependency
   - `depend(mutexinoutset: dep_mutexinoutset)` - OpenMP 5.0+
   - `depend(inoutset: dep_inoutset)` - OpenMP 5.0+
   - `depend(destroy: destroy_obj)` - OpenMP 5.2+

2. **C++ Specific Features:**
   - Uses C++ references (`int& ref_dep`)
   - Uses pointers to class objects (`Data* obj_ptr`)
   - C++ class with constructor

3. **Valid Dependency Graph:**
   - Tasks are ordered correctly (e.g., `dep_in` must be set before it's read)
   - Mixed dependencies in single task (`depend(in: dep_out) depend(inout: dep_inout)`)
   - Proper scoping with `shared` variables in parallel region

4. **OpenMP Version Guards:**
   - `#if _OPENMP >= 201811` for OpenMP 5.0+ features
   - `#if _OPENMP >= 202011` for OpenMP 5.2+ `destroy` dependency

5. **Execution Flow:**
   - Creates a task dependency graph
   - Uses `taskwait` to ensure completion
   - Produces deterministic output for verification

**Compilation and Testing:**
