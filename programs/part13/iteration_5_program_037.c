This code demonstrates a classic **phi (φ) function** scenario in compiler theory, particularly in **Static Single Assignment (SSA) form**.

## What's happening here:

1. **Two possible definitions of `val`**: 
   - `val = some_computation()` (when `input > 10`)
   - `val = another_computation()` (when `input ≤ 10`)

2. **Merge point**: After the if-else, both control flow paths converge

3. **Phi function needed**: In SSA form, each variable can only be assigned once. Since `val` gets assigned in two different places (two different basic blocks), the compiler creates a **phi function** at the merge point to select the correct value based on which path was taken.

## In SSA form, this becomes:

```llvm
; Pseudo LLVM IR
entry:
  %cmp = icmp sgt i32 %input, 10
  br i1 %cmp, label %then, label %else

then:
  %val1 = call i32 @some_computation()
  br label %merge

else:
  %val2 = call i32 @another_computation()
  br label %merge

merge:
  %val = phi i32 [ %val1, %then ], [ %val2, %else ]
  %cmp2 = icmp eq i32 %val, 0
  br i1 %cmp2, label %if_true, label %if_false
```

## Key points:

- **`%val` in the merge block** is a phi node that selects between `%val1` and `%val2`
- The phi function takes pairs: `[value, incoming block]`
- This allows the subsequent `if (val == 0)` to use a single SSA value (`%val`) even though it came from two different computation paths
- Without phi functions, SSA would be impossible for code with multiple control flow paths

This is a fundamental concept in compiler optimization and intermediate representations!
