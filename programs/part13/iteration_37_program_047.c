This program combines multiple techniques to exercise the partition code mapping:

1. **Template Metaprogramming**: Specialized templates for partition codes 0-7 generate distinct OpenACC constructs with different partitioning clauses.

2. **Runtime Exhaustive Testing**: Tests codes from -1 to 9 using volatile variables to prevent constant folding, ensuring all switch cases (including default) are compiled.

3. **Multi-dimensional Arrays**: Uses 3D arrays with broadcasting patterns to trigger complex partitioning decisions.

4. **Mixed Partitioning**: Combines gang, worker, and vector clauses in various combinations to stress the compiler's internal representation.

5. **Redundant Clauses**: Explicitly uses `redundant` clause for the gang redundant case (code 0).

6. **Present/Copy Clauses**: Uses both `copy` and `present_or_copyin` to exercise different data movement paths.

**Compilation commands:**
