This script provides comprehensive testing of the uncovered lines in `gcov-tool.cc`:

## Key Features:

1. **Valid GCDA Generation**: Creates a simple C program, compiles it with coverage flags, and runs it multiple times to generate different GCDA files.

2. **Individual Flag Testing**: Tests each switch case individually:
   - `-v` (verbose)
   - `-f` (function-level overlap)
   - `-F` (use full pathnames)
   - `-o` (object-level overlap)
   - `-h` (hot only)
   - `-t` (hot threshold with numeric argument)

3. **Combined Flag Testing**: Tests various combinations of flags to ensure they work together.

4. **Edge Case Testing**:
   - `-t` without argument (triggers error handling)
   - `-t` with non-numeric argument
   - Same input file twice
   - Non-existent files
   - Invalid options (triggers `default` case → `overlap_usage()`)

5. **Additional Scenarios**:
   - Multiple GCDA file pairs
   - Absolute paths (relevant for `-F` flag)
   - Different threshold values

## Usage:

1. Ensure `gcov-tool` is built with coverage instrumentation:
