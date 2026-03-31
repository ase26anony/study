**Key features that trigger the uncovered code:**

1. **Register Variables**: Explicit `register` variables bound to call-clobbered registers (`r12`, `r13` on x86_64) force specific register allocation.

2. **Basic Block Boundaries**: 
   - Function calls at the end of `if` blocks (before `return`)
   - Calls at the end of loop bodies (before increment)
   - Calls in `switch` cases with fall-through
   - Calls immediately before `return` statements

3. **Register Pressure**: Many local variables compete for registers, forcing spills.

4. **Live Across Calls**: Register variables are used both before and after function calls, requiring caller-save operations.

5. **Multiple Call Patterns**: Different helper functions with varying numbers of parameters create complex live ranges.

**Compilation options to ensure coverage:**
