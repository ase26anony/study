To compile and run this test as part of GCC's self-test suite:

1. **Save the file** as `test-fixed-value-range.cc` in GCC's source tree under the `gcc/testsuite/selftest/` directory or similar.

2. **Register the test** by adding it to the appropriate test registration mechanism in GCC. This typically involves adding a call to `fixed_value_cc_tests()` in the main selftest initialization.

3. **Compile GCC with self-tests enabled**:
