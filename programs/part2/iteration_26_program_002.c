This program includes:

1. **Loop-carried dependencies** with varying trip counts in `nested_loops_dependent`
2. **Mixed data types** and non-contiguous memory access via the `MixedData` structure
3. **Inline assembly with clobbers** that force the scheduler to work around fixed constraints
4. **SIMD intrinsics** (SSE/AVX) in `simd_processing`
5. **Function calls with varying arguments** to `compute_helper` and math functions
6. **Conditional moves** via ternary operators
7. **Computed goto** for indirect branching in `computed_goto_test`
8. **Loop unrolling pragmas** (`#pragma GCC unroll`)
9. **Switch statements** with dense cases in `switch_pattern`
10. **Matrix operations** with anti-dependencies
11. **Optimization attributes** to ensure selective scheduling is enabled

To maximize coverage of the uncovered lines, compile with:
