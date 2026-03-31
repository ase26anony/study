## Key Design Elements:

1. **Decrement-and-Compare Patterns**: The code uses multiple variations:
   - `while (counter-- > 0)` - Post-decrement with explicit comparison
   - `for (; counter; --counter)` - Pre-decrement with implicit zero check
   - `do { ... } while (--counter > 0)` - Pre-decrement at loop bottom
   - Explicit `counter = counter - 1` in loop body

2. **Volatile Variables**: Used to prevent:
   - Constant propagation of loop bounds
   - Complete loop elimination
   - Over-aggressive optimization of the decrement pattern

3. **Memory Side Effects**: 
   - `asm volatile("" : : : "memory")` creates memory barriers
   - Volatile stores force the compiler to maintain loop structure
   - Accumulator variable prevents dead code elimination

4. **Non-Constant Bounds**: Loop bounds come from `argc` or volatile variables, preventing compile-time trip count calculation.

5. **Nested Contexts**: Functions are called multiple times, and some loops are nested, increasing the chance the loop optimizer will process them in the right context.

## Recommended Compilation Commands:
