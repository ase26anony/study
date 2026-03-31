Looking at this code, I can see the pattern you're describing. In SSA (Static Single Assignment) form, the variable `val` would indeed be represented by a Phi node at the beginning of each loop iteration because:

1. **SSA Form Requirements**: Each variable can only be assigned once
2. **Loop Structure**: The loop creates a control flow merge point at the start of each iteration
3. **Multiple Definitions**: `val` can be assigned either `1` or `0` in the previous iteration

Here's how this would look in SSA form:

```llvm
; Pseudo SSA representation
entry:
  br label %loop.header

loop.header:
  %i = phi i32 [0, %entry], [%i.next, %loop.latch]
  %val = phi i32 [0, %entry], [%val.next, %loop.latch]  ; ← This is the Phi node
  %cmp = icmp slt i32 %i, %n
  br i1 %cmp, label %loop.body, label %loop.exit

loop.body:
  %cond = call i1 @some_condition(i32 %i)
  br i1 %cond, label %then, label %else

then:
  br label %merge

else:
  br label %merge

merge:
  %val.next = phi i32 [1, %then], [0, %else]  ; ← This defines value for next iteration
  ; Here we have the comparison: %val == 1
  %cmp.val = icmp eq i32 %val, 1  ; ← This matches your pattern
  br i1 %cmp.val, label %do.work, label %loop.latch

do.work:
  ; do work
  br label %loop.latch

loop.latch:
  %i.next = add i32 %i, 1
  br label %loop.header

loop.exit:
  ; ...
```

**Key observations about your pattern:**

1. **Phi Node Identification**: The variable `val` becomes `%val = phi i32 [0, %entry], [%val.next, %loop.latch]` at the loop header

2. **Pattern Match**: The comparison `if (val == 1)` becomes `%cmp.val = icmp eq i32 %val, 1` where:
   - LHS (`%val`) is an SSA name defined by a Phi node
   - RHS (`1`) is a constant

3. **Control Flow**: The Phi node merges values from:
   - Initial value (`0` from entry)
   - Previous iteration's result (`%val.next` from loop latch)

This is a classic pattern in compiler optimization where recognizing such Phi-constant comparisons can enable optimizations like:
- Loop-invariant code motion
- Dead code elimination
- Conditional constant propagation
- Loop unswitching (if the condition is loop-invariant)
