This code demonstrates a classic **phi (φ) function** scenario in compiler theory, particularly in **Static Single Assignment (SSA) form**.

## What's Happening Here:

1. **Two possible definitions of `val`**: 
   - `val = some_computation()` (when `input > 10`)
   - `val = another_computation()` (when `input ≤ 10`)

2. **Merge point**: After the if-else, both control flow paths converge

3. **Single use of `val`**: The second `if` statement uses `val`, which could come from either definition

## In SSA Form:

When converted to SSA, the compiler creates:

```llvm
; Pseudo SSA representation
entry:
  %input = ...
  %cmp = icmp sgt i32 %input, 10
  br i1 %cmp, label %then, label %else

then:
  %val_then = call i32 @some_computation()
  br label %merge

else:
  %val_else = call i32 @another_computation()
  br label %merge

merge:
  %val = phi i32 [ %val_then, %then ], [ %val_else, %else ]
  %cmp2 = icmp eq i32 %val, 0
  br i1 %cmp2, label %if_true, label %if_false
```

## Key Points:

- **Phi function (`φ`)**: At the merge point, `%val = phi i32 [%val_then, %then], [%val_else, %else]` selects which value to use based on which basic block we came from
- **SSA requirement**: Each variable is assigned exactly once
- **Control flow dependence**: The value of `val` depends on which path was taken
- **Data flow**: The phi node merges values from different control flow paths

This pattern is fundamental to how compilers reason about programs with conditional assignments and is crucial for many optimizations.
