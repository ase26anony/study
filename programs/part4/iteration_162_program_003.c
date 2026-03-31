This test program:

1. **Creates equal high parts with different low parts** using various high values (0, 1, -1, INT64_MAX, INT64_MIN) and systematically varies low parts.

2. **Tests boundary values for low part** including:
   - Minimal differences (0 vs 1)
   - Maximal differences (UINT64_MAX-1 vs UINT64_MAX)
   - MSB set vs not set (0x8000000000000000 vs 0x7FFFFFFFFFFFFFFF)

3. **Uses multiple comparison directions** by testing both `a < b` and `a > b` for each pair, as well as equality and inequality operators.

4. **Places comparisons in various contexts**:
   - Direct `if` statements
   - Sorting algorithms (`std::sort`, `std::binary_search`)
   - Loop bounds and array indexing patterns
   - All comparison operators (`<`, `>`, `<=`, `>=`, `==`, `!=`)

5. **Prevents optimization** by:
   - Using `volatile` counter
   - Incorporating command-line arguments
   - Using runtime-generated values
   - Outputting results to prevent dead code elimination

To compile and run with coverage:
