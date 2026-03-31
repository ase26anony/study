This program provides comprehensive coverage through:

1. **Template Specializations (0-7)**: Each partition code 0-7 has a dedicated template specialization with appropriate OpenACC directives.

2. **Runtime Exhaustive Testing**: Tests codes -1 through 9 using a volatile selector to prevent optimization.

3. **Multi-dimensional Array Broadcasting**: Uses `md_data[10][10][10]` with complex access patterns across different partition schemes.

4. **Mixed Partition Clauses**: Combines `gang`, `worker`, `vector`, and `redundant` clauses in various configurations.

5. **Illegal Code Coverage**: The default case handles values outside 0-7 range.

**Compilation commands:**
