This script comprehensively tests all the uncovered lines from `gcov-tool.cc`:

1. **Creates valid GCDA files**: Compiles and runs two C programs with coverage instrumentation
2. **Tests each option individually**: `-v`, `-f`, `-F`, `-o`, `-h`, `-t`
3. **Tests option combinations**: Including all boolean flags together
4. **Triggers the default case**: With invalid option `-x` to call `overlap_usage()`
5. **Tests edge cases**: Missing arguments, invalid numeric values, insufficient files
6. **Validates behavior**: Checks exit codes and output patterns

The script ensures that the specific switch cases in lines 534-554 are executed by:
- Setting `verbose = true` and calling `gcov_set_verbose()` via `-v`
- Setting `overlap_func_level = 1` via `-f`
- Setting `overlap_use_fullname = 1` via `-F`
- Setting `overlap_obj_level = 1` via `-o`
- Setting `overlap_hot_only = 1` via `-h`
- Setting `overlap_hot_threshold` via `-t` with float argument
- Triggering `default:` case and `overlap_usage()` via invalid option `-x`

To run the test:
