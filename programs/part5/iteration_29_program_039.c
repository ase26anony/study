**Key features that target the uncovered code:**

1. **Complex OpenMP Nesting**: The `#pragma omp target teams distribute parallel for simd` creates a multi-layered construct that generates the complex `gomp_for` statement needed for the SIMT transformation.

2. **Non-Trivial Loop Structure**: 
   - Loop bound `N` is not compile-time constant (though known, it's not marked constexpr)
   - Loop body calls a device function `device_func()`
   - Includes reduction operation with non-uniform weighting `(i % 10 + 1)`

3. **Data Mapping**: Explicit `map(to: ...)` and `map(from: ...)` clauses ensure data transfer between host and device.

4. **Conditional Execution Path**: The `argc` check creates runtime branching that may encourage generation of the conditional labels seen in the uncovered code.

5. **SIMD Clause**: The `simd` clause on the target region is crucial for triggering the SIMT transformation.

6. **Additional SIMD Loop**: The checksum calculation uses `#pragma omp simd` to further stress SIMD lowering.

**Compilation commands to test coverage:**
