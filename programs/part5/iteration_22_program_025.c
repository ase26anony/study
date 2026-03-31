This code appears to be part of a compiler optimization pass that transforms relational comparisons into bitwise operations. Let me analyze what's happening:

## Context
This is likely from GCC's middle-end optimization passes, specifically transforming comparisons of boolean values or bitmask operations.

## What it does

For each comparison type:
- **GT_EXPR** (`>`): Transforms `a > b` into `~(a & ~b)` or similar
- **GE_EXPR** (`>=`): Transforms `a >= b` into `~(a | ~b)` or similar  
- **LT_EXPR** (`<`): Transforms `a < b` into `~(b & ~a)` (after swapping operands)
- **LE_EXPR** (`<=`): Transforms `a <= b` into `~(b | ~a)` (after swapping operands)

## The pattern

The transformation follows De Morgan's laws and boolean algebra:
- `a > b` is equivalent to `a & ~b` (for single-bit booleans)
- `a >= b` is equivalent to `a | ~b` (for single-bit booleans)

The `BIT_NOT_EXPR` might be applied to the entire result or part of it, depending on the surrounding context.

## Why swap operands?

For `<` and `<=`, the code swaps `cond_expr0` and `cond_expr1` to reuse the same transformation logic:
- `a < b` becomes `b > a` (after swap, use GT_EXPR logic)
- `a <= b` becomes `b >= a` (after swap, use GE_EXPR logic)

## Use case

This optimization is useful when:
1. Comparing boolean values or single-bit flags
2. Working with bitmask operations
3. The target architecture has efficient bitwise operations but less efficient branches

## Example

If we have `(a > b)` where `a` and `b` are booleans:
- Original: `a > b`
- Transformed: `~(a & ~b)` or `a & ~b` (depending on exact implementation)

This allows the compiler to avoid conditional jumps and use bitwise operations instead, which can be faster on modern CPUs with good bit manipulation support.
