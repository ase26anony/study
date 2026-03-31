This code appears to be part of a C++ compiler's pretty-printing logic for OpenMP clauses, specifically handling `depend` clause variants. Here's what each case represents:

## OpenMP `depend` Clause Types:

1. **`OMP_CLAUSE_DEPEND_IN`** → `depend(in: ...)`
   - Input dependency: task reads from the specified storage location
   - Printed as `update(in)`

2. **`OMP_CLAUSE_DEPEND_INOUT`** → `depend(inout: ...)`
   - Input/output dependency: task reads and writes to the specified storage location
   - Printed as `update(inout)`

3. **`OMP_CLAUSE_DEPEND_OUT`** → `depend(out: ...)`
   - Output dependency: task writes to the specified storage location
   - Printed as `update(out)`

4. **`OMP_CLAUSE_DEPEND_MUTEXINOUTSET`** → `depend(mutexinoutset: ...)`
   - Mutual exclusion dependency for sets
   - Printed as `update(mutexinoutset)`

5. **`OMP_CLAUSE_DEPEND_INOUTSET`** → `depend(inoutset: ...)`
   - Input/output dependency for sets
   - Printed as `update(inoutset)`

6. **`OMP_CLAUSE_DEPEND_LAST`** → `depend(destroy: ...)`
   - Destroy dependency (for data environment destruction)
   - Printed as `destroy`

## Context:
This is likely from GCC's C++ frontend (`cp/` directory) where `pp_cxx_ws_string` is a pretty-printer function that outputs formatted text with appropriate whitespace. The `update(...)` format suggests this might be for internal representation or debugging output rather than the actual source code syntax.

The `default:` case handles any unexpected dependency types by doing nothing (no output).
