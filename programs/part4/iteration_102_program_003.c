This comprehensive test script:

1. **Creates a minimal C program** with loops and conditionals to generate meaningful coverage data
2. **Compiles it with coverage flags** (`-fprofile-arcs -ftest-coverage -O0`)
3. **Runs the program** to generate `.gcda` and `.gcno` files
4. **Tests all valid single-character flags** (`-h`, `-v`, `-l`, `-p`, `-r`, `-s`)
5. **Tests invalid flags** (`-x`, `-z`) to trigger the `default` case
6. **Tests combined flags** (`-lp`, `-rl`, `-lpx`) to ensure proper parsing
7. **Tests edge cases**:
   - Missing filename with flags that require one
   - Non-existent file
   - No arguments
   - Long options (`--version`, `--help`) if supported
8. **Redirects output** appropriately to avoid clutter
9. **Cleans up** generated files after testing

To run this test:
