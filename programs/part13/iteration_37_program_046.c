## Key Features:

1. **Exhaustive Partition Code Coverage**: Tests codes -1 through 9, covering all valid (0-7) and illegal values.

2. **Template Metaprogramming**: The `PartitionGenerator` template is explicitly instantiated for all partition codes 0-7 plus illegal codes 8 and -1, forcing the compiler to generate code for all cases at compile time.

3. **Mixed Partitioning Directives**: Uses combinations of `gang`, `worker`, `vector`, and `redundant` clauses in nested loops to trigger different internal partition codes.

4. **Multi-dimensional Array Broadcasting**: Uses a 3D array with complex access patterns to exercise broadcast operations.

5. **Volatile Variables**: Uses `volatile int partition_selector` to prevent constant folding and ensure all code paths remain in the compiled binary.

6. **Runtime Switch Statement**: The main loop contains a `switch` statement that selects different OpenACC loop structures based on the partition code, ensuring all cases are present in the final executable.

7. **Prevention of Dead Code Elimination**: Final reductions and output ensure all computations are used.

## Compilation Commands:
