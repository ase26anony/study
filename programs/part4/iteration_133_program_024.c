## Key Features of This Script:

1. **Comprehensive Flag Testing**:
   - Individual flags: `-l`, `-p`, `-r`, `-s`
   - Flag combinations: `-lp`, `-pr`, `-rs`, `-lpr`, `-lps`, `-prs`, `-lprs`
   - Help (`-h`) and version (`-v`) flags
   - Unknown flags: `-x`, `-z`, `-lx` (tests error path)

2. **File Handling Tests**:
   - Single `.gcda` file
   - Multiple `.gcda` files
   - Non-existent file (error handling)

3. **Coverage Verification**:
   - Outputs instructions for compiling `gcov-dump` with coverage
   - Verifies that all flag cases are exercised
   - Checks that error paths are triggered

4. **Robustness Features**:
   - Validates `gcov-dump` binary existence
   - Creates dummy `.gcda` files if needed
   - Handles different exit codes (help/version may exit early)
   - Clean output organization in `gcov-dump-test-output/`

5. **Coverage Compilation Recommendations**:
   The script includes instructions for compiling `gcov-dump` with different optimization levels to ensure the switch statement logic remains observable:
   - `-O0` for clear line-by-line coverage
   - `-O2` for typical optimization
   - `-O3` for aggressive optimization stress test

## Usage:

1. **Prepare test data** (if you don't have `.gcda` files):
