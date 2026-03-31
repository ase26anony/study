This script provides a comprehensive test suite that:

1. **Creates valid GCDA files** by compiling and running a simple C program with different inputs
2. **Tests all individual flags** (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) as specified in the uncovered lines
3. **Tests flag combinations** to ensure they work together
4. **Tests edge cases** including missing arguments, non-numeric thresholds, same file, and non-existent files
5. **Triggers the default case** by using unrecognized options (`-z`, `-x -y`)
6. **Tests various threshold values** from 0.0 to 1.5
7. **Tests different argument orders** to ensure robust parsing
8. **Creates additional test data** using `gcov-tool merge` for more interesting comparisons

To use this script:
