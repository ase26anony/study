**Key features that target the uncovered lines:**

1. **Mixed argument counts**: 
   - One argument: `sin(0.0)`, `cos(0.0)`, `exp(1.0)`, `log(1.0)`
   - Two arguments: `pow(2.0, 3.0)`, `fmod(10.0, 2.0)`, `remainder(15.0, 5.0)`

2. **Integer-valued real results**:
   - `sqrt(25.0)` = 5.0
   - `pow(2.0, 3.0)` = 8.0  
   - `ceil(4.2)` = 5.0
   - `floor(9.9)` = 9.0
   - `fmod(10.0, 2.0)` = 0.0

3. **Constant evaluation contexts**:
   - `constexpr` variables (compile-time evaluation)
   - `static const` initializers
   - Array sizes via template
   - Case labels in switch statement
   - Conditional expressions with constant conditions

4. **GCC built-ins**:
   - `__builtin_sqrt`, `__builtin_pow`, `__builtin_floor`, `__builtin_ceil`

5. **Complex expressions**:
   - Nested calls: `floor(pow(2.5, 2.0))`
   - Conditional expressions with function calls in branches

**Compilation recommendations:**
