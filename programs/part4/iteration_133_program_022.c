This script provides comprehensive testing of the `gcov-dump` command-line parsing logic with the following features:

## Key Test Coverage:

1. **Individual Flags**: Tests `-l`, `-p`, `-r`, `-s` separately
2. **Flag Combinations**: Tests `-lp`, `-pr`, `-lprs`, `-srlp` to verify sequential `getopt` processing
3. **Help/Version**: Tests `-h` and `-v` for early exit paths
4. **Unknown Flags**: Tests `-x`, `-z`, `-lx` to trigger the `default:` case
5. **Multiple Files**: Tests with multiple `.gcda` file arguments
6. **Error Cases**: Tests with non-existent files
7. **Edge Cases**: Empty arguments, mixed valid/invalid files

## Environment Handling:

- Auto-detects `gcov-dump` binary location
- Generates test `.gcda` files if needed
- Creates separate output directory for test results
- Provides clear error messages and instructions

## Coverage Integration:

- Script structured to facilitate coverage measurement
- Includes compilation instructions for instrumented builds
- Tests designed to exercise all switch statement cases
- Output verification to ensure flags have intended effects

## Usage:
