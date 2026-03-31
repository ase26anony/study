## Key Design Elements:

1. **High Register Pressure**: The functions declare many local variables of different types (int, double, float, pointers) and use them in complex expressions within nested loops.

2. **Pure Functions**: `pure_compute()` is marked with `__attribute__((const))` to encourage the compiler to consider rematerializing its calls instead of spilling results.

3. **Control Flow Complexity**: Uses `goto`, `switch` statements, and nested loops to create a non-trivial control flow graph that inhibits value numbering and CSE.

4. **Mixed Data Types**: Uses structs with mixed types (int, double, float, short, pointer) to create varied register modes (`GET_MODE (*loc)`).

5. **Inline Assembly**: Creates explicit register references that the compiler must manage, increasing pressure on specific registers.

6. **Register Hints**: Uses `register` variables with specific register constraints to influence allocation.

7. **Volatile and Optimization Pragmas**: Uses `volatile` and `#pragma GCC optimize` to control optimization decisions.

8. **Noinline Attributes**: Prevents function inlining to preserve the intended code structure.

## Compilation Instructions:

To test coverage of the early-remat.cc lines:
