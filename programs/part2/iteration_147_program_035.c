This program combines all the required patterns:

1. **`_reductemp_` generation**: The `OMPTemplateProcessor::process_reduction` method uses a complex `#pragma omp target teams distribute parallel for simd reduction(+:sum)` with a user-defined `MyType` that has overloaded `operator+`. This forces the compiler to create internal reduction temporaries.

2. **`_scantemp_` generation**: The `OMPTemplateProcessor::process_scan` method uses `#pragma omp simd reduction(inscan, +:scan_sum)` with the exclusive scan directive, which is known to generate `_scantemp_` clauses.

3. **`_condtemp_` generation**: The `process_tasks` function uses a generic lambda within a parallel region with `#pragma omp taskloop if(task_id < threshold)`, where the condition depends on runtime values.

4. **`enter` clause**: The `main` function uses `#pragma omp target enter data map(to: large_array[0:large_size])` which explicitly triggers the `enter` clause with a `to` mapper.

5. **Template context**: All OpenMP pragmas are embedded within template functions and classes, with perfect forwarding via `process_value` helper and `decltype(auto)` return types.

6. **Runtime validation**: The program uses command-line arguments for iteration counts, includes debug prints protected by `omp_get_thread_num() == 0`, and computes checksums to prevent dead code elimination.

To maximize coverage of the pretty-printer code paths, compile with:
