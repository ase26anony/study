**Key features that target the uncovered lines:**

1. **Wide Integer Operations**: Uses `__int128` and `unsigned __int128` with bitwise shifts and arithmetic operations.

2. **Variable Bit-Width**: The `bits` parameter varies from 65 to 127, forcing the compiler to analyze different bit-width scenarios.

3. **High/Low Part Extraction**: Explicitly extracts high and low 64-bit parts, mimicking the double_int structure in the uncovered code.

4. **Comparison Structure**: The `if (a_high == 0 && a_low > max_s_low)` comparison directly mirrors the structure in lines 264-277 of fixed-value.cc.

5. **Mixed Signed/Unsigned**: Uses both signed and unsigned 128-bit integers with conversions between them.

6. **Prevents Constant Folding**: 
   - `noinline` and `noclone` attributes on helper functions
   - `volatile` dummy variables to prevent dead code elimination
   - Command-line argument to vary the starting bit width
   - Variable shift counts based on input parameters

7. **Range of Operations**: Includes left/right shifts, addition, subtraction, and comparisons against computed maximum/minimum values for specific bit-widths.

**Compilation recommendations:**
