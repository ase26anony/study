This test program includes:

1. **All 8 Partition Cases**: Each test function targets a specific partition type with appropriate OpenACC directives.

2. **Data Mapping Variations**: Uses `copy`, `copyin/out`, `present`, `private`, and reduction operations with different data types.

3. **Loop Structures**: Includes single loops, nested loops with `collapse`, and triangular iteration spaces.

4. **Runtime Parameters**: Uses dynamic sizes and runtime-determined loop bounds.

5. **Verification**: Each test includes basic verification to ensure correctness.

To compile and run:
