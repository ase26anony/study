## 2. Individual Test Files (already embedded in script, but here's a summary):

### test1_default.gt
Focuses on triggering the `default:` case with:
- Semicolons, commas, asterisks in type definitions
- Preprocessor directives
- Comments with various punctuation

### test2_balanced.gt
Tests `consume_balanced` with:
- Function pointers with nested parentheses
- Arrays of pointers
- Nested unions/structs
- Complex type signatures

### test3_mixed.gt
Combines default and balanced cases:
- Empty balanced pairs
- Multiple punctuators in sequence
- Mixed nested delimiters
- Preprocessor conditions

### test4_errors.gt
Tests error recovery:
- Missing semicolons
- Unclosed delimiters
- Mismatched brackets
- Gibberish input

### test5_literals.gt
Ensures delimiters in strings aren't parsed:
- String literals with parentheses, braces, brackets
- Character constants

### test6_stress.gt
Stress tests with deep nesting:
- Extremely nested parentheses
- Complex array/function combinations
- Mixed delimiter types

### test7_walk.gt
Tests walk mode if supported

## 3. Makefile for Easy Execution
