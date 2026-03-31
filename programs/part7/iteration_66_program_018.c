This program systematically exercises the uncovered code paths:

1. **Constant-bounded assignments**: All loops use `constexpr` bounds
2. **Mixed target types**: Arrays (memory), vectors (possibly registers), structs, unions
3. **Different counts**: Cases with count = 1, 2, 3, 4, 5, 11
4. **Volatile/Atomic**: Uses `volatile` and `_Atomic` qualifiers
5. **Multi-dimensional arrays**: Contiguous sub-array assignments
6. **Different element sizes**: char (1 byte), int (4 bytes), double (8 bytes)
7. **Prevents optimization**: `escape()` function and checksum prevent dead code elimination

**Compilation options to test:**
