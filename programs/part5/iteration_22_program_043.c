This code appears to be part of a compiler optimization pass that transforms relational expressions into bitwise operations. Let me analyze what's happening:

## Context
This is likely from GCC or another C/C++ compiler's middle-end optimization, where relational expressions are being converted to bitwise operations for efficiency or to enable further optimizations.

## Transformation Logic

### For `GT_EXPR` (greater than):
- `bitop1 = BIT_NOT_EXPR` (bitwise NOT)
- `bitop2 = BIT_AND_EXPR` (bitwise AND)
- No swap of operands

### For `GE_EXPR` (greater than or equal):
- `bitop1 = BIT_NOT_EXPR` (bitwise NOT)
- `bitop2 = BIT_IOR_EXPR` (bitwise OR)
- No swap of operands

### For `LT_EXPR` (less than):
- Same operations as `GT_EXPR` (`BIT_NOT_EXPR` + `BIT_AND_EXPR`)
- **But** swaps `cond_expr0` and `cond_expr1`
- This makes sense because `a < b` is equivalent to `b > a`

### For `LE_EXPR` (less than or equal):
- Same operations as `GE_EXPR` (`BIT_NOT_EXPR` + `BIT_IOR_EXPR`)
- **But** swaps `cond_expr0` and `cond_expr1`
- This makes sense because `a ≤ b` is equivalent to `b ≥ a`

## Mathematical Reasoning

The pattern suggests this is implementing comparisons using two's complement arithmetic. For signed integers in two's complement:

- `x > y` ≡ `~(x - y) & ~(x ^ y) & msb_mask` (simplified)
- `x ≥ y` ≡ `~(x - y) | ~(x ^ y) & msb_mask` (simplified)

The `std::swap` for LT/LE cases handles the symmetry: comparing `a < b` is the same as comparing `b > a` with swapped operands.

## Purpose
This transformation likely enables:
1. Better instruction selection on certain architectures
2. Common subexpression elimination opportunities
3. Vectorization possibilities
4. Constant propagation through bitwise operations

The code is elegantly using symmetry to reduce the number of distinct code paths needed.
