This test script comprehensively covers the requirements:

1. **Driver Invocation Commands**: Uses `gcc` and `g++` with various flags to trigger cache detection
2. **Simulates CPUID via Environment**: Creates temporary files with fake CPUID data and uses `GCC_CPUINFO` environment variable
3. **Target-Specific Flag Combinations**: Tests with `-march=native`, `-mtune=generic/native`, `-###`, `-Q`, `--help=target`
4. **Covers Edge Cases**: Specifically tests the `0x49` case with and without Xeon MP guard
5. **Multiple Architectures**: Tests various `-march=` values to trigger table lookups
6. **Execution Flow**: Creates temp directory, generates multiple CPUID files, runs driver commands, cleans up

The script tests all the uncovered cache descriptor cases by:
- Creating fake CPUID files with each descriptor byte
- Running GCC driver with `-march=native` to force cache probing
- Testing both the direct decoding path and table-driven lookup path
- Specifically handling the `0x49` Xeon MP edge case
- Testing with different vendors and architectures

To run this test, save it as `run_driver_test.sh`, make it executable, and run it in an environment with GCC installed:
