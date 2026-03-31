**Key Features of This Test Program:**

1. **Comprehensive `depend` Clause Coverage**: Includes all the uncovered dependency types with `update` modifier:
   - `depend(update(in:))` (line 2154)
   - `depend(update(inout:))` (line 2156)
   - `depend(update(out:))` (line 2158)
   - `depend(update(mutexinoutset:))` (line 2160)
   - `depend(update(inoutset:))` (line 2162)
   - `depend(destroy:)` (line 2164)

2. **C++ Specific Features**:
   - Uses C++ references (`int& ref_dep_in`)
   - Uses pointer to class object (`Data* obj_ptr`)
   - C++ class with constructor

3. **Valid OpenMP Constructs**:
   - Tasks with various `depend` clauses
   - `taskgroup` for nesting
   - `target` construct with `depend` clause
   - `taskwait` for synchronization

4. **OpenMP Version Guards**:
   - `#if _OPENMP >= 201811` for OpenMP 5.0+ features
   - `#if _OPENMP >= 202111` for OpenMP 5.2+ `destroy` dependency

5. **Execution Flow**:
   - Creates a dependency graph between tasks
   - Tasks perform actual work (modifying variables)
   - Final output verifies all tasks executed correctly

**Compilation and Testing**:
