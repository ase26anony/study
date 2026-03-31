This code snippet demonstrates a classic **phi node** scenario in compiler intermediate representations (like SSA form). Here's what's happening:

## Control Flow Analysis

1. **Two possible definitions of `val`**:
   - `val = some_computation()` (when `input > 10`)
   - `val = another_computation()` (when `input ≤ 10`)

2. **Merge point**: After the if-else, both control flow paths converge
3. **Single use**: The second `if` statement uses `val`, which could come from either path

## In SSA (Static Single Assignment) Form

The compiler would transform this to use a **phi function** at the merge point:

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

## Key Points

- **Phi node** (`φ`): Selects a value based on which control flow path was taken
- **Single assignment**: Each variable (`val_then`, `val_else`, `val`) is assigned exactly once
- **Merge dominance**: The `merge` block dominates all uses of `val` after it
- **Compiler optimization**: This representation makes data flow explicit, enabling optimizations like:
  - Constant propagation
  - Dead code elimination
  - Value numbering

This pattern is fundamental to how compilers reason about values that come from different control flow paths.
