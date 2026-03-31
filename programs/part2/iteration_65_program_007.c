## Key Design Features:

1. **Four Distinct Comparison Types**: Each function handles one of the four operators (`>`, `>=`, `<`, `<=`) to trigger all switch cases.

2. **Vectorization-Friendly Patterns**:
   - Simple data-parallel loops
   - Aligned memory accesses with `__builtin_assume_aligned`
   - `restrict` qualifiers to indicate no aliasing
   - Different data types (int, float, double) to test various vectorization paths

3. **Prevention of Optimizations**:
   - Loop bounds depend on `argc` to prevent constant propagation
   - Non-uniform initialization using `argc` and loop indices
   - Checksum computation and volatile printing to keep results live

4. **Conditional Patterns**:
   - `process_gt`: Conditional assignment `(a > b) ? a : b`
   - `process_ge`: Conditional with arithmetic `(a >= b) ? a*2 : b*3`
   - `process_lt`: Conditional with arithmetic `(a < b) ? a+1 : b-1`
   - `process_le`: Conditional with bitwise operations `(a <= b) ? a|1 : b&0xFE`

## Compilation Recommendations:
