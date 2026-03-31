This script provides comprehensive testing of the uncovered lines in `gcov-tool.cc`:

## Key Features:

1. **Creates valid GCDA files**: Compiles a simple C program with coverage instrumentation and runs it with different inputs to generate `.gcda` files.

2. **Tests all target flags individually**:
   - `-v` (verbose)
   - `-f` (function-level overlap)
   - `-F` (use full pathnames)
   - `-o` (object-level overlap)
   - `-h` (hot only)
   - `-t` (hot threshold with numeric argument)

3. **Tests flag combinations**: Various combinations to ensure flags work together correctly.

4. **Triggers the default case**: Uses invalid options (`-z`, `--invalid`) to trigger `overlap_usage()`.

5. **Tests edge cases**:
   - `-t` without argument
   - `-t` with non-numeric argument
   - Same input file twice
   - Non-existent files
   - Boundary values for `-t` (0.0, 1.0, 1.5, -0.1)

6. **Tests different argument orders**: Flags before and after filenames.

## To use this script:

1. Ensure `gcov-tool` is built with coverage instrumentation:
