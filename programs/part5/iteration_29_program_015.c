## Key Features Targeting the Uncovered Code:

1. **SIMT Transformation Trigger**: The `#pragma omp target teams distribute parallel for simd` with GPU offloading (`-foffload`) specifically triggers the SIMT transformation that generates `IFN_GOMP_USE_SIMT`.

2. **Complex Loop Structure**: 
   - Non-constant trip count (N = 1000)
   - Device function call inside the loop
   - Multiple arithmetic operations
   - Conditional execution within the loop body
   - Reduction operation

3. **Multi-layered OpenMP Context**: The combination of `teams distribute parallel for simd` creates a complex nested OpenMP construct that stresses the GIMPLE transformation.

4. **Data Mapping**: Explicit `map(to:...)` and `map(from:...)` clauses ensure full offloading infrastructure engagement.

5. **Conditional Execution Path**: The `argc` check creates runtime branching that encourages generation of the conditional labels (`lab1`, `lab2`, `lab3`) seen in the uncovered code.

6. **Device Function**: The `device_compute` function marked with `declare target` tests SIMT transformation with device function calls.

## Compilation Commands:
