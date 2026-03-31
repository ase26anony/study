This test program provides comprehensive coverage for the uncovered pretty-printer lines:

1. **All `depend` clause types**: Includes `in`, `out`, `inout`, `mutexinoutset`, `inoutset`, and `destroy` dependencies.

2. **`update` modifier usage**: Each dependency type is used with the `update` modifier as required by the uncovered code.

3. **C++ specific features**: Uses references (`int& ref_dep`), class objects (`DataObject`), and object addresses.

4. **OpenMP version guards**: Protects OpenMP 5.0+ and 5.2+ features with appropriate preprocessor checks.

5. **Valid dependency scoping**: All dependency variables are properly shared in the parallel region.

6. **Semantically valid task graph**: Tasks have dependencies that ensure proper execution ordering.

7. **Mixed OpenMP constructs**: Uses `parallel`, `single`, `task`, and `taskwait` constructs.

To compile and test:
