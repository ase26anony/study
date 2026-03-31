**Key features that target the uncovered lines:**

1. **Mixed argument counts**: 
   - One argument: `sqrt(25.0)`, `ceil(4.2)`, `exp(1.0)`, `sin(1.570796)`
   - Two arguments: `pow(2.0, 3.0)`, `fmod(10.5, 3.0)`, `__builtin_pow(3.0, 2.0)`
   - Zero arguments: `rand()` (though not constant)

2. **Integer-valued real results**:
   - `sqrt(25.0)` = 5.0
   - `pow(2.0, 3.0)` = 8.0
   - `ceil(4.2)` = 5.0
   - `floor(9.9)` = 9.0
   - `cos(0.0)` = 1.0
   - `log(1.0)` = 0.0

3. **Constant evaluation contexts**:
   - Static initializers (`static const double`)
   - Array sizes using casts to int
   - Constexpr function calls
   - Conditional expressions with constant conditions

4. **Built-in and library functions**:
   - GCC built-ins: `__builtin_sqrt`, `__builtin_pow`, `__builtin_round`
   - Standard library: `sqrt`, `pow`, `ceil`, `floor`, `exp`, `log`

**Compilation recommendations:**
