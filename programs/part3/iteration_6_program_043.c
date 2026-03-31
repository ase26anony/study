## Key points about PHI nodes:

- **SSA requirement**: Each variable can only be assigned once
- **Merge point resolution**: PHI nodes resolve which value to use when control flow merges
- **Compiler optimization**: This pattern allows optimizations like:
  - **Constant propagation**: If `cond` is known at compile-time, `val` becomes constant
  - **Dead code elimination**: Unreachable branches can be removed
  - **Value range analysis**: The compiler can track possible values of `val`

## Without SSA/PHI nodes, the code would need:
