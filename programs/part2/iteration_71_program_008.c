This script provides comprehensive testing for the uncovered lines in gcov-tool.cc:

1. **Creates minimal C programs** with different control flow patterns to generate meaningful coverage data for overlap comparison.

2. **Tests each specific option** mentioned in the uncovered lines:
   - `-v` triggers `verbose = true` and `gcov_set_verbose()`
   - `-f` sets `overlap_func_level = 1`
   - `-F` sets `overlap_use_fullname = 1`
   - `-o` sets `overlap_obj_level = 1`
   - `-h` sets `overlap_hot_only = 1`
   - `-t` with argument sets `overlap_hot_threshold = atof(optarg)`
   - Invalid option triggers the `default:` case and `overlap_usage()`

3. **Tests edge cases**:
   - Missing argument for `-t`
   - Invalid (non-numeric) argument for `-t`
   - Insufficient positional arguments
   - Combined options

4. **Provides proper cleanup** using trap to ensure temporary files are removed.

To run the test:
