To compile and run this test as part of GCC's self-test suite:

1. **Save the file** as `fixed-value-test.cc` in GCC's source tree (e.g., in the `gcc/testsuite/selftest/` directory).

2. **Register the test** by adding it to the appropriate test registration mechanism in GCC. This typically involves modifying a test registry file to include `fixed_value_cc_tests()`.

3. **Compile GCC with self-tests enabled**:
