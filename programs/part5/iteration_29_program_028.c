**Key features that target the uncovered code:**

1. **Nested OpenMP constructs**: The `#pragma omp target teams distribute parallel for simd` creates a complex `gomp_for` statement that will be processed by the SIMT transformation.

2. **Non-trivial loop structure**: 
   - Loop bound `N` is not compile-time constant (though known, it's not marked constexpr)
   - Contains inner loop (`for (int j = 0; j < 3; j++)`)
   - Conditional operations (`if (i % 2 == 0)`)
   - Device function call (`device_func()`)
   - Mathematical operations that generate substantial GIMPLE

3. **Runtime conditional offloading**: The `use_offload` variable based on `argc` creates different execution paths, encouraging the generation of conditional labels seen in the uncovered code.

4. **Data mapping**: Explicit `map(to:...)` and `map(from:...)` clauses ensure full offloading infrastructure engagement.

5. **Reduction clause**: The `reduction(+:result)` adds complexity to the OpenMP lowering.

**Compilation commands to test:**
