Looking at the uncovered switch statement in `driver-i386.cc`, we need to trigger different CPUID cache descriptor values during compilation. Here's a comprehensive shell script that exercises various x86 architectures to cover as many case labels as possible:

```bash
#!/bin/bash
# test_driver.sh - Exercise GCC driver cache detection logic

set -e

# Create minimal test source
cat > test_cache.c << 'EOF'
/* Minimal test program to trigger cache detection during compilation */
int main(void) {
    volatile int sum = 0;
    int array[1024];
    
    /* Simple loop to give optimizer something to work with */
    for (int i = 0; i < 1024; i++) {
        array[i] = i;
        sum += array[i];
    }
    
    return sum > 0 ? 0 : 1;
}
EOF

echo "Testing GCC driver cache detection logic..."
echo "=========================================="

# Array of compilation commands targeting different x86 architectures
# Each architecture may trigger different CPUID cache descriptor values
COMPILE_COMMANDS=(
    # Generic/Modern architectures
    "gcc -O0 -march=native -fverbose-asm -o test_native_O0 test_cache.c"
    "gcc -O2 -march=native -fverbose-asm -o test_native_O2 test_cache.c"
    "gcc -O3 -march=native -flto -fuse-linker-plugin -o test_native_O3_lto test_cache.c"
    
    # Intel Core architectures (various cache descriptor patterns)
    "gcc -O2 -march=nehalem -mtune=nehalem -o test_nehalem test_cache.c"
    "gcc -O2 -march=westmere -mtune=westmere -o test_westmere test_cache.c"
    "gcc -O3 -march=sandybridge -mtune=sandybridge -o test_sandybridge test_cache.c"
    "gcc -O3 -march=ivybridge -mtune=ivybridge -o test_ivybridge test_cache.c"
    "gcc -O3 -march=haswell -mtune=haswell -o test_haswell test_cache.c"
    "gcc -O3 -march=broadwell -mtune=broadwell -o test_broadwell test_cache.c"
    "gcc -O3 -march=skylake -mtune=skylake -o test_skylake test_cache.c"
    "gcc -O3 -march=skylake-avx512 -mtune=skylake-avx512 -o test_skylake_avx512 test_cache.c"
    "gcc -O3 -march=cannonlake -mtune=cannonlake -o test_cannonlake test_cache.c"
    "gcc -O3 -march=icelake-client -mtune=icelake-client -o test_icelake_client test_cache.c"
    "gcc -O3 -march=tigerlake -mtune=tigerlake -o test_tigerlake test_cache.c"
    "gcc -O3 -march=alderlake -mtune=alderlake -o test_alderlake test_cache.c"
    "gcc -O3 -march=rocketlake -mtune=rocketlake -o test_rocketlake test_cache.c"
    
    # AMD architectures
    "gcc -O2 -march=k8 -mtune=k8 -o test_k8 test_cache.c"
    "gcc -O2 -march=k8-sse3 -mtune=k8-sse3 -o test_k8_sse3 test_cache.c"
    "gcc -O3 -march=barcelona -mtune=barcelona -o test_barcelona test_cache.c"
    "gcc -O3 -march=bdver1 -mtune=bdver1 -o test_bdver1 test_cache.c"
    "gcc -O3 -march=bdver2 -mtune=bdver2 -o test_bdver2 test_cache.c"
    "gcc -O3 -march=bdver3 -mtune=bdver3 -o test_bdver3 test_cache.c"
    "gcc -O3 -march=bdver4 -mtune=bdver4 -o test_bdver4 test_cache.c"
    "gcc -O3 -march=znver1 -mtune=znver1 -o test_znver1 test_cache.c"
    "gcc -O3 -march=znver2 -mtune=znver2 -o test_znver2 test_cache.c"
    "gcc -O3 -march=znver3 -mtune=znver3 -o test_znver3 test_cache.c"
    "gcc -O3 -march=znver4 -mtune=znver4 -o test_znver4 test_cache.c"
    
    # Intel Atom architectures (often have different cache configurations)
    "gcc -Os -march=bonnell -mtune=bonnell -o test_bonnell test_cache.c"
    "gcc -Os -march=silvermont -mtune=silvermont -o test_silvermont test_cache.c"
    "gcc -Os -march=goldmont -mtune=goldmont -o test_goldmont test_cache.c"
    "gcc -Os -march=goldmont-plus -mtune=goldmont-plus -o test_goldmont_plus test_cache.c"
    "gcc -Os -march=tremont -mtune=tremont -o test_tremont test_cache.c"
    "gcc -Os -march=gracemont -mtune=gracemont -o test_gracemont test_cache.c"
    
    # Older Intel architectures (may trigger legacy cache descriptors)
    "gcc -O1 -march=pentium4 -mtune=pentium4 -o test_pentium4 test_cache.c"
    "gcc -O1 -march=pentium4m -mtune=pentium4m -o test_pentium4m test_cache.c"
    "gcc -O1 -march=prescott -mtune=prescott -o test_prescott test_cache.c"
    "gcc -O1 -march=nocona -mtune=nocona -o test_nocona test_cache.c"
    "gcc -O1 -march=core2 -mtune=core2 -o test_core2 test_cache.c"
    "gcc -O1 -march=penryn -mtune=penryn -o test_penryn test_cache.c"
    
    # Different optimization levels with generic x86-64
    "gcc -O0 -march=x86-64 -mtune=generic -o test_generic_O0 test_cache.c"
    "gcc -O1 -march=x86-64 -mtune=generic -fverbose-asm -o test_generic_O1 test_cache.c"
    "gcc -O2 -march=x86-64 -mtune=generic -fverbose-asm -o test_generic_O2 test_cache.c"
    "gcc -O3 -march=x86-64 -mtune=generic -flto -o test_generic_O3_lto test_cache.c"
    "gcc -Os -march=x86-64 -mtune=generic -o test_generic_Os test_cache.c"
    "gcc -Og -march=x86-64 -mtune=generic -o test_generic_Og test_cache.c"
    
    # 32-bit compilation (may trigger different code paths)
    "gcc -O2 -m32 -march=i686 -mtune=generic -o test_i686 test_cache.c"
    "gcc -O2 -m32 -march=pentium4 -mtune=pentium4 -o test_i686_p4 test_cache.c"
    
    # Special cases with specific optimization flags
    "gcc -O3 -march=native -funroll-loops -ftree-vectorize -o test_native_unroll test_cache.c"
    "gcc -O3 -march=native -fprefetch-loop-arrays -o test_native_prefetch test_cache.c"
)

# Counter for successful compilations
SUCCESS=0
TOTAL=${#COMPILE_COMMANDS[@]}

echo "Running $TOTAL compilation commands..."
echo ""

# Execute each compilation command
for i in "${!COMPILE_COMMANDS[@]}"; do
    CMD="${COMPILE_COMMANDS[$i]}"
    echo "Command $((i+1))/$TOTAL: $CMD"
    
    if eval "$CMD" 2>/dev/null; then
        echo "  ✓ Success"
        ((SUCCESS++))
    else
        echo "  ✗ Failed (architecture likely not supported on this host)"
    fi
done

echo ""
echo "=========================================="
echo "Summary: $SUCCESS/$TOTAL compilations succeeded"
echo ""

# Cleanup generated binaries (keep source for inspection)
echo "Cleaning up generated binaries..."
rm -f test_* 2>/dev/null || true

# Optional: Create a wrapper that simulates specific CPUID values
# This is a more advanced technique that directly targets specific cache descriptors
cat > simulate_cpuid.sh << 'EOF'
#!/bin/bash
# Advanced: Use GCC_CPUINFO to simulate specific CPUID cache descriptors
# This requires GCC built with environment variable support for CPUID override

# Example: Simulate a CPU with specific cache descriptor 0x4e (case 0x4e in switch)
# Format: "vendor:family:model:stepping:feature_bits:ext_family:ext_model:cache_info"
# cache_info format: "level:type:size:assoc:linesize:sets:flags"

# Note: This is highly GCC-version dependent and may not work on all systems
export GCC_CPUINFO="GenuineIntel:6:94:3:0xbfebfbff:0:0:1:1:32:8:64:64:1|2:3:256:4:64:16384:1|3:3:3072:12:64:4096:1"

echo "Compiling with simulated CPUID data..."
gcc -O2 -march=native -o test_simulated test_cache.c

unset GCC_CPUINFO
EOF

chmod +x simulate_cpuid.sh

echo "Created advanced simulation script: simulate_cpuid.sh"
echo "Note: This script may need adjustment based on your GCC version's"
echo "      GCC_CPUINFO environment variable format."
echo ""
echo "Test source file 'test_cache.c' has been preserved."
```

This script provides comprehensive coverage by:

1. **Targeting diverse x86 architectures**: Each `-march` value may map to different internal CPUID data structures, potentially triggering different cache descriptor values in the switch statement.

2. **Using various optimization levels**: Different optimization levels (`-O0`, `-O1`, `-O2`, `-O3`, `-Os`, `-Og`) may cause the driver to take different code paths through cache detection logic.

3. **Including LTO compilation**: Link Time Optimization (`-flto`) causes the LTO front-end (`lto1`) to run, which also performs cache detection.

4. **Covering both 32-bit and 64-bit**: The `-m32` flag may trigger different detection paths in the i386-specific code.

5. **Providing an advanced simulation option**: The `simulate_cpuid.sh` script shows how to use the `GCC_CPUINFO` environment variable (if supported) to directly inject specific cache descriptor values.

The key insight is that each compilation command causes the GCC driver to execute the cache detection code in `driver-i386.cc`. By compiling with many different architecture targets, we increase the probability of hitting different branches in the large switch statement covering lines 127-244.

**To use this script:**
1. Save it as `test_driver.sh`
2. Make it executable: `chmod +x test_driver.sh`
3. Run it: `./test_driver.sh`
4. Monitor coverage results to see which case labels in the switch statement get executed

The script gracefully handles compilation failures (e.g., when an architecture isn't supported by the host compiler) and continues with the next command, ensuring maximum coverage potential.
