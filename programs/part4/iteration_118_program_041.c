This generated program specifically targets the uncovered lines in `cxx-pretty-print.cc` by:

1. **Using all five update types and destroy** in distinct contexts:
   - `depend(in: arr[0])` in `test_depend_basic`
   - `depend(inout: arr[1])` in `test_depend_basic`
   - `depend(out: arr[2])` in `test_depend_basic`
   - `depend(mutexinoutset: lock_var)` in `test_depend_sets`
   - `depend(inoutset: arr[0:50])` in `test_depend_sets`
   - `depend(destroy: resource)` in `test_destroy`

2. **Combining with array sections and iterators**:
   - `depend(in: arr[0:N])` and `depend(out: arr[N:M])` in `test_array_sections`
   - `depend(inout: *begin)` and `depend(out: *(end-1))` in `test_iterator_depend`

3. **Nesting in complex constructs**:
   - `#pragma omp target teams distribute parallel for depend(in: input[0:N])` in `test_nested_target`
   - Tasks nested inside `parallel` + `single` regions throughout
   - Combined with `if`, `final`, `mergeable`, and `priority` clauses

4. **Using templates and lambdas**:
   - `test_template_depend<T>()` instantiated for `int` and `float`
   - Lambda capturing `var` with `depend(out: var)` in `test_lambda_depend`

5. **Preventing optimization**:
   - `volatile` variables
   - `USE()` macro to consume values
   - `__attribute__((noinline, noipa))` on test functions
   - Final checksum calculation and output

Compile with the recommended flags to trigger the pretty-printer during compilation:
