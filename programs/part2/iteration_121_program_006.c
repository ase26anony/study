### Key Design Elements:

1. **SIMT Transformation Triggers:**
   - Uses `#pragma omp target teams distribute parallel for simd` which is the primary trigger for SIMT transformation
   - Includes `collapse(2)` clause creating a 2D loop structure
   - Uses `simdlen(8)` clause to hint at vectorization width

2. **Dynamic Loop Bounds:**
   - Uses `volatile int base_size` to prevent constant propagation
   - Reads from command-line arguments (`argv[1]`, `argv[2]`)
   - Uses runtime-computed indices in collapsed loops

3. **Multiple Offloaded Regions:**
   - Three distinct target regions with different computation patterns
   - Different data mappings (`map(to:)`, `map(from:)`, `map(tofrom:)`)
   - Mix of simple computation and conditional operations

4. **Device Data Management:**
   - Nested `#pragma omp target data` regions
   - Explicit data mappings between host and device
   - `declare target` functions for device execution

5. **Prevention of Optimizations:**
   - Runtime-dependent loop bounds
   - Volatile variables for thresholds
   - Final checksum computation prevents dead code elimination
   - Reduction operation adds complexity

### Recommended Compilation Commands:
