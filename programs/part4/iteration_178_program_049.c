**Key design elements targeting the uncovered lines:**

1. **Wide Integer Types**: Uses `__int128` and `unsigned __int128` which are handled as double-int types in GCC's internal representation.

2. **Non-Constant Bounded Shifts**: 
   - `shift_amount = shift_amount & 63` creates bounded but non-constant shift counts
   - Different bounds (63, 31) create different range analysis scenarios

3. **Mixed Signed/Unsigned Comparisons**:
   - Direct signed comparisons (`signed_result > 0x7F...`)
   - Direct unsigned comparisons (`unsigned_result > 0xFF...`)
   - Complex conditions with `&&` and `||` operators
   - Casts between signed and unsigned types

4. **Complex Control Flow**:
   - Conditional arithmetic (`if (modifier & 2)`)
   - Loop with varying parameters
   - Nested conditions in comparisons

5. **Range-Creating Operations**:
   - Left shifts that can overflow into negative values
   - Multiplication that expands ranges
   - Logical right shifts that ensure non-negative values
   - Mask operations that bound ranges

6. **Prevention of Simplification**:
   - `volatile` seed prevents constant propagation
   - `noinline` attributes keep functions intact
   - Complex expressions prevent early optimization

**Compilation recommendations:**
