This test harness comprehensively covers all the requirements:

## Key Features:

1. **Direct Invocation of gcov-dump**: 
   - Tests `-h` for `print_usage()` (lines 111-113)
   - Tests `-v` for `print_version()` (lines 114-116)
   - Tests `-l`, `-p`, `-r`, `-s` individually (lines 117-128)
   - Tests combinations like `-lp`, `-rs`, `-lprs`

2. **Invalid Argument Handling**:
   - Tests single invalid flags: `-x`, `-9`, `-@` (triggers default case lines 129-130)
   - Tests valid + invalid: `-l -z`, `-p -x`
   - Tests multi-character invalid: `-abc`, `-xyz`

3. **Valid .gcda/.gcno File Generation**:
   - Creates a minimal C program with branches
   - Compiles with `-fprofile-arcs -ftest-coverage`
   - Executes to generate coverage data

4. **System Call Execution**:
   - Uses `popen()` to capture output
   - Verifies exit codes and output content
   - Checks for "unknown flag" in stderr for invalid cases

5. **Edge Cases and File Handling**:
   - Tests with non-existent file
   - Tests with invalid file (plain text)
   - Tests with no arguments
   - Tests with both .gcda and .gcno files

## Compilation and Execution:
