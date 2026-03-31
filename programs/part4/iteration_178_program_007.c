## Key Design Elements:

1. **Wide Integer Operations**: Uses `long long` (64-bit) and optionally `__int128` (128-bit) types that require double-word handling in GCC's fixed-value analysis.

2. **Non-Constant Shifts**: Shift amounts are bounded but non-constant (`shift_amount & 63`), forcing the compiler to compute possible ranges.

3. **Mixed Signed/Unsigned Comparisons**: 
   - Direct signed comparisons (`signed_result > 0x7FFFFFFFFFFFFFFFLL`)
   - Mixed expressions with both `sgt` and `ugt` equivalents
   - Conditional expressions with `||` and `&&` operators

4. **Complex Control Flow**: 
   - Loop with 1000 iterations creates multiple analysis paths
   - Conditional shifts based on previous results
   - Different computation paths based on modifier values

5. **Range-Bounded Values**: All operations use masking (`&`) to keep values within analyzable bounds while remaining non-constant.

6. **Prevention of Simplification**:
   - `volatile` variables prevent constant propagation
   - `__attribute__((noinline))` keeps complex operations intact
   - Final checksum output prevents dead code elimination

## Compilation Recommendations:
