This program comprehensively targets the uncovered AVX-512 blend expansion code by:

1. **Explicit AVX-512 Intrinsic Usage**: Direct calls to `_mm512_mask_blend_*` intrinsics for all supported data types (epi8, epi16, epi32, epi64, ps, pd).

2. **Auto-vectorization for HF/BF modes**: Standard C loops with ternary operators that should compile to AVX-512 blend instructions when compiled with `-O3 -mavx512fp16 -mavx512bf16`.

3. **Volatile Control Flow**: Uses `volatile` variables for thresholds and selectors to prevent constant folding and ensure blend instructions are generated.

4. **Template Metaprogramming**: The `test_blend_template<Mode>` function uses `if constexpr` to handle all 8 modes, with explicit template instantiations for each.

5. **Mixed Data Type Functions**: The `blend_all_types()` function uses a switch statement to call different blend implementations based on a volatile selector, ensuring all code paths are exercised.

6. **Execution Flow**: The `main()` function calls all template instantiations, runs the mixed-type function with all selectors, and performs explicit tests for each data type with random data to ensure the blend expansion logic is triggered.

Compile with:
