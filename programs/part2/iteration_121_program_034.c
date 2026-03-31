## Key Design Elements:

1. **SIMT Transformation Triggers:**
   - Uses `#pragma omp target teams distribute parallel for simd` which is the primary construct that triggers SIMT transformation
   - Includes `collapse(2)` clause for 2D loops, increasing complexity
   - Multiple distinct offloaded regions with different computation patterns

2. **Preventing Constant Folding:**
   - Uses command-line arguments (`argc`, `argv`) for loop bounds and scaling factors
   - `volatile` variable for loop limit
   - Data-dependent computations with conditional branches

3. **GPU Offloading Requirements:**
   - `#pragma omp declare target` for functions called within target regions
   - `#pragma omp target data map` for explicit data management
   - Multiple target regions to increase coverage probability

4. **Complex Control Flow:**
   - Conditional statements inside loops
   - Function calls within target regions
   - Reduction operation in third target region

## Compilation Commands:
