This code snippet demonstrates a common pattern in compiler intermediate representations (IR) where **PHI nodes** (Φ nodes) are used to handle control flow merges.

## What's happening here:

1. **Control flow split**: The first `if (cond)` creates two possible execution paths
2. **Variable assignment**: `val` gets different values in each branch (1 or 2)
3. **Control flow merge**: After the if-else, execution paths converge
4. **PHI node creation**: At the merge point, the compiler creates a PHI node that selects the correct value of `val` based on which path was taken

## In SSA (Static Single Assignment) form:

```llvm
; Before the if-else
br i1 %cond, label %then, label %else

then:
  %val_then = 1
  br label %merge

else:
  %val_else = 2
  br label %merge

merge:
  %val = phi i32 [ %val_then, %then ], [ %val_else, %else ]
  ; Now %val contains either 1 or 2 depending on the path taken
  %cmp = icmp eq i32 %val, 1
  br i1 %cmp, label %if_true, label %if_false
```

## Key points:

- **SSA form** requires each variable to be assigned exactly once
- **PHI nodes** are special instructions that select values based on which control flow path was taken
- The `%val` in the second `if (val == 1)` refers to the PHI node's result, not either `%val_then` or `%val_else` directly
- This allows the compiler to perform optimizations while maintaining correct program semantics

This pattern is fundamental to how compilers like LLVM, GCC, and others represent programs in their intermediate representations for optimization purposes.
