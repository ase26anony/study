This comprehensive test script:

1. **Creates a minimal C program** with basic operations to generate meaningful coverage data
2. **Compiles with coverage instrumentation** using `-fprofile-arcs -ftest-coverage -O0`
3. **Runs the program** to generate `.gcda` and `.gcno` files
4. **Tests all valid single-character flags** (`-h`, `-v`, `-l`, `-p`, `-r`, `-s`)
5. **Tests invalid flags** (`-x`, `-z`) to trigger the `default` case
6. **Tests combined flags** (`-lp`, `-rl`, `-lprs`) to ensure proper parsing of multiple options
7. **Tests missing filename** for flags that require a file argument
8. **Tests long options** (`--help`, `--version`) if supported
9. **Tests with both `.gcda` and `.gcno` files**
10. **Redirects output appropriately** to prevent clutter
11. **Includes optional valgrind memory checking**
12. **Provides clear output** showing what was tested and the results

To run the script:
