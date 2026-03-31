**Key Design Rationale:**

1. **Complete Coverage of Target Cases:**
   - `depend(in: arr1)` → triggers `OMP_CLAUSE_DEPEND_IN`
   - `depend(out: arr2)` → triggers `OMP_CLAUSE_DEPEND_OUT`
   - `depend(inout: arr3)` → triggers `OMP_CLAUSE_DEPEND_INOUT`
   - `depend(mutexinoutset: arr1)` → triggers `OMP_CLAUSE_DEPEND_MUTEXINOUTSET`
   - `depend(inoutset: arr2)` → triggers `OMP_CLAUSE_DEPEND_INOUTSET`
   - `taskwait depend(destroy: arr3)` → triggers `OMP_CLAUSE_DEPEND_LAST`

2. **Anti-Optimization Measures:**
   - `volatile` seed prevents constant folding of array initializations
   - Atomic operations on shared `sum` variable ensure tasks have side effects
   - Final checksum computation uses all arrays after parallel region
   - Array sections (`arr1[0:N]`) ensure whole arrays are referenced

3. **Valid OpenMP Semantics:**
   - All tasks are created inside `single` construct within parallel region
   - Proper data-sharing attributes (`shared`) specified
   - Array sections use valid OpenMP 4.5+ syntax
   - `taskwait` ensures proper synchronization

4. **Compilation Recommendations:**
