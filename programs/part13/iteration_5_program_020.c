This code demonstrates a classic **phi node** scenario in compiler intermediate representation (IR). Here's what's happening:

## Control Flow Analysis

1. **Branching Paths**: The first `if` creates two separate execution paths
2. **Variable Assignment**: `val` gets assigned in both branches but with potentially different values
3. **Merge Point**: After the first `if-else`, control flow merges back together
4. **Value Usage**: The second `if` uses `val`, which could come from either branch

## Phi Node Concept

In compiler IR (like LLVM's SSA form), this would be represented with a **phi node**:

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

## Key Points

- **SSA Form**: Static Single Assignment requires each variable be assigned once
- **Phi Resolution**: At the merge point, `%val` "chooses" which value to use based on which path was taken
- **Runtime Decision**: The actual value of `val` isn't known until runtime when the first condition is evaluated

## Optimization Considerations

Compilers can optimize this by:
- **Constant Propagation**: If either computation returns a known constant
- **Branch Prediction**: Based on profiling or heuristics about `input > 10`
- **Code Motion**: Moving invariant computations out of branches if possible

This pattern is fundamental to how compilers handle control flow in SSA-based optimizations.
