### Key Design Elements:

1. **SIMT Transformation Triggers:**
   - Uses `#pragma omp target teams distribute parallel for simd` which is the primary construct that triggers SIMT transformation
   - Includes `collapse(2)` to create 2D loops that increase transformation complexity
   - Multiple target regions increase the chance of hitting the uncovered block

2. **Preventing Constant Folding:**
   - Uses command-line arguments (`argc`, `argv`) for loop bounds and scale factors
   - `volatile` variables `v_n` and `v_m` prevent compile-time optimization
   - Data-dependent computations with modulo operations

3. **GPU Offloading Requirements:**
   - `#pragma omp declare target` marks functions for device execution
   - `#pragma omp target data map` manages data transfers
   - `num_teams()` and `thread_limit()` clauses provide GPU-specific hints

4. **Complex Control Flow:**
   - Multiple target regions with different computation patterns
   - Conditional operations inside loops
   - Reduction operation to ensure computation isn't optimized away
   - Dynamic scheduling to create varied control flow

### Compilation Commands:
