This code demonstrates a classic **phi (φ) function** scenario in compiler theory, particularly in **Static Single Assignment (SSA) form**.

## What's happening here:

1. **Two possible definitions of `val`**: 
   - `val = some_computation()` (if `input > 10`)
   - `val = another_computation()` (if `input ≤ 10`)

2. **Merge point**: After the if-else, both control flow paths converge

3. **Single use**: The second `if` statement uses `val`, which could come from either definition

## In SSA form:

The compiler would transform this to:
```llvm
if (input > 10) {
    val1 = some_computation();
} else {
    val2 = another_computation();
}
val = φ(val1, val2);  // Phi function: chooses val1 or val2 based on which path was taken

if (val == 0) { ... }
```

## Key points:

- **Phi function (φ)**: A special operation at control flow merge points that selects which value to use based on which predecessor block was executed
- **Single assignment**: Each variable (`val1`, `val2`) is assigned only once
- **Merge variable**: `val` gets its value from a phi function, not a direct assignment

## Why this matters for compilers:

1. **Optimization**: SSA makes many optimizations easier (constant propagation, dead code elimination, etc.)
2. **Analysis**: Simplifies data flow analysis since each variable has a single definition point
3. **Code generation**: Modern compilers (LLVM, GCC) use SSA internally for optimization

The phi function is essentially a "merge" operation that happens implicitly in the original code but explicitly in SSA form.
