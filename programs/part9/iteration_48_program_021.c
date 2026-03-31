This script provides comprehensive coverage testing by:

1. **Creating a test program** with a simple loop that gives the optimizer something to work with
2. **Testing a wide range of x86 architectures** from old Pentium 4 to modern Zen 4 and Alder Lake
3. **Using various optimization levels** (O0 through O3, Os, Oz, Ofast)
4. **Including special compilation modes** like LTO and PGO
5. **Attempting environment variable simulation** for specific CPUID data
6. **Gracefully handling compilation failures** and continuing with other tests

The script systematically exercises the cache detection logic by forcing GCC to:
- Query CPUID information for each `-march` target
- Process cache descriptor values specific to each microarchitecture
- Build accurate cache models for optimization decisions

To use this script:
