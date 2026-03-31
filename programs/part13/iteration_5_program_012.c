This code demonstrates a classic **phi (φ) function** scenario in compiler theory, particularly in **Static Single Assignment (SSA) form**.

## What's happening here:

1. **Two possible definitions of `val`**: 
   - `val = some_computation()` when `input > 10`
   - `val = another_computation()` when `input ≤ 10`

2. **Merge point**: After the if-else, both control flow paths converge

3. **SSA transformation**: In SSA form, the compiler creates a **phi function** at the merge point:

```llvm
; Pseudo LLVM IR
entry:
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

## Key points:

- **Phi function** (`%val = phi i32 [ %val_then, %then ], [ %val_else, %else ]`):
  - Chooses which value to use based on which basic block we came from
  - At runtime: if we came from `then`, use `%val_then`; if from `else`, use `%val_else`

- **SSA benefits**:
  - Each variable assigned exactly once
  - Enables better optimizations (constant propagation, dead code elimination, etc.)
  - Makes data flow analysis easier

- **Original vs SSA**: The original code has one `val` variable with two possible definitions, while SSA creates two distinct variables (`val_then`, `val_else`) and merges them with a phi.

This pattern is fundamental to how compilers reason about control flow and variable definitions in programs with branches.
