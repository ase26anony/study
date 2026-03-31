This script provides comprehensive coverage of the target lines in `gcov-tool.cc`:

1. **Individual Flag Testing**: Each uncovered flag (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) is tested individually.

2. **Flag Combinations**: Multiple combinations of flags are tested to ensure the parser handles them correctly.

3. **Invalid Flag**: The `-Z` flag triggers the `default` case and calls `overlap_usage()`.

4. **Valid GCOV Data**: Creates a C program, compiles it with instrumentation, runs it multiple times to generate `.gcda` files.

5. **Multiple Input Files**: Tests with 1, 2, and multiple `.gcda` files, including files from different compilation modes.

6. **Threshold Values**: Tests `-t` with different floating-point values (0.5, 1.0, 10.5).

7. **Verbose Output**: Captures output from `-v` flag invocations.

8. **Directory/Wildcard Inputs**: Tests with directory paths and wildcard patterns.

To run the test:
