**Key features that should trigger the uncovered function:**

1. **Multiple partitioning patterns**: The code uses `gang worker vector`, `gang vector`, and `worker vector` clauses across different parallel regions, which should generate different integer partitioning codes.

2. **Complex data clauses**: Each region uses combinations of `private`, `firstprivate`, `reduction`, and `present` clauses on multi-dimensional arrays, forcing the compiler to analyze data partitioning strategies.

3. **Nested loops with collapse**: The `collapse(2)` clause on multi-dimensional loops creates complex iteration spaces that require partitioning decisions.

4. **Conditional data access**: Multiple `if` conditions with different access patterns (`(i + j) % 3`, `i % 2`, `j % 3`, etc.) create data-dependent access patterns that may affect broadcasting/neutering decisions.

5. **Mixed OpenACC/OpenMP**: Pure OpenMP regions outside OpenACC data regions stress the compiler's ability to manage multiple offloading paradigms.

6. **Persistent device data**: `global_matrix` and `global_stats` are declared with `#pragma acc declare create`, requiring persistent device memory management and partitioning.

7. **Dynamic loop bounds**: Using `argc` and `volatile` variables prevents constant folding and forces runtime analysis.

**Recommended compilation commands to maximize coverage:**
