## Key Design Elements:

1. **High Register Pressure**: 
   - 16+ variables of mixed types (int, long, float, double)
   - Computations before and after the call create live ranges across the call

2. **Non-inline Function Call**:
   - `helper_function` marked `__attribute__((noinline))`
   - Called with multiple arguments of different types

3. **Instruction Placement Opportunities**:
   - Computations like `int r1 = v1 * v2 + v3` produce values used after the call
   - Memory barriers (`asm volatile`) limit reordering, forcing explicit save/restore

4. **Prevent Optimization**:
   - `volatile` variables for final results
   - Command-line arguments as seeds
   - Global side effects in helper function
   - Loop with varying inputs

5. **Basic Block Structure**:
   - Instructions after the call (`int result = r1 + ...`)
   - The moved instruction could be placed between the call and these uses

## Compilation and Testing:
