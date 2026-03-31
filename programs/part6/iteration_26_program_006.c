## Key Design Elements:

1. **High Register Pressure**: Multiple nested loops with many live variables of different types (int, double, float, long long, pointers).

2. **Pure Functions**: `pure_compute` and `pure_double` marked with `__attribute__((const))` to encourage rematerialization instead of spilling.

3. **Complex Control Flow**: Uses `goto`, `switch`, and nested conditionals to create non-trivial CFG that inhibits CSE and LICM.

4. **Mixed Data Types**: Structure with different-sized members forces varied register modes.

5. **Inline Assembly**: Creates artificial register references that the compiler must manage.

6. **Memory Barriers**: `asm volatile` with memory clobber prevents unwanted optimizations.

7. **Register Hints**: `register` keyword on some variables to influence allocation.

8. **Varied Addressing Modes**: Array indexing, structure member access, pointer arithmetic.

## Compilation Recommendations:
