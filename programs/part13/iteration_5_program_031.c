This code demonstrates a classic **phi (φ) function** scenario in compiler theory, particularly in **Static Single Assignment (SSA) form**.

## What's happening here:

1. **Two possible definitions of `val`**: 
   - `val = some_computation()` (if `input > 10`)
   - `val = another_computation()` (if `input ≤ 10`)

2. **Merge point**: After the if-else, both control flow paths converge

3. **SSA requires unique assignments**: In SSA form, each variable can only be assigned once. Since `val` gets assigned in two different places, the compiler creates a **phi function** at the merge point:

## In SSA form, this becomes:
```llvm
; Pseudo-LLVM IR
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
- **Phi function (`φ`)**: At the merge block, `%val` gets its value from either `%val1` or `%val2` depending on which path was taken
- **Single assignment**: Each SSA variable (`%val1`, `%val2`, `%val`) is assigned exactly once
- **Control flow dependence**: The phi function encodes the control flow merge

This is a fundamental concept in:
- **Compiler optimizations**: Enables better data flow analysis
- **Program analysis**: Makes dependencies explicit
- **Just-in-Time compilation**: Used in LLVM, Java HotSpot, etc.

The phi function essentially says: "`val` gets the value from the predecessor block that actually executed."
