This program includes:

1. **Complex inline assembly with alternative constraints** (`"=r,m"`, `"r,m,i"`) and explicit hard register clobbers to force various reload types.

2. **Nested address computations** with volatile arrays, triggering address reloads for both input and output addresses.

3. **Large multi-operand asm statements** with 10+ operands, mixing different types and register-bound variables to maximize register pressure.

4. **`__builtin_constant_p` usage** in address contexts to create conditional address expressions that may require different reload handling.

5. **Register-bound variables** (`register ... asm("rXX")`) that tie specific variables to hard registers, forcing the reload pass to work around these constraints.

6. **Mixed integer/float operations** using `std::bit_cast` (C++20) to create additional type constraints.

**Recommended compilation commands to maximize reload coverage:**
