This script:

1. **Creates minimal C programs** with loops and conditionals to generate meaningful coverage data
2. **Compiles with coverage flags** (`-fprofile-arcs -ftest-coverage`)
3. **Runs the programs** to generate `.gcda` files
4. **Tests each uncovered option**:
   - `-v` for verbose mode
   - `-f`, `-F`, `-o`, `-h` as boolean flags
   - `-t` with various numeric arguments (0.75, 0.5, 1)
   - Invalid option `-x` to trigger the default case
5. **Tests combined options** to ensure they work together
6. **Tests with multiple `.gcda` files** to ensure the overlap subcommand handles them properly
7. **Cleans up** all temporary files

To run the test:
