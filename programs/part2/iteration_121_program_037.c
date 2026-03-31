### Key Design Elements for Triggering the SIMT Block:

1. **Multiple Target Regions with SIMD**: Three distinct `#pragma omp target teams distribute parallel for simd` regions with different loop structures increase the probability of hitting the transformation block.

2. **Collapsed 2D Loop**: The first region uses `collapse(2)` with a 2D loop structure, which is specifically mentioned in the requirements to increase complexity and likelihood of triggering `scan_omp_for`.

3. **Dynamic Loop Bounds**: Using `argc/argv` for loop bounds prevents constant folding. The `volatile` variable `base_size` adds another layer of protection against optimization.

4. **Data-Dependent Computations**: 
   - First loop: Uses `compute_scale()` function with trigonometric operations
   - Second loop: Contains conditional branches (`if/else if/else`)
   - Third loop: Reduction pattern

5. **Device Data Management**: The `#pragma omp target data map` ensures proper data transfer between host and device, keeping the offloading pipeline active.

6. **Declare Target Functions**: The `compute_scale()` and `init_array()` functions are marked with `declare target`, ensuring they're processed through the full offloading pipeline.

### Compilation Commands:
