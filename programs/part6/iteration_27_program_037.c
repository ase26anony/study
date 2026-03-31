This script provides comprehensive coverage of the uncovered lines in `gcov-tool.cc`:

1. **Individual flag testing**: Each flag (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) is tested individually
2. **Flag combinations**: Multiple combinations including `-f -o`, `-F -h -t 1.0`, and `-v -f -F -o -h -t 5.0`
3. **Invalid flag**: Tests `-Z` to trigger the `default` case and `overlap_usage()`
4. **Multiple input files**: Tests with 2, 3, and directory of `.gcda` files
5. **Different threshold values**: Tests `-t` with 0.0, 0.5, 1.0, 5.0, 10.5, and 100.0
6. **Valid GCOV data generation**: Creates a C program, compiles it with instrumentation, and runs it multiple times to generate different `.gcda` files
7. **Verbose output handling**: Captures all output to `gcov_output.txt`

The script handles edge cases like:
- Single file input for overlap analysis
- Different optimization levels (O0, O2, LTO if available)
- Various threshold values including edge cases (0.0, 100.0)
- Directory with multiple gcda files

To run the script:
