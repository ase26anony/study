This program comprehensively targets the AVX-512 blend expansion logic by:

1. **Exhaustive AVX-512 Intrinsic Usage**: Directly calls `_mm512_mask_blend_epi8`, `_mm512_mask_blend_epi16`, `_mm512_mask_blend_epi32`, `_mm512_mask_blend_epi64`, `_mm512_mask_blend_pd`, and `_mm512_mask_blend_ps` for the corresponding data types.

2. **Auto-vectorization Patterns**: For `_Float16` (V32HFmode) and `__bf16` (V32BFmode), uses conditional ternary operators in loops that should auto-vectorize to AVX-512 blend instructions with appropriate compiler flags.

3. **Volatile Control Flow**: Uses `volatile` variables for thresholds and selectors to prevent constant folding and ensure the blend logic is actually generated.

4. **Template Metaprogramming**: Implements `test_blend_impl<Mode>` template with specializations for all 8 modes (0-7), forcing the compiler to instantiate code for each data type.

5. **Mixed Data Type Functions**: Provides separate functions (`blend_int8_arrays`, `blend_float_arrays`, `blend_double_arrays`) that use different blend patterns, increasing the chance of hitting different code paths in the expansion logic.

6. **Execution Flow**: The `main()` function initializes arrays with random data, uses volatile selectors to call different blend implementations, and includes explicit intrinsic calls with hardcoded masks to ensure the blend instructions are generated even if auto-vectorization fails.

**Recommended compilation command:**
