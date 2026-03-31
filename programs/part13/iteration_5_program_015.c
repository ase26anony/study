## Key Points:

1. **Control Flow Merge**: The two branches of the first `if` converge, creating a merge point
2. **Variable Re-definition**: `val` is defined in both branches
3. **Phi Function**: At the merge point, a φ function conceptually "chooses" which definition of `val` to use based on which path was taken
4. **Downstream Use**: The second `if` uses the merged value of `val`

## Why This Matters for Compilers:

- **Optimization**: The φ function makes data flow explicit, enabling better optimizations
- **Analysis**: Compilers can track where values come from more easily
- **Code Generation**: The φ function gets lowered to moves/copies in the final machine code

## Equivalent Lowered Code (non-SSA):
