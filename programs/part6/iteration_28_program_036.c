## Key Design Elements:

1. **Multiple Trigger Patterns**:
   - `func_high_gt_zero`: Creates values with non-zero high parts (targets `a_high.sgt(max_r)`)
   - `func_zero_high_large_low`: Creates values with zero high part but large low part (targets `a_high == max_r && a_low.ugt(max_s)`)
   - Uses bit masks like `0x80000000ULL` to create values just above `2^31-1`

2. **Compiler Optimization Triggers**:
   - Loops with induction variables for VRP analysis
   - Complex arithmetic that requires range tracking
   - `__builtin_expect` for branch prediction hints
   - `__attribute__((const))` for pure function optimization
   - `__int128` types for wider fixed-value analysis

3. **Boundary Value Creation**:
   - Values like `0x80000000ULL` (exactly `2^31`) to exceed `max_s = (1<<31)-1`
   - Operations that ensure high 32 bits are zero while low 32 bits are large
   - Recursive functions for complex value flow analysis

4. **Compilation Recommendations**:
