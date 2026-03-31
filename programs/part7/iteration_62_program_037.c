## Key Design Elements:

1. **Decrementing Loop Patterns**: Each test function implements a different variant of decrementing loops with zero comparisons:
   - `for (int i = N; i != 0; i--)`
   - `while (n-- > 0)`
   - `do { ... } while (--count)`
   - Explicit `i -= 1` with `if (i == 0) break`

2. **Register Pressure**: Each function creates many temporary variables before and inside the loop, forcing the compiler to use hardware registers for the loop counter.

3. **Control Flow in Loop Bodies**: Each loop contains `if` statements and conditional logic, making the loops non-trivial but still optimizable.

4. **Post-Loop Counter Usage**: Each function uses the loop counter after the loop ends, preventing dead code elimination.

5. **`noinline` Attribute**: Prevents interprocedural optimizations from altering the loop structure.

6. **Volatile Variables**: Used strategically to prevent optimization of register pressure variables and loop computations.

## Compilation Commands:
