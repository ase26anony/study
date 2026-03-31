This program specifically targets the uncovered lines by:

1. **All Five Update Types + Destroy**: Each of the six cases (`in`, `inout`, `out`, `mutexinoutset`, `inoutset`, `destroy`) appears in distinct contexts:
   - `depend(in: var_in)` in `test_depend_basic()`
   - `depend(inout: var_inout)` in `test_depend_basic()`
   - `depend(out: var_out)` in `test_depend_basic()`
   - `depend(mutexinoutset: lock_var)` in `test_depend_sets()` within a taskgroup
   - `depend(inoutset: arr[0:50])` in `test_depend_sets()`
   - `depend(destroy: resource)` in `test_destroy()`

2. **Array Sections and Iterators**:
   - Array sections: `arr[0:50]`, `arr[50:100]`
   - Iterators: `*(begin)`, `*(end - 1)` with C++ std::vector

3. **Complex OpenMP Constructs**:
   - Combined directive: `#pragma omp target teams distribute parallel for depend(in: input[0:1000])`
   - Nested tasks with different depend clauses
   - Combined with `if`, `final`, `mergeable`, `priority` clauses

4. **C++ Templates and Lambdas**:
   - Template function `template_depend<T>` instantiated for `int` and `float`
   - Lambda capturing variable and creating task with `depend(out: captured_var)`

5. **Prevents Optimization**:
   - `volatile` variables
   - `USE()` macro that forces variable usage
   - `__attribute__((noinline, noipa))` to prevent inlining
   - Final checksum computation and output

To trigger the pretty-printer coverage, compile with:
