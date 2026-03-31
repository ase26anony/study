**Key features of this test script:**

1. **Complete flag coverage**: Tests each individual flag (`-h`, `-v`, `-l`, `-p`, `-r`, `-s`) and various combinations.

2. **Invalid flag testing**: Includes tests with `-x` and `-Z` to trigger the `default` case in the switch statement.

3. **Real GCOV file handling**: Creates, compiles, and runs a test program to generate actual `.gcda` and `.gcno` files.

4. **Multiple execution contexts**: Tests single files, multiple files, wildcards, and directory inputs.

5. **Error path testing**: Tests non-existent files, permission errors, and invalid arguments.

6. **Flag ordering variations**: Tests different ways flags and files can be ordered on the command line.

**To use this script:**

1. First build `gcov-dump` with coverage instrumentation:
