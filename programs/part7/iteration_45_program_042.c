This script provides comprehensive coverage of the target code:

1. **Individual Flags**: Tests each switch case (`-h`, `-v`, `-l`, `-p`, `-r`, `-s`)
2. **Flag Combinations**: Tests multiple flags together to ensure all flag-setting logic executes
3. **Invalid Flags**: Tests `-x`, `-Z`, and `-lxz` to trigger the `default` case with `fprintf(stderr, "unknown flag...")`
4. **Valid GCOV Files**: Creates and uses real `.gcda`/`.gcno` files for meaningful tests
5. **Execution Contexts**: Tests single files, multiple files, wildcards, different directories
6. **Error Conditions**: Tests missing files, wrong permissions, and missing arguments

To use this script:

1. First build `gcov-dump` with coverage instrumentation:
