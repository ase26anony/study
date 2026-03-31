This script comprehensively tests all the uncovered lines in the specified block:

1. **Line-by-line coverage**:
   - `case 'v'`: Tested in Test 1
   - `case 'f'`: Tested in Test 2
   - `case 'F'`: Tested in Test 3
   - `case 'o'`: Tested in Test 4
   - `case 'h'`: Tested in Test 5
   - `case 't'`: Tested in Tests 6, 7, 11, 12
   - `default:` case: Tested in Test 10

2. **Key features**:
   - Creates minimal C programs with different control flow for meaningful overlap comparison
   - Generates valid `.gcda` files by compiling with `-fprofile-arcs -ftest-coverage`
   - Tests each option individually and in combination
   - Tests valid and invalid arguments for the `-t` option
   - Triggers the default case with invalid option `-x`
   - Tests edge cases (insufficient arguments, three files)
   - Includes proper cleanup

3. **To run the test**:
