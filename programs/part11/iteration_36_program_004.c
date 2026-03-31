**Key features that target the uncovered lines:**

1. **Three loop structures with specific relationships:**
   - **Loop A (outer):** Lines 78-118 - contains conditional execution of inner loop
   - **Loop B (inner):** Lines 87-103 - fully contained within Loop A when branch is taken
   - **Loop C (sibling):** Lines 122-147 - shares blocks with Loop A but has unique blocks

2. **Complex basic block creation:**
   - Multiple `if-else` branches within loops
   - `continue` and `break` statements creating different exit paths
   - Nested loops within conditionals
   - `goto` statement creating irreducible flow (line 34)
   - Function calls within loops adding more blocks

3. **Optimization prevention:**
   - `volatile` accumulator prevents dead code elimination
   - Command-line arguments prevent constant folding
   - `__attribute__((noinline))` on helper functions
   - Side effects (`printf`) and return value usage

4. **Hardware loop candidacy:**
   - Simple countable loops (`for (int i = 0; i < N; ++i)`)
   - Compile-time constants mixed with runtime values
   - Simple arithmetic operations in loop bodies

**Compilation recommendations:**
