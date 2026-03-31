This program systematically exercises the uncovered code paths:

1. **Constant-Bounded Array Assignments**: Uses `constexpr` variables for bounds (lines with `lo1`, `hi1`, etc.)
2. **Mixed Target Types**: 
   - Memory targets: `arr1`, `volatile_arr`, `atomic_arr`, `md_arr`
   - Register targets: `vec1` (vector extension)
   - Counts of 1, 2, and >2 elements
3. **Struct/Union Assignment**: `ContiguousInts` and `MixedUnion` with multiple member assignments
4. **Vector Extension Slicing**: `v4si` and `v8f` types with slice assignments
5. **Volatile and Atomic Qualifiers**: `volatile_arr` and `atomic_arr`
6. **Multi-dimensional Array**: `md_arr[1][1..3]` section assignment
7. **Small Element Size**: `v16hi` with 16 short elements (tests `TYPE_SIZE * count`)

Compile with:
