## Key Features:

1. **All Six Update Modifiers Covered:**
   - `depend(in: ...)` in `test_depend_in()` and `template_depend_test()`
   - `depend(inout: ...)` in `test_depend_inout()` and nested construct
   - `depend(out: ...)` in `test_depend_out()` and template
   - `depend(mutexinoutset: ...)` in `test_depend_mutexinoutset()`
   - `depend(inoutset: ...)` in `test_depend_inoutset()` and `test_with_if_clause()`
   - `depend(destroy: ...)` in `test_depend_destroy()` and questionable use

2. **Multiple Pretty-Printer Contexts:**
   - Valid OpenMP constructs for compilation
   - Questionable use (`destroy` on `target update`) to trigger diagnostics
   - Template instantiations for C++-specific paths
   - Different optimization levels via attributes

3. **Prevents Optimization:**
   - `volatile` variables throughout
   - External function calls
   - Inline assembly memory clobber
   - Checksum calculation and output

4. **Complex OpenMP Usage:**
   - Nested clauses (`if`, `nowait`, `map`)
   - Multiple constructs (`target update`, `target enter data`, `task`, `teams`)
   - Template class and functions

## Compilation Commands:
