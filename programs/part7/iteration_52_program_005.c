This comprehensive test script:

1. **Creates test C programs** and compiles them with GCOV instrumentation
2. **Runs the programs** to generate `.gcda` files
3. **Tests each uncovered case** from the switch statement:
   - `-h` and `--help` for case 'h'
   - `-v` and `--version` for case 'v'
   - `-l` for case 'l'
   - `-p` for case 'p'
   - `-r` for case 'r'
   - `-s` for case 's'
   - `-x` (invalid) for the default case

4. **Tests combinations** of flags to ensure sequential execution
5. **Tests with different file types** (`.gcda`, `.gcno`) and multiple files
6. **Includes edge cases** like no files, non-existent files
7. **Verifies output differences** to ensure flags actually modify behavior
8. **Cleans up** temporary files automatically

To run this test, save it as `test_gcov_dump.sh`, make it executable, and run it:
