**Key Features of This Test Program:**

1. **Comprehensive `depend` Clause Coverage**:
   - `depend(in: ref_dep_in)` - using C++ reference
   - `depend(out: *ptr_dep_out)` - using pointer dereference
   - `depend(inout: dep_inout)` - basic inout dependency
   - `depend(mutexinoutset: dep_mutexinoutset)` - OpenMP 5.0+
   - `depend(inoutset: dep_inoutset)` - OpenMP 5.0+
   - `depend(destroy: destroy_obj)` - OpenMP 5.2+

2. **C++ Specific Features**:
   - Uses C++ references (`int& ref_dep_in`)
   - Uses pointers and pointer dereferencing
   - Uses class objects and member access (`obj_ptr->value`)
   - C++11 compatible

3. **Valid Dependency Patterns**:
   - Variables are properly shared in parallel region
   - Dependency chains ensure tasks wait appropriately
   - `taskgroup` and `taskwait` for synchronization
   - Deterministic execution order through dependencies

4. **OpenMP Version Guards**:
   - `#if _OPENMP >= 201811` for OpenMP 5.0+ features
   - `#if _OPENMP >= 202111` for OpenMP 5.2+ `destroy` clause
   - Graceful degradation with older OpenMP implementations

5. **Execution Verification**:
   - Tasks perform actual work (incrementing, modifying variables)
   - Final result verification ensures dependencies were respected
   - Clear success/failure output

**Compilation and Testing**:
