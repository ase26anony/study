## Key Features Targeting the Uncovered Code:

1. **SIMT Transformation Trigger**: The `#pragma omp target teams distribute parallel for simd` with GPU offloading (`-foffload=nvptx-none`) should trigger the SIMT transformation that generates `IFN_GOMP_USE_SIMT`.

2. **Complex Loop Structure**: 
   - Non-constant trip count from command-line argument
   - Data-dependent computation (`i % 8 + 1`)
   - Conditional execution within loop (`if (i % 3 == 0)`)
   - Device function call (`device_compute()`)

3. **Multi-layered OpenMP Context**: The combination of `teams distribute parallel for simd` creates a complex `gomp_for` statement that needs SIMT transformation.

4. **Data Mapping**: Explicit `map(to:...)` and `map(from:...)` clauses ensure full offloading infrastructure engagement.

5. **Conditional Execution Paths**: The `use_offload` variable (made volatile) creates runtime branching that may encourage generation of the conditional labels seen in the uncovered code.

6. **Portable Target Specification**: `#pragma omp declare target` marks `device_compute()` as callable from device code.

## Compilation Commands:
