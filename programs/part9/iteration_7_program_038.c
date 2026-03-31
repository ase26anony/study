This test script creates multiple compilation commands targeting CPU architectures known to have specific cache descriptor patterns:

1. **Pentium III family** (`-march=pentium3`, `-march=pentium-m`): Targets descriptors 0x0a, 0x0c, 0x0d, 0x0e
2. **Core/Core 2 family** (`-march=core2`, `-march=nehalem`): Targets descriptors 0x2c, 0x21, 0x39-0x3e
3. **Sandy Bridge and later** (`-march=sandybridge`, `-march=skylake`): Targets descriptors 0x41-0x45, 0x48, 0x49
4. **AMD processors** (`-march=k8`, `-march=znver1`): Covers AMD-specific cache configurations
5. **Generic probing** (`-mtune=generic`, `-march=x86-64`): Forces full CPUID detection
6. **Native detection** (`-march=native`): Uses actual host CPUID values

The script also includes:
- Verbose output filtering for cache-related messages
- Explicit cache parameter testing with `--param`
- Multiple optimization levels to ensure driver execution
- A Makefile alternative for batch processing

To run the test:
