## Key Features of This Script:

1. **Systematic Flag Testing**:
   - Individual flags: `-l`, `-p`, `-r`, `-s`
   - Flag combinations: `-lp`, `-pr`, `-lprs` (tests `getopt` character processing)
   - Help (`-h`) and version (`-v`) early exits
   - Unknown flags: `-x`, `-z`, `-lx` (triggers `default:` case)

2. **Output Verification**:
   - Checks that `-l` produces more verbose output
   - Verifies `-r` contains numeric/hex patterns
   - Confirms unknown flags generate error messages

3. **File Handling Tests**:
   - Multiple `.gcda` file arguments
   - Non-existent file error handling
   - Mixed valid/invalid file arguments

4. **Environment Robustness**:
   - Validates `gcov-dump` binary existence
   - Checks for required `.gcda` files with helpful instructions
   - Creates dummy files if needed for parser testing
   - Comprehensive error handling and logging

5. **Coverage Integration**:
   - Clear instructions for instrumented builds
   - Supports testing with `-O0`, `-O2`, `-O3` optimized binaries
   - Separates test execution from coverage measurement

## Usage:

1. **Prepare test data**:
