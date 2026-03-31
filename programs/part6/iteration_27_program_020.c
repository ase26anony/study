This script provides comprehensive coverage of the target lines in `gcov-tool.cc`:

1. **Individual flag testing** (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`):
   - Each flag is tested individually with valid `.gcda` files
   - The `-t` flag is tested with multiple threshold values (0.5, 1.0, 10.5)

2. **Flag combinations**:
   - `-f -o` (function and object level)
   - `-F -h -t 1.0` (fullname, hot only, with threshold)
   - `-v -f -F -o -h -t 5.0` (all flags together)
   - `-f -o -t 0.8` (as specified in requirements)
   - `-v -f -F` (verbose with function and fullname)

3. **Multiple input files**:
   - Tests with 2, 3, and 4 `.gcda` files from different runs
   - Creates distinct profile data by running the test program with different arguments

4. **Invalid flag testing**:
   - Tests `-Z` flag to trigger the `default` case and `overlap_usage()`
   - Tests combination of valid and invalid flags

5. **Valid GCOV data generation**:
   - Creates a C program with conditional branches
   - Compiles with `-fprofile-arcs -ftest-coverage` at different optimization levels
   - Executes multiple times to generate distinct `.gcda` files

6. **Overlap analysis mode**:
   - All invocations use `gcov-tool overlap` subcommand as required

7. **Verbose output handling**:
   - Captures output from `-v` flag to files
   - Tests `-v` both alone and in combination with other flags

The script is self-contained and will clean up generated files (commented out by default for inspection). To run it, make it executable and execute:
