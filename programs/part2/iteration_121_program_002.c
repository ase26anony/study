## Key Features for Triggering the SIMT Transformation:

1. **Multiple SIMT-eligible constructs**: Four different `target teams distribute parallel for simd` regions with varying clauses and patterns.

2. **Dynamic loop bounds**: Uses command-line arguments and `volatile` variables to prevent constant propagation and folding.

3. **Nested parallelism with collapse**: Uses `collapse(2)` on 2D loops to increase complexity and trigger the transformation.

4. **Device data mappings**: Explicit `map` clauses ensure data transfers between host and device.

5. **Declare target functions**: Functions marked for offloading ensure the full offloading pipeline is used.

6. **Varied computation patterns**: 
   - First region: Simple arithmetic with function call
   - Second region: Conditional updates
   - Third region: Reduction operation
   - Fourth region: Mathematical function with different scheduling

7. **Prevents optimization**: 
   - `volatile` variables prevent constant folding
   - Final checksum computation prevents dead code elimination
   - Data-dependent computations prevent loop removal

## Compilation Commands:
