### Key Design Elements:

1. **Multiple SIMT Transformation Triggers**:
   - Three separate `target teams distribute parallel for simd` regions
   - Different loop structures and clauses to increase coverage probability

2. **Dynamic Loop Bounds**:
   - Uses command-line arguments (`argc`, `argv`) for loop sizes
   - Volatile variable `volatile_n` prevents constant propagation
   - Dynamic scaling factor `scale` from command line

3. **Complex Loop Structures**:
   - 2D loops with `collapse(2)` clause
   - Mixed computation patterns (direct computation, conditional updates, reduction)
   - Function calls within loops marked with `declare target`

4. **Device Data Management**:
   - Multiple `target data` regions with explicit `map` clauses
   - Proper data movement between host and device

5. **Preventing Optimizations**:
   - Non-constant initialization patterns
   - Final checksum computation ensures no dead code elimination
   - Output of results prevents removal of computations

### Recommended Compilation Command:
