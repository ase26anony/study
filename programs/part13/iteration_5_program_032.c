## Key Points:

1. **Two Definitions**: `val` has two possible definitions:
   - `some_computation()` if `input > 10`
   - `another_computation()` if `input ≤ 10`

2. **Merge Point**: At the `merge` block, the compiler inserts a **phi node** (`%val = phi ...`) that selects between `%val_then` and `%val_else` based on which predecessor block was executed.

3. **Single Assignment**: Despite having two possible sources, `%val` in SSA form is assigned exactly once (by the phi node).

## Why This Matters:

- **Optimization**: Phi nodes make data flow explicit, enabling better optimizations
- **Analysis**: Compilers can easily track where values come from
- **Register Allocation**: Helps determine live ranges of variables

## In Your Original Code:
