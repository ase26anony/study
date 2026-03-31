This script comprehensively tests the uncovered cache descriptor decoding logic by:

1. **Creating fake CPUID files** with specific cache descriptor bytes (0x0a, 0x0c, 0x21, etc.)
2. **Using GCC_CPUINFO environment variable** to feed fake CPUID data to the driver
3. **Testing the special 0x49 case** with both regular Intel and Xeon MP configurations
4. **Exercising leaf 4 cache descriptors** which use a different decoding path
5. **Testing various -march values** to trigger internal cache table lookups
6. **Using different flag combinations** (-march=native, -mtune=generic, etc.) to ensure full code path coverage

The script creates temporary files with carefully crafted CPUID data that includes the specific cache descriptor bytes in the correct leaves (2 or 4), along with appropriate vendor, family, and model information to trigger the desired code paths.

To run this test:
