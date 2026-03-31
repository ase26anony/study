## Key Design Elements:

1. **Constant Bounds Generation**: Uses `constexpr` template functions and `static_assert` to ensure compile-time constant indices.

2. **Memory vs Non-Memory Targets**:
   - `memory_target_operations()`: Direct array accesses and copies (MEM_P path)
   - `non_memory_target_operations()`: Arithmetic operations with array elements (non-MEM_P path)

3. **Count Value Control**:
   - `count = 1`: Single element access (`lo=5, hi=5`)
   - `count = 2`: Two-element range (`lo=10, hi=11`)
   - `count > 2` with small elements: 6 char elements (`lo=20, hi=25`)
   - `count > 2` with large elements: 6 long long elements (`lo=30, hi=35`)

4. **Volatile Inhibition**: Uses `volatile` pointers and `VolatileWrapper` to prevent early constant folding.

5. **Multiple Control Paths**: Uses `argc` to select different scenarios, ensuring the compiler analyzes multiple paths.

6. **Built-in Functions**: Includes `__builtin_memcpy` with constant sizes to trigger array block operations.

## Recommended Compilation:
