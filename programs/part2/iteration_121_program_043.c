### Key Features for Triggering the SIMT Transformation:

1. **Multiple Target Regions**: Four different OpenMP target regions with varying characteristics:
   - First: `target teams distribute parallel for simd collapse(2)` - Most likely to trigger SIMT
   - Second: `target teams distribute parallel for simd` with conditional computation
   - Third: Reduction pattern with SIMD
   - Fourth: Nested loop without SIMD clause for comparison

2. **Dynamic Loop Bounds**: Uses command-line arguments (`argc`, `argv`) and `volatile` variables to prevent constant folding and optimization.

3. **Data-Dependent Computations**: 
   - `compute_value()` with non-constant scale
   - `conditional_update()` with branches
   - Reduction with varying weights
   - Square root vs square based on condition

4. **Device Data Management**: Uses `target data map` for explicit data transfers between host and device.

5. **Declare Target Functions**: Marks computation functions for offloading with `#pragma omp declare target`.

### Compilation Commands:
