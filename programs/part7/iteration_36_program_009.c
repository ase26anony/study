**Key Design Elements:**

1. **Volatile Control Flow**: Each test function uses `volatile int cond` parameter in an `if` statement, ensuring the parser must analyze the erroneous code block.

2. **Isolated Functions**: Each function targets exactly one keyword case with `__attribute__((noinline, noipa))` to prevent inlining and inter-procedural analysis.

3. **Valid Context First**: Each function starts with `int local = 0;` to establish normal parsing state before the error.

4. **Conditional Execution in Main**: The loop with `volatile int iterations` ensures all functions are compiled and their bodies parsed.

5. **Global Side Effect**: `global_counter` prevents dead code elimination across the entire program.

**Compilation Recommendations:**

For maximum parser path coverage:
