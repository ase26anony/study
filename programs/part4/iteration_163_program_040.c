This script provides comprehensive testing of the uncovered lines by:

1. **Creating valid GCOV data files**: It compiles and runs two slightly different C programs to generate `.gcda` files with varying coverage patterns.

2. **Testing each individual option**: It systematically tests `-v`, `-f`, `-F`, `-o`, `-h`, and `-t` with valid arguments.

3. **Testing option combinations**: It combines multiple options to ensure they work together correctly.

4. **Testing edge cases**: 
   - Boundary values for `-t` (0.0, 1.0)
   - Out-of-range values for `-t` (-1.0, 2.5)
   - Scientific notation for `-t` (5e-1)
   - Invalid option to trigger the `default` case

5. **Validating execution**: Each test checks that the command executes without crashing, and some tests check for expected output patterns.

To run this test, save it as `test_gcov_tool_overlap.sh`, make it executable, and run it:
