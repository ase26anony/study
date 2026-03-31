This program systematically exercises the uncovered comparison logic by:

1. **Using constants that exercise all comparison paths:**
   - High parts differ, low parts equal (`U128_HIGH_DIFF_LOW_EQ_A` vs `U128_HIGH_DIFF_LOW_EQ_B`)
   - High parts equal, low parts differ (`U128_HIGH_EQ_LOW_DIFF_A` vs `U128_HIGH_EQ_LOW_DIFF_B`)
   - Both parts differ (`U128_BOTH_DIFF_A` vs `U128_BOTH_DIFF_B`)
   - Signed values with sign bits (`S128_NEG_ONE` vs `S128_ZERO`)

2. **Embedding comparisons in compile-time contexts:**
   - `static_assert` for compile-time verification
   - `constexpr` variables and functions
   - Array sizes based on comparisons
   - Template metaprogramming with `__int128` values

3. **Including runtime comparisons:**
   - Conditional branches with `if` statements
   - Loop conditions with `for` and `while`
   - Switch statements based on comparison results
   - `volatile` variables to prevent optimization

4. **Using GCC built-ins:**
   - `__builtin_mul_overflow` with 128-bit values
   - `__builtin_add_overflow_p` for overflow checking

5. **Mixing signed and unsigned comparisons:**
   - Both `__int128` (signed) and `unsigned __int128` comparisons
   - Values with sign bits set to test unsigned high-part comparison

**Compilation recommendations:**

For C++ compilation with constexpr evaluation:
