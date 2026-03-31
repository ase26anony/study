This script comprehensively tests all the uncovered lines from `gcov-tool.cc`:

1. **Creates minimal C programs** with branching logic to generate meaningful coverage data
2. **Compiles with coverage instrumentation** using `-fprofile-arcs -ftest-coverage`
3. **Tests each specific option** mentioned in the uncovered lines:
   - `-v` for verbose mode
   - `-f` for function-level overlap
   - `-F` for fullname usage
   - `-o` for object-level overlap
   - `-h` for hot-only mode
   - `-t` with various float arguments
4. **Triggers the default case** with invalid options (`-x`, `-Z`)
5. **Tests error cases** like missing or invalid arguments for `-t`
6. **Combines multiple options** to ensure they work together
7. **Cleans up** all temporary files

To run the test:
