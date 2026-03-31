This script provides comprehensive testing for the uncovered lines in gcov-tool.cc:

1. **Creates valid GCDA files**: Compiles and runs two C programs with coverage instrumentation
2. **Tests each specific option**:
   - `-v` triggers `gcov_set_verbose()` (lines 534-536)
   - `-f` sets `overlap_func_level = 1` (lines 537-539)
   - `-F` sets `overlap_use_fullname = 1` (lines 540-542)
   - `-o` sets `overlap_obj_level = 1` (lines 543-545)
   - `-h` sets `overlap_hot_only = 1` (lines 546-548)
   - `-t` calls `atof(optarg)` (lines 549-551)
   - Invalid option triggers `default:` case and `overlap_usage()` (lines 552-554)

3. **Tests edge cases**:
   - Valid and invalid float arguments for `-t`
   - Missing required arguments
   - Non-existent files
   - Combined options

4. **Provides clear output** showing which options were tested and their results

To run the test:
