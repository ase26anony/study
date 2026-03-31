**Key features that target the uncovered code:**

1. **OpenMP Target Offloading with SIMD**: Uses `#pragma omp target teams distribute parallel for simd` which should trigger the SIMT transformation for GPU offloading.

2. **Non-Trivial Loop Structure**: 
   - Loop bound `N` is not compile-time constant
   - Complex loop body with mathematical operations and device function calls
   - Uses `std::sqrt()` which requires math library support

3. **Multiple Nested OpenMP Constructs**: The `simd` clause is nested within `parallel for` which is within `teams distribute`, creating the multi-layered context needed.

4. **Data Mapping**: Uses `map(to:...)` and `map(from:...)` clauses for arrays and reduction variable.

5. **Conditional Offloading**: Uses `argc` check to decide between offloaded and host execution paths.

6. **Device Function**: `device_function` is declared with `#pragma omp declare target` and called from within the SIMD loop.

7. **Reduction**: Uses `reduction(+:result)` which creates additional GIMPLE complexity.

8. **Atomic Operation**: Additional atomic operation in the second loop to create more complex GIMPLE sequences.

**Compilation commands to test:**
