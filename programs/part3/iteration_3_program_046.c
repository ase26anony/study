**Key features that target the uncovered lines:**

1. **Mixed argument counts**: 
   - Two arguments: `pow`, `fmod`, `remainder`, `fmax`, `fmin`, `ldexp`
   - One argument: `ceil`, `floor`, `sqrt`, `trunc`, `round`, `exp`, `log`, `sin`, `cos`
   - Zero arguments: `rand()` (though not constant)

2. **Integer-valued real results**: Most calls produce results like 2.0, 3.0, 0.0, 1.0, etc., which are real numbers with integer values.

3. **Constant expressions**: 
   - `static_val` is a compile-time constant
   - `arr_size` uses `constexpr` with `ceil(6.3)`
   - Many variables are initialized with constant expressions

4. **Complex expressions**: Ternary operators and nested function calls create deeper expression trees.

5. **GCC built-ins**: `__builtin_sqrt` and `__builtin_pow` directly test GCC's internal handling.

**Compilation recommendations:**
