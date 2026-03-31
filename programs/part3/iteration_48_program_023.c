Looking at this code pattern, I can see you're describing a common SSA (Static Single Assignment) transformation scenario. Let me break down what's happening:

## SSA Analysis

In SSA form, the variable `val` gets:
1. **Initial definition**: `val₀ = 0` (before the loop)
2. **Loop body definitions**:
   - `val₁ = 1` in Path A (when `some_condition(i)` is true)
   - `val₂ = 0` in Path B (when `some_condition(i)` is false)
3. **Phi node**: At the merge point after the if-else, we get `val₃ = φ(val₁, val₂)`

## The Optimization Opportunity

The key insight is that the condition `if (val == 0)` is checking a phi node that merges:
- `1` from Path A
- `0` from Path B

Since the phi node's value depends on which path was taken, the compiler can't trivially determine if `val == 0` is always true or false.

## Possible Optimizations

1. **Condition propagation**: If the compiler can prove that `some_condition(i)` is always false, then `val` is always 0, and the inner `if` can be eliminated.

2. **Loop-invariant code motion**: If `some_condition(i)` doesn't depend on `i`, the entire loop's behavior becomes predictable.

3. **Dead code elimination**: If the "do work" section has no side effects, it might be removable.

## Example of What a Compiler Might See (pseudo-SSA)

```llvm
; Before optimization
val.initial = 0
for i = 0 to n:
  cond = some_condition(i)
  if cond:
    val.A = 1
  else:
    val.B = 0
  val.phi = φ(val.A, val.B)  ; Phi node merging both paths
  if val.phi == 0:
    ; do work
```

## Real Compiler Behavior

Modern compilers (GCC, Clang) would:
1. Build the SSA form
2. Analyze the phi node's possible values
3. Try to simplify the condition based on the phi's incoming values
4. Possibly unroll or vectorize if the pattern is predictable

Is there a specific optimization or compiler behavior you're interested in exploring further?
