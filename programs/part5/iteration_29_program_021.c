**Key features that target the uncovered code:**

1. **Nested OpenMP constructs**: The `#pragma omp target teams distribute parallel for simd` combines multiple constructs that should trigger the SIMT transformation when lowering to GIMPLE.

2. **Non-trivial loop structure**: 
   - Loop bound `N` is not compile-time constant (though known, it's not a literal)
   - Complex loop body with device function calls, conditionals, and mathematical operations
   - Reduction operation with conditional logic

3. **Data mapping**: Explicit `map` clauses with both `to` and `from` directions ensure proper offloading infrastructure engagement.

4. **Runtime conditional offloading**: The `use_offload` boolean creates a runtime decision point that may encourage generation of conditional execution paths.

5. **Device function**: `device_compute` is declared as a target function and called from within the SIMD loop, testing SIMT transformation with device calls.

6. **Multiple offloaded regions**: Two separate target regions increase the complexity of the GIMPLE representation.

**Compilation commands to test coverage:**
