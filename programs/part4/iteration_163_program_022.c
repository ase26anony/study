This comprehensive test script:

1. **Creates valid GCOV data files** by compiling and running two instrumented C programs
2. **Tests each individual option** (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) from the uncovered switch block
3. **Tests option combinations** to ensure they work together correctly
4. **Tests edge cases** for the `-t` option with various threshold values (0.0, 1.0, 1.5, -0.1)
5. **Triggers the default case** by using an invalid option (`-x`)
6. **Tests error conditions** like missing argument for `-t`
7. **Captures output** for verification without cluttering the terminal
8. **Cleans up automatically** using a temporary directory and trap handler

To run this test:
