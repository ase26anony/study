This script comprehensively tests the uncovered cache descriptor decoding logic by:

1. **Creating fake CPUID files** for each cache descriptor byte mentioned in the uncovered lines
2. **Testing the 0x49 case with xeon_mp guard** by creating two variants with different family/model values
3. **Using GCC_CPUINFO environment variable** to force the driver to use our fake CPUID data
4. **Invoking the driver with -march=native** which triggers cache probing
5. **Testing table-driven cache lookup** with various -march values
6. **Testing both Intel and AMD vendor strings**
7. **Using various driver flags** (-###, -Q, --help=target, -dumpspecs) to trigger different initialization paths

The script generates temporary CPUID files in the format expected by the GCC driver, with proper vendor strings, family/model information, and cache descriptor bytes placed in the correct CPUID leaves. Each test runs the GCC driver with the fake CPUID data and checks for cache-related output.

To run this test, save it as `run_driver_test.sh`, make it executable, and run it in an environment with GCC installed:
