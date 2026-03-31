**Key features of this test script:**

1. **Complete flag coverage**: Tests each individual flag (`-h`, `-v`, `-l`, `-p`, `-r`, `-s`) to cover all switch cases.

2. **Flag combinations**: Tests 11 different flag combinations to ensure the flag-setting logic works correctly when multiple flags are specified.

3. **Invalid flag testing**: Tests `-x`, `-Z`, and `-lxz` to trigger the `default` case in the switch statement, executing the `fprintf(stderr, "unknown flag `%c'\n", opt)` line.

4. **Real GCOV file handling**: Creates and compiles actual C programs with coverage instrumentation, then runs them to generate `.gcda` files for realistic testing.

5. **Multiple usage scenarios**:
   - Help and version flags (no files needed)
   - Single file analysis
   - Multiple file arguments
   - Wildcard expansion
   - Error conditions (missing files, no arguments)

6. **Additional error paths**: Tests various error conditions that might stress related parsing logic.

**To use this script:**

1. First build `gcov-dump` with coverage instrumentation:
