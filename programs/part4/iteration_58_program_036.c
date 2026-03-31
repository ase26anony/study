This script systematically tests the uncovered reset logic by:

1. **Multiple compilation units with mixed options**: Tests 2, 6, and 10 use different `-save-temps`, `-dumpdir`, and `-dumpbase` options for different files in the same invocation.

2. **Combine help/version with compilation**: Tests 1, 5, 11, and 12 mix `--help`, `--version`, and `--target-help` with actual source files.

3. **Use `@file` syntax**: Tests 3 and 5 use response files (`@args.txt`, `@opts.txt`) to set options.

4. **Exercise `-save-temps` and dump variants**: Tests 2, 3, 4, 6, and 9 use various combinations of `-save-temps`, `-dumpdir`, `-dumpbase`, and `-dumpbase-ext`.

5. **Driver-specific mode switches**: Test 4 chains `-E`, `-S`, and `-c` modes; Test 7 uses `-x` language switches.

6. **Environment variables and wrapper**: Test 8 sets environment variables and uses a wrapper script.

The script redirects stderr to `/dev/null` and uses `|| true` to continue even if some invocations fail (as expected when mixing incompatible options), since we're testing the driver's internal state transitions, not successful compilation.

To run this test:
