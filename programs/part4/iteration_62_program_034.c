Looking at this code snippet, I can see several optimization opportunities:

## 1. **Copy Propagation**
The chain `x → y → z` can be eliminated:
- `y = x` → `y` can be replaced with `x` everywhere
- `z = y` → `z` can be replaced with `x` everywhere
- After propagation: `if (x == 0) { ... }`

## 2. **Constant Propagation** (if `phi_result` is known)
If `phi_result` is a compile-time constant (e.g., from a phi node merging constants), then:
- `x` becomes constant
- The condition `if (x == 0)` can be evaluated at compile time
- The entire if-statement can be eliminated (dead code elimination)

## 3. **SSA-based Optimizations**
Since this is in SSA form:
- Each variable (`x`, `y`, `z`) is assigned only once
- The copy chain is trivial to track
- The phi node result `phi_result` might have additional constraints that could enable further optimizations

## 4. **Condition Simplification**
After copy propagation:
- `if (x == 0)` might be simplified based on known value ranges of `x`
- If `x` is known to be non-zero (from phi node analysis), the condition is always false
- If `x` is known to be zero, the condition is always true

## Potential Optimized Result:
Depending on what `phi_result` is:
1. **If `phi_result` is constant 0**: Entire if-statement becomes always true, can be replaced with unconditional jump
2. **If `phi_result` is constant non-zero**: Entire if-statement becomes always false, can be removed
3. **If `phi_result` is unknown**: Just `if (phi_result == 0) { ... }` after copy propagation

The key insight is that in SSA form, copy propagation is trivial and should always be performed when possible, often enabling further constant propagation and dead code elimination.
