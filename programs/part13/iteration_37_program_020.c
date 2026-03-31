This program systematically exercises all partition codes through:

1. **Exhaustive Runtime Enumeration**: The main loop iterates from -1 to 9, covering all valid partition codes (0-7) and illegal values.

2. **Template Metaprogramming**: The `PartitionGenerator` template is specialized for each partition code 0-7, forcing the compiler to generate code for all patterns at compile time.

3. **Multi-dimensional Array Broadcasting**: Uses a 3D array with nested OpenACC loops that apply different partitioning schemes.

4. **Volatile Variables**: The `selector` and `current_code` variables are volatile to prevent constant folding and ensure all code paths remain in the compiled output.

5. **Mixed Redundant and Partitioned Clauses**: Includes examples with `gang redundant`, individual partitions, and combined partitions.

6. **Complex Nesting Patterns**: Nested loops with different partitioning at each level to stress the compiler's internal representation.

**Compilation commands:**
