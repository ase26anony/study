This program systematically tests the uncovered code paths:

1. **Constant-Bounded Assignments**: All slice bounds are compile-time constants using `constexpr`
2. **Mixed Target Types**: 
   - Memory targets (`arr1`, `arr2`, `arr3`, `mdarr`, `short_arr`)
   - Register targets (vectors `vec1`, `vec2`, `register` array)
3. **Struct/Union Assignment**: `ContiguousStruct` and `MultiIntUnion` with multiple member assignments
4. **Vector Extensions**: GCC vector types with slice assignments
5. **Volatile/Atomic**: `volatile` arrays/vectors and `_Atomic` arrays
6. **Multi-dimensional Arrays**: `mdarr[1][1..3]` section assignment
7. **Various Count Values**: Tests count = 1, 2, 3, 4, 5 cases
8. **TYPE_SIZE Variations**: `int` vs `short` arrays to test size calculations

Compile with different optimization levels to trigger different code paths:
