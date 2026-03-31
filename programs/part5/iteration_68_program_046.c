This script systematically tests all the uncovered lines in the `gcov-tool.cc` overlap command parsing:

1. **Creates valid GCDA files**: Compiles and runs a simple C program multiple times to generate different coverage profiles
2. **Tests individual flags**: Each of the switch cases (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) is tested individually
3. **Tests flag combinations**: Multiple flags are combined to ensure they work together
4. **Tests edge cases**:
   - `-t` without argument (triggers error handling)
   - `-t` with non-numeric argument
   - Invalid option `-z` (triggers the `default` case and `overlap_usage()`)
   - Same file twice
   - Non-existent files
   - Boundary threshold values
5. **Tests additional scenarios**: Empty files, binary files, different flag orders

To use this script with a coverage-instrumented `gcov-tool`:

1. First build `gcov-tool` with coverage:
