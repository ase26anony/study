## Key Design Elements:

1. **Complete Coverage of All Cases**: The code includes:
   - `update(in)` via `depend(in: ...)`
   - `update(inout)` via `depend(inout: ...)`
   - `update(out)` via `depend(out: ...)`
   - `update(mutexinoutset)` via `depend(mutexinoutset: ...)`
   - `update(inoutset)` via `depend(inoutset: ...)`
   - `destroy` via `depend(destroy: ...)`

2. **Triggering Diagnostics**: Each OpenMP region contains `__attribute__((unused))` variables that will trigger `-Wunused-variable` warnings, forcing the compiler to print the OpenMP constructs with pretty-printing.

3. **Multiple Contexts**: The code uses:
   - Template function (`template_openmp_depend`)
   - Lambda expression
   - Class member function (`OpenMPDestroyTest::test_destroy_clause`)
   - Regular function (`test_target_data_regions`)
   - Nested template in struct

4. **Valid OpenMP Usage**: All constructs use valid OpenMP 4.5+ syntax with `nowait` clauses to avoid runtime synchronization issues.

5. **Variable Visibility**: Uses `volatile` and `#pragma omp declare target` to ensure variables are visible in target regions and not optimized away.

## Compilation Commands:

For warning-based coverage (triggers pretty-printer in diagnostics):
