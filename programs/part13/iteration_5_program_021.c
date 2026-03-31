This code demonstrates a classic **phi (φ) function** scenario in compiler theory, specifically in **Static Single Assignment (SSA) form**.

## What's happening:

1. **Two possible definitions of `val`**: 
   - `val = some_computation()` (when `input > 10`)
   - `val = another_computation()` (when `input ≤ 10`)

2. **Merge point**: After the if-else, both control flow paths merge, but `val` could have different values from each path.

3. **SSA requires unique assignments**: In SSA form, each variable can only be assigned once. To handle this merge, the compiler creates a **phi function**:

## In SSA form, this becomes:

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

- **Phi function (`%val = phi ...`)**: At the merge block, it selects which value to use based on which predecessor block we came from
- **Unique names**: `val_then` and `val_else` are distinct SSA variables
- **Single assignment**: Each SSA variable (`%val_then`, `%val_else`, `%val`) is assigned exactly once
- **Control flow aware**: The phi function encodes the control flow dependence

This is essential for many compiler optimizations because it makes data flow explicit and eliminates ambiguity about which definition reaches a use.
