This script comprehensively tests all the uncovered lines in the `gcov-tool.cc` overlap analysis flag parsing logic:

1. **Individual flag testing**: Tests each flag (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) separately with valid arguments.

2. **Flag combinations**: Tests various combinations of flags together to ensure the parser handles multiple options correctly.

3. **Invalid flag testing**: Tests `-Z` to trigger the `default` case and call `overlap_usage()`.

4. **Multiple input files**: Creates and uses multiple `.gcda` files from different program runs.

5. **Verbose output**: Uses `-v` flag and redirects output to ensure the code path is executed.

6. **Different threshold values**: Tests various floating-point values for `-t` flag.

7. **Different file patterns**: Tests with wildcards and files in subdirectories.

8. **Different compilation options**: Tests with both `-O0` and `-O2` compiled programs.

To run the script:
