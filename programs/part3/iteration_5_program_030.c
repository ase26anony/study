## Key Design Elements:

1. **Constant Bounds Generation**: Uses template `ConstIndex<N>` to ensure `lo_index` and `hi_index` are compile-time constants visible to the middle-end.

2. **MEM_P Path Testing**: 
   - Direct array stores (`v_char[i] = ...`) create memory targets
   - `__builtin_memcpy` with constant size triggers block copy optimizations

3. **Non-MEM_P Path Testing**: 
   - Arithmetic results stored in local variables (`result = v_arr[lo] + ...`)
   - Computations that produce register values

4. **Count Control**:
   - `count = 2`: `lo1=5, hi1=6`
   - `count = 6`: `lo2=10, hi2=15` (tests both small and large element sizes)

5. **Volatile Inhibition**: `make_volatile()` wrapper prevents early constant folding while keeping indices constant.

6. **Multi-Path Execution**: `argc` controls which code paths execute, ensuring the compiler analyzes multiple control flows.

## Recommended Compilation:
