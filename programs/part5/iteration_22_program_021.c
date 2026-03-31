This code appears to be part of a compiler optimization pass that transforms relational expressions into bitwise operations. Let me explain what's happening:

## Context
This is likely from GCC or a similar compiler's middle-end optimization phase, where it's converting comparison operations into equivalent bitwise operations for optimization purposes.

## What the code does

For each comparison operator:
- `GT_EXPR` (greater than): Converts `a > b` to bitwise operations
- `GE_EXPR` (greater than or equal): Converts `a >= b` to bitwise operations  
- `LT_EXPR` (less than): Converts `a < b` to bitwise operations
- `LE_EXPR` (less than or equal): Converts `a <= b` to bitwise operations

## The transformation logic

The pattern seems to be:
1. **`GT_EXPR`**: `a > b` becomes `!(a & ~b)` (using BIT_NOT_EXPR and BIT_AND_EXPR)
2. **`GE_EXPR`**: `a >= b` becomes `!(a | ~b)` (using BIT_NOT_EXPR and BIT_IOR_EXPR)
3. **`LT_EXPR`**: `a < b` becomes `!(b & ~a)` (same as GT but with swapped operands)
4. **`LE_EXPR`**: `a <= b` becomes `!(b | ~a)` (same as GE but with swapped operands)

## Why swap operands for LT/LE?

The `std::swap(cond_expr0, cond_expr1)` for `LT_EXPR` and `LE_EXPR` is clever:
- `a < b` is equivalent to `b > a`
- `a <= b` is equivalent to `b >= a`

So instead of creating new bitwise patterns for LT/LE, it reuses the GT/GE patterns by swapping the operands.

## Example

For `x < y`:
- Original: `x < y`
- After swap: `y > x` (treating it as GT_EXPR with swapped operands)
- Transformation: `!(y & ~x)` (which is equivalent to `x < y`)

This is a common optimization technique in compilers to reduce the number of code patterns needed and leverage existing transformations.
