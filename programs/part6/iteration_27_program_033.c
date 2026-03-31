This script provides comprehensive coverage of all the requirements:

1. **Individual flag testing**: Each flag (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) is tested individually with valid arguments for `-t`.

2. **Flag combinations**: Multiple combinations are tested including `-f -o`, `-F -h -t 1.0`, and the comprehensive `-v -f -F -o -h -t 5.0`.

3. **Invalid flag testing**: The `-Z` flag triggers the `default` case in the switch statement, calling `overlap_usage()`.

4. **Multiple input files**: The script generates 4 different `.gcda` files and tests with multiple files simultaneously.

5. **Directory inputs**: Creates separate directories with profile data to test directory processing.

6. **Varied threshold values**: Tests edge cases with very low (0.01) and very high (99.9) thresholds.

7. **Verbose output handling**: Captures verbose output to files and shows a sample.

8. **Different execution paths**: The test program is run with different arguments to generate varied profile data.

To run this script:
