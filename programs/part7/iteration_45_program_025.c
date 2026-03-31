**Key features of this test script:**

1. **Complete flag coverage**: Tests each individual flag (`-h`, `-v`, `-l`, `-p`, `-r`, `-s`)
2. **Flag combinations**: Tests various combinations of flags together
3. **Invalid flag testing**: Tests `-x` and `-Z` to trigger the `default` case
4. **Real GCOV files**: Creates and uses actual `.gcda` and `.gcno` files
5. **Multiple usage scenarios**:
   - Single file analysis
   - Multiple file analysis
   - Wildcard expansion
   - Error conditions (missing files, wrong file types)
6. **Edge cases**: Tests various argument orderings and edge conditions

**To use this script:**

1. First build `gcov-dump` with coverage instrumentation:
