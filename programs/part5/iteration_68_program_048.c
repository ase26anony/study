This script provides comprehensive testing for the uncovered lines in `gcov-tool.cc`:

## Key Features:

1. **Creates valid GCDA files**: Compiles a simple C program with coverage, runs it with different inputs to generate two distinct `.gcda` files.

2. **Tests all individual switch cases**:
   - `-v` (verbose mode)
   - `-f` (function-level overlap)
   - `-F` (use full pathnames)
   - `-o` (object-level overlap)
   - `-h` (hot only)
   - `-t` (hot threshold with various numeric values)

3. **Tests combined flags**: Multiple combinations to ensure flags work together.

4. **Tests edge cases and error handling**:
   - Invalid option `-z` (triggers `default` case → `overlap_usage()`)
   - `-t` without argument
   - `-t` with non-numeric argument
   - Same input file twice
   - Non-existent files
   - Different argument orders
   - Edge threshold values

5. **Clean execution flow**: Creates, tests, and cleans up all temporary files.

## To use this script:

1. Ensure `gcov-tool` is built with coverage instrumentation:
