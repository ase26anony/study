**Key Features of This Script:**

1. **Complete Flag Coverage**:
   - Tests each individual flag: `-h`, `-v`, `-l`, `-p`, `-r`, `-s`
   - Tests multiple flag combinations to ensure all setting branches execute

2. **Invalid Flag Testing**:
   - Tests `-x`, `-Z`, and `-lxz` to trigger the `default` case
   - Uses `2>&1 || true` to capture stderr output without stopping execution

3. **Real GCOV File Processing**:
   - Creates a valid C program, compiles with coverage, runs it
   - Uses the generated `.gcda` and `.gcno` files for testing

4. **Execution Context Diversity**:
   - Single file analysis
   - Multiple file analysis
   - Directory scanning with wildcards
   - Different argument combinations

5. **Error Path Testing**:
   - Non-existent files
   - Missing arguments
   - Permission errors
   - Malformed files

**To Use This Script:**

1. First, build `gcov-dump` with coverage instrumentation:
