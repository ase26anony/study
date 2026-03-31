**Key Design Points:**

1. **Complete Coverage of All `depend` Types:**
   - `depend(in: arr1)` - Task 1
   - `depend(out: arr2)` - Task 2
   - `depend(inout: arr3)` - Task 3
   - `depend(mutexinoutset: arr1)` - Task 4
   - `depend(inoutset: arr2)` - Task 5
   - `depend(destroy: arr3)` - `taskwait` construct

2. **Valid OpenMP Semantics:**
   - All tasks are created inside an `omp single` region within a parallel region
   - Proper data-sharing attributes (`shared`) for all accessed variables
   - Atomic operations for concurrent updates to `sum`
   - Array sections (`arr1[0:N]`) for proper dependency specification

3. **Optimization Prevention:**
   - `volatile` seed variable and helper function
   - Final checksum computation and print ensure all tasks have observable effects
   - Multiple atomic operations prevent dead code elimination

4. **Compilation Recommendations:**
