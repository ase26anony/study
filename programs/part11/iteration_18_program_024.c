This script systematically tests all the command-line options specified in the uncovered block:

1. **Individual flag testing**: Each flag (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) is tested individually to ensure the switch-case logic for each character is entered.

2. **Combination testing**: Various combinations of flags are tested to ensure they work together correctly.

3. **Edge cases for `-t` flag**: The threshold flag is tested with integer values (`1`), fractional values (`0.33`, `0.75`), boundary values (`0`, `100`), and exact values (`1.0`).

4. **Flag ordering**: Tests different orderings of flags and files to ensure parsing is position-independent.

5. **Invalid option**: Tests `-x` to trigger the `default:` case and call `overlap_usage()`.

6. **Valid input files**: Creates and uses actual `.gcda` files from a compiled C program with coverage instrumentation.

7. **Multi-file merge**: Uses two distinct `.gcda` files to exercise the merge logic that uses the flags.

8. **Output handling**: Redirects output appropriately for both successful and error cases.

To run this test and get coverage for the specific lines in `gcov-tool.cc`:
