## Key Features That Trigger the Target Code Block:

1. **SIMT Transformation Triggers**:
   - Uses `#pragma omp target teams distribute parallel for simd` which is the primary construct that triggers SIMT transformation
   - The `collapse(2)` clause creates nested loops that need restructuring for GPU execution
   - Multiple target regions increase the chance of hitting the transformation

2. **Prevents Constant Propagation**:
   - Uses `volatile` variables (`base_size`, `adjust`)
   - Uses command-line arguments (`argc`, `argv`) for loop bounds
   - Dynamic loop bounds (`inner_limit = M - adjust`)

3. **GPU Offloading Specifics**:
   - `#pragma omp declare target` marks functions for device execution
   - `#pragma omp target data map` manages data transfer
   - `num_teams()` and `thread_limit()` clauses provide hints for GPU execution

4. **Complex Control Flow**:
   - Data-dependent computation with function call `compute_scale()`
   - Conditional updates in the second loop (`if (i % 4 == 0)`)
   - Reduction operation in the third region

## Compilation Commands:

To compile with NVIDIA PTX offloading (recommended for coverage):
