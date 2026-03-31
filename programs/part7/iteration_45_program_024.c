## Key Features of This Test Script:

1. **Complete Flag Coverage**:
   - Tests each individual flag: `-h`, `-v`, `-l`, `-p`, `-r`, `-s`
   - Tests multiple flag combinations to stress the flag-setting logic

2. **Invalid Flag Testing**:
   - Tests `-x` and `-Z` to trigger the `default` case
   - Tests mixed valid/invalid flags

3. **Real GCOV File Processing**:
   - Creates a real C program with coverage instrumentation
   - Generates actual `.gcda` and `.gcno` files
   - Uses these files for all flag tests that require input files

4. **Execution Context Diversity**:
   - Single file analysis
   - Multiple file analysis
   - Wildcard expansion
   - No arguments case
   - Error cases (non-existent files, permission errors)

5. **Additional Coverage**:
   - Tests both short and long option forms (if supported)
   - Tests positional arguments without flags
   - Includes error handling paths

## To Use This Script:

1. First build `gcov-dump` with coverage instrumentation:
