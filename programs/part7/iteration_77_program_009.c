## Key Design Elements:

1. **Multiple TM Constructs**: Uses both `__transaction_atomic` and `__transaction_relaxed` to increase coverage probability.

2. **Varied Loop Patterns**:
   - Simple for-loops with volatile indices
   - While-loops with non-constant strides
   - Nested loops accessing multi-dimensional data
   - Loops with transaction cancellation

3. **Memory Access Patterns**:
   - Array loads and stores with arithmetic operations
   - Cross-access between different shared arrays
   - Pointer-based access with volatile qualifiers
   - Dynamic array access within transactions

4. **Anti-Optimization Measures**:
   - `volatile` variables prevent dead code elimination
   - `noinline` and `noipa` attributes prevent function inlining
   - Non-constant loop bounds from function arguments
   - Final checksum calculation ensures all code executes

## Compilation and Testing:
