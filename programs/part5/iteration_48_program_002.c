## Key Features for Coverage:

1. **All Required Depend Modifiers:**
   - `depend(in: ...)` - Lines 31, 70, 104
   - `depend(inout: ...)` - Lines 27, 67, 103
   - `depend(out: ...)` - Lines 34, 72, 104
   - `depend(mutexinoutset: ...)` - Lines 38, 75
   - `depend(inoutset: ...)` - Line 44
   - `depend(destroy: ...)` - Lines 49, 78, 110, 142

2. **Multiple Contexts:**
   - Inside `taskgroup` regions (all major tests)
   - Combined with `priority`, `final`, `mergeable` clauses
   - Within template function `process_tasks()`
   - In class method `TaskProcessor::execute_tasks()`
   - Using `#pragma omp taskloop` (Test 3)

3. **Complex Dependence Expressions:**
   - Multiple items: `depend(inout: data[0], data[1])`
   - Mixed types: `depend(in: data[0]) depend(out: data[2])`
   - Pointer dereferencing: `depend(inout: *ptr, buffer[1])`
   - Array indexing: `depend(out: buffer[idx], buffer[idx+1])`
   - 2D array access: `depend(inout: grid[1][1], grid[2][0])`

4. **Prevents Dead Code Elimination:**
   - All tasks modify shared variables
   - Uses `atomic` operations for synchronization
   - Final verification with `assert()` statements
   - Computes and checks expected values

5. **Standard Conformance:**
   - C++17 with OpenMP 4.5+ features
   - Uses both `#pragma omp task` and `_Pragma("omp task")`
   - Portable across platforms

## Compilation and Testing:
