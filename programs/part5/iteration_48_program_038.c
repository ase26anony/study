**Key features that trigger the uncovered lines:**

1. **All `depend` modifiers are used:**
   - `depend(in: ...)` - Line 20
   - `depend(inout: ...)` - Line 28
   - `depend(out: ...)` - Line 35
   - `depend(mutexinoutset: ...)` - Line 52 and 94
   - `depend(inoutset: ...)` - Line 58 and 100
   - `depend(destroy: ...)` - Line 64 and 106

2. **Complex dependence expressions:**
   - Multiple items in single clause (line 20, 28, 35, 88)
   - Mixed modifiers (line 41)
   - Pointer dereferencing (line 41)
   - Array indexing (line 69)

3. **Various OpenMP contexts:**
   - Inside `taskgroup` (line 17)
   - Combined with `priority`, `final`, `mergeable` clauses
   - Within template function `process_tasks`
   - In class method `TaskProcessor::run_complex_tasks`
   - Using `_Pragma` form (line 52)

4. **Prevents dead code elimination:**
   - All tasks modify shared data
   - Uses `atomic` operations
   - Final verification in `main()`

5. **Execution flow ensures coverage:**
   - Parallel region with master
   - Multiple tasks with dependencies
   - `taskwait` for synchronization
   - Final verification with checksum

**Compilation options to trigger pretty-printing:**
