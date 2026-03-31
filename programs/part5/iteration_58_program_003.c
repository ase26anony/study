## Key Design Elements:

1. **High Register Pressure**: 
   - 8+ local variables of mixed types (double, int, float, long)
   - All variables are used in complex expressions before and after the call
   - Variables are forced to be live across the call using memory barriers

2. **Non-Inlineable Call**:
   - `helper_function` marked with `noinline, noclone`
   - Takes 12 arguments (exceeds x86-64's 6 register parameters)
   - Forces some arguments to be passed on the stack

3. **Forced Register Usage**:
   - `forced_reg_var` explicitly assigned to `r12` (call-clobbered on x86-64)
   - This variable is live across the call, forcing save/restore

4. **Basic Block Structure**:
   - The call is in the middle of straight-line code
   - No jumps or labels immediately after the call
   - Multiple dependent operations after the call

5. **Memory Barriers**:
   - `asm volatile("" : : : "memory")` before and after calls
   - Prevents compiler from reordering or eliminating saves

6. **Multiple Call Sites**:
   - Two different functions with register pressure
   - Called repeatedly in a loop with different inputs
   - Increases chance of hitting the uncovered code

## Compilation Recommendations:
