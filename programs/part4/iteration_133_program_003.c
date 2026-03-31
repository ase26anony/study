## Key Features of This Script:

1. **Comprehensive Flag Testing**:
   - Individual flags: `-l`, `-p`, `-r`, `-s`
   - Flag combinations: `-lp`, `-pr`, `-rps`, `-lprs`
   - Help (`-h`) and version (`-v`) flags
   - Unknown flags: `-x`, `-z`, `-lx`

2. **Environment Robustness**:
   - Checks for `gcov-dump` binary existence
   - Validates `.gcda` file availability with helpful instructions
   - Uses `set -u` for safer execution
   - Color-coded output for clarity

3. **Coverage Measurement Integration**:
   - Clear instructions for compiling with coverage instrumentation
   - Tests designed to exercise all switch cases
   - Output verification to ensure flags have intended effects

4. **Execution Flow**:
   - 8 phases covering all required test scenarios
   - Logs all commands and outputs for debugging
   - Handles both success and expected error cases

5. **Additional Edge Cases**:
   - Multiple file arguments
   - Non-existent file handling
   - Empty argument list
   - Flag ordering variations

## Usage Instructions:

1. **Prepare Test Data**:
