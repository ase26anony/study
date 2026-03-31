**Key features that target the uncovered lines:**

1. **Wide Integer Comparisons**: The `compare_int128` function explicitly breaks down `__int128` into high/low parts and compares them with unsigned casts, exactly matching the uncovered code pattern.

2. **Mixed Signed/Unsigned**: 
   - `compare_mixed` uses `unsigned __int128` comparisons
   - Main function mixes signed and unsigned checksums
   - Constants include both signed and unsigned large values

3. **Large Constants**: 
   - `VERY_LARGE_POS`, `VERY_LARGE_NEG` have non-zero high parts
   - `UNSIGNED_LARGE` uses the full 128-bit range
   - `MIXED_HIGH` has specific high/low bit patterns

4. **Arithmetic + Comparisons**: 
   - `process_arithmetic` performs operations then comparisons
   - Array values are generated with arithmetic operations
   - Range checks with wide integer bounds

5. **Control Flow**: 
   - `qsort` forces many comparison calls
   - `binary_search` adds more comparisons
   - `unreachable_path` ensures comparison code generation
   - Final checksum prevents dead code elimination

**Compilation suggestions:**
