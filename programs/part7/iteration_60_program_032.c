**Key features that target the uncovered lines:**

1. **Wide Integer Operations**: Uses `__int128` and `unsigned __int128` throughout with relational comparisons.

2. **Mixed Signed/Unsigned Contexts**: 
   - The `cmp_mixed()` function directly compares signed and unsigned 128-bit values
   - Loop iterations create both signed and unsigned values for comparison
   - Uses values where the high bit is set (triggering sign extension issues)

3. **Loop-Based Range Testing**: 
   - Loop from -5 to 5 creates values where the high word changes
   - Tests both signed and unsigned versions near boundaries

4. **Function Returns Based on Comparisons**: 
   - Multiple helper functions return `int` based on 128-bit comparisons
   - Results are aggregated to prevent dead code elimination

5. **Complex Expressions**: 
   - Chains multiple comparisons with `&&` and `||` operators
   - Uses ternary operator with 128-bit comparisons
   - Combines arithmetic operations with comparisons

6. **Boundary Cases**: 
   - Tests maximum positive and negative 128-bit values
   - Tests equality cases (where high and low words are equal)
   - Tests values where only the high word differs

**Compilation recommendations:**
