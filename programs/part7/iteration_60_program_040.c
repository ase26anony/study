**Key features that target the uncovered lines:**

1. **Wide Integer Operations**: Uses `__int128` and `unsigned __int128` throughout with relational comparisons in conditional statements.

2. **Mixed Signed/Unsigned Contexts**: 
   - The `compare_mixed()` function compares signed `__int128` with unsigned `unsigned __int128`
   - Various tests compare negative signed values with large unsigned values

3. **Loop-Based Range Testing**: 
   - Loop from -5 to 5 with values scaled by 2^62
   - `test_boundary_transitions()` loops around the 64-bit boundary
   - Both test high-word transition points

4. **Function Returns Based on Comparisons**: 
   - Multiple helper functions (`compare_int128`, `compare_uint128`, `compare_mixed`)
   - Return `int` results based on 128-bit comparisons
   - Test cases cover all branches (high-word less/greater, low-word less/greater, equality)

5. **Complex Expressions**: 
   - `compare_with_overflow()` chains multiple comparisons with logical operators
   - Main function includes `if ((x < y) && (y < z) && (x != z))`
   - Arithmetic operations (addition, subtraction, multiplication) followed by comparisons

**Compilation recommendations:**
