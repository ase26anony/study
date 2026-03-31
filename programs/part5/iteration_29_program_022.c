## Key Features Targeting the Uncovered Code:

1. **SIMT Transformation Trigger**: The `#pragma omp target teams distribute parallel for simd` with GPU offloading (`-foffload=nvptx-none`) specifically triggers the SIMT transformation that generates `IFN_GOMP_USE_SIMT`.

2. **Complex Loop Structure**: 
   - Non-constant trip count (`N = 1000`)
   - Data-dependent condition (`if (result[i] > 100.0f)`)
   - Device function call (`device_compute()`)
   - Mathematical operations preventing optimization removal

3. **Multi-layered OpenMP Context**: The combination of `teams distribute parallel for simd` creates the nested OpenMP constructs needed for the complex `gomp_for` statement transformation.

4. **Data Mapping**: Explicit `map(to:...)` and `map(from:...)` clauses engage the full offloading infrastructure.

5. **Conditional Execution Path**: The `argc` check creates runtime branching that encourages generation of the conditional labels (`lab1`, `lab2`, `lab3`) seen in the uncovered code.

6. **Device Function**: `#pragma omp declare target` marks `device_compute()` for device execution, testing SIMT transformation with function calls.

## Compilation Commands:
