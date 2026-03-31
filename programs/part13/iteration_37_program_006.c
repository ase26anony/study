This program:

1. **Exhaustively tests partition codes 0-7** through the runtime loop with `code` from -1 to 9
2. **Uses template metaprogramming** to generate compile-time specializations for each partition code
3. **Employs volatile variables** to prevent constant folding and ensure all branches are compiled
4. **Creates complex nested loops** with multi-dimensional arrays for broadcasting scenarios
5. **Mixes redundant and partitioned clauses** to trigger different internal representations
6. **Uses both `parallel` and `kernels` constructs** to exercise different code generation paths
7. **Includes illegal partition codes** (-1, 8, 9) to trigger the default case
8. **Performs a final reduction** to prevent dead code elimination

Compile with:
