**Key features that target the uncovered lines:**

1. **Mixed Signed/Unsigned Contexts**: The `compare_mixed()` function and compound conditions compare `__int128` with `unsigned __int128`, which should trigger the `(unsigned HOST_WIDE_INT)` casting in the uncovered code.

2. **High-Word Boundary Testing**: Values like `boundary_val = (__int128)1 << 63` are exactly at the 64-bit boundary, forcing comparisons where the high word matters.

3. **Loop-Based Range Testing**: The loop from `-10` to `10` multiplies by `1 << 62`, creating values that cross sign boundaries and exercise different comparison paths.

4. **Complex Expressions**: Multiple compound conditions chain comparisons together, like `(x > y && ux < uy)`, which should generate multiple comparison operations.

5. **Direct High/Low Word Comparisons**: Variables `a`, `b`, `c`, `d` are constructed to explicitly test:
   - High word less (`a.high < b.high`)
   - High word greater (`a.high > b.high`) 
   - High word equal with low word less (`c.low < d.low`)
   - High word equal with low word greater
   - Equality

**Compilation recommendations:**
