**Key features that exercise the uncovered code:**

1. **Exhaustive Partition Code Testing**: The main loop tests codes -1 through 9, covering all valid (0-7) and illegal values.

2. **Template Metaprogramming**: The `PartitionGenerator` template is explicitly instantiated for codes 0-7, ensuring compile-time generation of all partition combinations.

3. **Multi-dimensional Array Broadcasting**: Uses a 3D array with complex nested OpenACC directives that require internal partition code representation.

4. **Volatile Variables**: `partition_selector` and `code` are volatile to prevent constant folding and ensure all code paths are considered.

5. **Mixed Partition Clauses**: Combines `gang`, `worker`, `vector`, and `redundant` clauses in various combinations:
   - `gang redundant` for code 0
   - `gang(static:1)` for code 1
   - `worker` for code 2
   - `gang worker` for code 3
   - `vector` for code 4
   - `gang vector` for code 5
   - `worker vector` for code 6
   - `gang worker vector` for code 7

6. **Complex Nesting**: Nested loops with different partition schemes at each level force the compiler to generate complex internal representations.

**Compilation commands:**
