## Key Features of This Script:

1. **Systematic Flag Testing**: Covers all individual flags (`-l`, `-p`, `-r`, `-s`), combinations, help/version, and unknown flags.

2. **Error Path Coverage**: Tests unknown flags (`-x`, `-z`, `-lx`) to trigger the `default:` case and error message.

3. **Multi-file Handling**: Tests with multiple `.gcda` file arguments.

4. **File Error Handling**: Tests with non-existent files.

5. **Output Verification**: Checks that flags actually affect output (e.g., `-l` produces more output).

6. **Environment Robustness**: Checks for binary and input files, provides helpful error messages.

7. **Coverage Integration**: Includes instructions for compiling with instrumentation and generating coverage reports.

## Usage Instructions:

1. **Prepare test data**:
