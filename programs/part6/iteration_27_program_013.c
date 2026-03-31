This script provides comprehensive coverage of the uncovered lines in `gcov-tool.cc`:

1. **Individual Flag Testing**: Tests each uncovered flag (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) individually with valid GCOV data files.

2. **Flag Combinations**: Tests various combinations of flags together to ensure the parser handles multiple options correctly.

3. **Valid GCOV Data**: Generates actual `.gcda` and `.gcno` files by:
   - Creating a C program with conditional branches
   - Compiling with `-fprofile-arcs -ftest-coverage`
   - Running the program multiple times with different inputs
   - Creating both optimized and non-optimized versions

4. **Overlap Analysis Mode**: All invocations use `gcov-tool overlap` as the subcommand.

5. **Error Testing**: Includes an invalid flag `-Z` to trigger the `default` case and `overlap_usage()`.

6. **Multiple Input Files**: Tests with 2-3 different `.gcda` files to stress the overlap merging logic.

7. **Verbose Output**: Captures verbose output to files and tests `-v` both alone and in combination.

8. **Edge Cases**: Tests various edge cases including:
   - Different threshold values (0.5, 1.0, 10.5, 0.001, 1000.0)
   - Missing arguments
   - Non-existent files
   - Different flag orders
   - Multiple input files

To run the script:
