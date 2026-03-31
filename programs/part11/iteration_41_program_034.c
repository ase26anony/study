Looking at this code, I can see it's a loop where `val` is set to either 0 or 1 in each iteration based on `some_condition(i)`. The key insight is that `val` is always defined in the loop body before being used, so at the start of each iteration, `val` needs a phi node to merge the value from the previous iteration with the initial value (0).

Here's how this would look in SSA form with phi nodes:

```llvm
entry:
  br label %loop

loop:
  %val.phi = phi i32 [ 0, %entry ], [ %val, %loop.end ]
  %i = phi i32 [ 0, %entry ], [ %i.next, %loop.end ]
  
  ; Check some_condition(i)
  %cond = call i1 @some_condition(i32 %i)
  br i1 %cond, label %if.true, label %if.false

if.true:
  br label %merge

if.false:
  br label %merge

merge:
  %val = phi i32 [ 1, %if.true ], [ 0, %if.false ]
  
  ; Check if val == 1
  %cmp = icmp eq i32 %val, 1
  br i1 %cmp, label %do.work, label %loop.end

do.work:
  ; do work
  br label %loop.end

loop.end:
  %i.next = add i32 %i, 1
  %loop.cond = icmp slt i32 %i.next, %n
  br i1 %loop.cond, label %loop, label %exit

exit:
  ; ...
```

The pattern you mentioned is in the `merge` block where we have:
- `%val = phi i32 [ 1, %if.true ], [ 0, %if.false ]`

And then immediately after in the same block:
- `%cmp = icmp eq i32 %val, 1`

This is indeed the pattern where:
1. `val` is defined by a phi node
2. Immediately compared with a constant (1)
3. The phi node has incoming values that are constants (1 and 0)

This pattern could potentially be optimized by:
1. Directly using the condition from `some_condition(i)` instead of materializing `val` as 0/1
2. Or simplifying the comparison since we know `val` can only be 0 or 1

For example, the comparison `val == 1` is equivalent to just using the boolean result of `some_condition(i)` directly, since `val` is set to 1 when `some_condition(i)` is true and 0 when it's false.
