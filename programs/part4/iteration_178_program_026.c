## Key Design Elements:

1. **Wide Integer Types**: Uses both `__int128` (128-bit) and `long long` (64-bit) to trigger double-int analysis.

2. **Non-Constant Bounded Shifts**: 
   - `shift_amount & 63` for 128-bit (0-63 bits)
   - `shift_amount & 31` for 64-bit (0-31 bits)
   This creates bounded but non-constant shift amounts.

3. **Mixed Signed/Unsigned Comparisons**:
   - Direct signed comparison: `result128 > 0x7FFFFFFFFFFFFFFFLL`
   - Combined with unsigned: `(unsigned long long)result64 > 0x7FFFFFFFFFFFFFFFULL`
   - Complex conditions with `||` and `&&` operators

4. **Conditional Control Flow**: 
   - `if (sign_selector & 1)` creates two different computation paths
   - Multiple comparison conditions increase analysis complexity

5. **Preventing Simplification**:
   - `volatile` seed prevents constant propagation
   - `noinline` functions keep computations opaque
   - Loop with varying parameters ensures dynamic analysis

## Compilation Recommendations:
