This script systematically tests all the code paths in the uncovered block:

1. **Individual flag testing**: Each case in the switch statement (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) is tested individually.

2. **Flag combinations**: Tests various combinations like `-v -f -o` and `-F -h -t 0.75` to ensure flags work together.

3. **Threshold variations**: Tests `-t` with integer (1), fractional (0.5, 0.33), edge (0, 0.001, 100) values.

4. **Flag ordering**: Tests flags in different positions relative to input files.

5. **Invalid option**: Tests `-x` to trigger the `default:` case and call `overlap_usage()`.

6. **Output redirection**: Tests with output redirected to ensure the code paths execute.

7. **Multi-file merge**: Always uses at least two `.gcda` files to ensure merge logic is exercised.

8. **Cleanup**: Removes all generated files to keep the test self-contained.

To run this test:
