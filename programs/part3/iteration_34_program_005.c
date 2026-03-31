**Key aspects of this test script:**

1. **Invalid Flag Targeting**: The script uses various invalid flags (`-x`, `-?`, `-y`, `-z`, `-a`, `-b`, `-c`, `-q`) that are not in the switch statement (`h`, `v`, `l`, `p`, `r`, `s`), ensuring they hit the `default` case.

2. **Valid Flag Combinations**: Each test (except Test 4) includes valid flags (`-l`, `-p`, `-v`, `-r`, `-s`) to ensure the program executes the parsing loop and reaches the invalid flag processing.

3. **Required GCOV File**: The script creates and compiles a simple C program with coverage flags, runs it to generate `test.gcda`, and passes this valid file to `gcov-dump`.

4. **Multiple Invalid Flags**: Test 3 specifically uses multiple invalid flags (`-y -z`) to trigger the error path multiple times.

5. **Mixed Flag Order**: Test 5 places an invalid flag (`-q`) between valid flags (`-l` and `-p`) to test the parser's behavior in different positions.

**Expected Output:**
The script should produce error messages like:
