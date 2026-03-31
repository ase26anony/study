**Key Design Elements:**

1. **High Register Pressure**: Multiple functions with many live variables of different types (int, double, float, pointers, struct members) competing for registers.

2. **Pure Functions**: `pure_compute()` marked with `__attribute__((const))` to encourage rematerialization instead of spilling.

3. **Complex Control Flow**: Uses `goto`, `switch`, nested loops, and labels to create non-trivial CFGs that inhibit CSE and LICM.

4. **Mixed Data Types and Addressing**: Accesses struct members of different sizes, array elements, and performs pointer arithmetic to create varied `GET_MODE` requirements.

5. **Inline Assembly**: Forces specific register usage with constraints, creating artificial register pressure.

6. **Compiler Pragmas**: Uses `#pragma GCC unroll` and `optimize` attributes to control optimization levels.

7. **Register Variables**: Hints at register allocation with `register` variables and asm constraints.

**Compilation Recommendations:**
