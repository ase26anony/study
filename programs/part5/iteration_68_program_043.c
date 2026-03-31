This script provides comprehensive testing of the uncovered lines in `gcov-tool.cc`:

1. **Creates valid GCDA files**: Compiles and runs a simple C program to generate `.gcda` files
2. **Tests each individual switch case**: `-v`, `-f`, `-F`, `-o`, `-h`, `-t`
3. **Tests flag combinations**: Multiple flags together
4. **Tests edge cases**:
   - Invalid option `-z` (triggers `default:` case and `overlap_usage()`)
   - `-t` without argument
   - `-t` with non-numeric argument
   - Same file twice
   - Non-existent files
   - Boundary values for `-t`
5. **Uses proper execution flow**: Compile → Run → Generate GCDA → Test gcov-tool

To use this script:

1. Ensure `gcov-tool` is built with coverage instrumentation:
