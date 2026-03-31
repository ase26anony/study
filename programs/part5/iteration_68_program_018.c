This comprehensive test script:

1. **Creates valid GCDA files** by compiling and running two different C programs with coverage instrumentation
2. **Tests each individual flag** (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) from the uncovered switch cases
3. **Tests flag combinations** to ensure they work together correctly
4. **Triggers the default case** with an invalid option (`-z`)
5. **Tests edge cases**:
   - `-t` without argument
   - `-t` with non-numeric argument
   - Same file twice
   - Non-existent files
   - Empty/malformed GCDA files
   - Various argument orderings

To use this script:

1. Save it as `test_gcov_tool.sh`
2. Make it executable: `chmod +x test_gcov_tool.sh`
3. Ensure `gcov-tool` is in your PATH
4. Run it: `./test_gcov_tool.sh`

For coverage collection on `gcov-tool` itself, you'll need to build it with coverage flags first:
