This script provides:

1. **Comprehensive flag testing**:
   - Individual flags: `-l`, `-p`, `-r`, `-s`
   - Flag combinations: `-lp`, `-pr`, `-rps`, `-lprs`
   - Help and version: `-h`, `-v`
   - Unknown flags: `-x`, `-z`, `-lx`

2. **Environment robustness**:
   - Checks for `gcov-dump` binary in PATH and common locations
   - Validates existence of required `.gcda` files
   - Provides instructions if files are missing

3. **Flag effect verification**:
   - Compares output verbosity for `-l` flag
   - Checks for numeric data in `-r` flag output
   - Validates error messages for unknown flags

4. **Coverage measurement integration**:
   - Clear instructions for instrumented compilation
   - Structured test sequence for line-by-line coverage
   - Tests all switch cases and error paths

To use this script for coverage measurement:
