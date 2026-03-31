This script creates multiple test scenarios that:

1. **Mix compilation with help/version requests** (Tests 1, 3, 5, 12) - Forces the driver to handle informational options alongside actual compilation
2. **Use response files** (Tests 3, 8, 10) - Sets `at_file_supplied` flag
3. **Vary dump options between files** (Tests 2, 4, 6, 7) - Exercises `dumpdir`, `dumpbase`, `save_temps_flag` reset logic
4. **Change processing modes** (Test 2) - Uses `-E`, `-S`, `-c` in single invocation
5. **Include environment variables** (Test 13) - Tests `GCC_EXEC_PREFIX` interaction
6. **Use `--` separator** (Test 14) - Tests argument parsing edge cases
7. **Multiple output specifications** (Test 15) - Tests `outbase` reset logic

The script suppresses most output with `2>/dev/null` or `| head/tail` since we're only interested in exercising the driver's internal logic, not the compilation results. Each test is designed to trigger the reset block by changing driver state between processing different arguments or input files.

To run this test:
