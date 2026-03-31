This program includes:

1. **Complex inline assembly with multiple constraints** (`test_complex_constraints`): Uses alternative constraints (`"=r,m"`, `"r,m,i"`) and explicit hard register clobbers to force various reload types.

2. **Nested address computations** (`test_nested_addresses`): Takes addresses of volatile array elements with complex index calculations, potentially triggering `RELOAD_FOR_INPUT_ADDRESS` and `RELOAD_FOR_INPADDR_ADDRESS`.

3. **Register variables with explicit binding** (`test_register_variables`): Binds variables to specific registers (r10-r13) and uses them in assembly, forcing the reload pass to handle fixed registers.

4. **Large number of operands** (`test_many_operands`): A single `asm` statement with 10+ operands, mixing integer and float (via `__builtin_bit_cast`) types.

5. **`__builtin_constant_p` usage** (`test_builtin_constant`): Uses constant detection to choose between different addressing modes, potentially triggering `RELOAD_FOR_OPERAND_ADDRESS`.

6. **Mixed address types** (`test_mixed_address_types`): Uses pointer-to-pointer indirection and volatile addresses to stress address reload logic.

**Compilation recommendations:**
