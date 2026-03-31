This comprehensive test script covers all the requirements:

1. **Individual flag testing**: Tests `-l`, `-p`, `-r`, `-s` flags in isolation
2. **Flag combinations**: Tests various combinations of valid flags
3. **Invalid flag handling**: Tests `-x`, `-?`, `-L` to trigger the `default:` case
4. **Help and version**: Tests `-h` and `-v` flags
5. **Valid GCOV files**: Creates and uses actual `.gcda` and `.gcno` files
6. **Additional coverage**: Tests with different file types, multiple files, merged flags, and edge cases

The script creates two test programs with different control flow structures to ensure the GCOV files have meaningful data. Each test captures the first few lines of output to verify the commands execute without errors (except for the invalid flag tests which are expected to fail).

To run this test and then check coverage:
