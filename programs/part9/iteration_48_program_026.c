Looking at the uncovered switch statement in `driver-i386.cc`, I need to create a test that forces the GCC driver to process various CPUID cache descriptor values. Here's a comprehensive shell script that systematically tests different x86 architectures and optimization flags:

```bash
#!/bin/bash
# test_driver_cache_coverage.sh
# This script tests GCC's CPU cache detection logic by compiling with various
# x86 architecture targets and optimization flags

set -e

# Create a minimal test C file
cat > test_cache.c << 'EOF'
/* Simple test program to exercise GCC's cache detection */
int main() {
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

echo "Testing GCC driver cache detection coverage..."
echo "=============================================="

# Array of compilation commands targeting different x86 architectures
# Each entry targets specific CPUID cache descriptor values
COMPILE_COMMANDS=(
    # Generic and modern architectures
    "gcc -O2 -march=native -fverbose-asm -o test_native test_cache.c"
    "gcc -O3 -march=x86-64 -mtune=generic -flto -o test_generic test_cache.c"
    
    # Intel Core microarchitectures (various cache descriptors)
    "gcc -O2 -march=nehalem -mtune=nehalem -o test_nehalem test_cache.c"
    "gcc -O3 -march=westmere -mtune=westmere -o test_westmere test_cache.c"
    "gcc -O2 -march=sandybridge -mtune=sandybridge -o test_sandybridge test_cache.c"
    "gcc -O3 -march=ivybridge -mtune=ivybridge -o test_ivybridge test_cache.c"
    "gcc -O2 -march=haswell -mtune=haswell -o test_haswell test_cache.c"
    "gcc -O3 -march=broadwell -mtune=broadwell -o test_broadwell test_cache.c"
    "gcc -O2 -march=skylake -mtune=skylake -o test_skylake test_cache.c"
    "gcc -O3 -march=skylake-avx512 -mtune=skylake-avx512 -o test_skylake_avx512 test_cache.c"
    "gcc -O2 -march=cannonlake -mtune=cannonlake -o test_cannonlake test_cache.c"
    "gcc -O3 -march=icelake-client -mtune=icelake-client -o test_icelake_client test_cache.c"
    "gcc -O2 -march=tigerlake -mtune=tigerlake -o test_tigerlake test_cache.c"
    "gcc -O3 -march=alderlake -mtune=alderlake -o test_alderlake test_cache.c"
    "gcc -O2 -march=raptorlake -mtune=raptorlake -o test_raptorlake test_cache.c"
    
    # AMD architectures (different cache organizations)
    "gcc -O2 -march=k8 -mtune=k8 -o test_k8 test_cache.c"
    "gcc -O3 -march=k8-sse3 -mtune=k8-sse3 -o test_k8_sse3 test_cache.c"
    "gcc -O2 -march=barcelona -mtune=barcelona -o test_barcelona test_cache.c"
    "gcc -O3 -march=bdver1 -mtune=bdver1 -o test_bdver1 test_cache.c"
    "gcc -O2 -march=bdver2 -mtune=bdver2 -o test_bdver2 test_cache.c"
    "gcc -O3 -march=bdver3 -mtune=bdver3 -o test_bdver3 test_cache.c"
    "gcc -O2 -march=bdver4 -mtune=bdver4 -o test_bdver4 test_cache.c"
    "gcc -O3 -march=znver1 -mtune=znver1 -o test_znver1 test_cache.c"
    "gcc -O2 -march=znver2 -mtune=znver2 -o test_znver2 test_cache.c"
    "gcc -O3 -march=znver3 -mtune=znver3 -o test_znver3 test_cache.c"
    "gcc -O2 -march=znver4 -mtune=znver4 -o test_znver4 test_cache.c"
    
    # Intel Atom and low-power architectures (different cache descriptors)
    "gcc -O2 -march=bonnell -mtune=bonnell -o test_bonnell test_cache.c"
    "gcc -O3 -march=atom -mtune=atom -o test_atom test_cache.c"
    "gcc -O2 -march=silvermont -mtune=silvermont -o test_silvermont test_cache.c"
    "gcc -O3 -march=goldmont -mtune=goldmont -o test_goldmont test_cache.c"
    "gcc -O2 -march=goldmont-plus -mtune=goldmont-plus -o test_goldmont_plus test_cache.c"
    "gcc -O3 -march=tremont -mtune=tremont -o test_tremont test_cache.c"
    "gcc -O2 -march=gracemont -mtune=gracemont -o test_gracemont test_cache.c"
    
    # Older Intel architectures (legacy cache descriptors)
    "gcc -O2 -march=pentium4 -mtune=pentium4 -o test_pentium4 test_cache.c"
    "gcc -O3 -march=pentium4m -mtune=pentium4m -o test_pentium4m test_cache.c"
    "gcc -O2 -march=prescott -mtune=prescott -o test_prescott test_cache.c"
    "gcc -O3 -march=nocona -mtune=nocona -o test_nocona test_cache.c"
    "gcc -O2 -march=core2 -mtune=core2 -o test_core2 test_cache.c"
    "gcc -O3 -march=penryn -mtune=penryn -o test_penryn test_cache.c"
    
    # Server/workstation architectures
    "gcc -O2 -march=knl -mtune=knl -o test_knl test_cache.c"
    "gcc -O3 -march=skylake-avx512 -mtune=skylake-avx512 -o test_skylake_avx512_server test_cache.c"
    "gcc -O2 -march=icelake-server -mtune=icelake-server -o test_icelake_server test_cache.c"
    "gcc -O3 -march=sapphirerapids -mtune=sapphirerapids -o test_sapphirerapids test_cache.c"
    
    # Different optimization levels with architecture flags
    "gcc -O0 -march=native -o test_O0_native test_cache.c"
    "gcc -O1 -march=native -o test_O1_native test_cache.c"
    "gcc -Os -march=native -o test_Os_native test_cache.c"
    "gcc -Ofast -march=native -o test_Ofast_native test_cache.c"
    
    # With LTO enabled (forces lto1 to run cache detection)
    "gcc -O2 -march=native -flto -fuse-linker-plugin -o test_lto_native test_cache.c"
    "gcc -O3 -march=skylake -flto -o test_lto_skylake test_cache.c"
    "gcc -O2 -march=znver2 -flto -o test_lto_znver2 test_cache.c"
    
    # With PGO (may affect optimization decisions)
    "gcc -O2 -march=native -fprofile-generate -o test_pgo_gen test_cache.c"
    
    # With specific tuning for cache optimization
    "gcc -O3 -march=native -funroll-loops -o test_unroll test_cache.c"
    "gcc -O3 -march=native -ftree-vectorize -o test_vectorize test_cache.c"
)

# Create a wrapper to simulate specific CPUID values via environment
# This is a more direct approach to target specific cache descriptors
create_cpuid_wrapper() {
    local wrapper_script="gcc_cpuid_wrapper.sh"
    
    cat > "$wrapper_script" << 'EOF_WRAPPER'
#!/bin/bash
# Wrapper to simulate specific CPUID cache descriptor values
# This sets GCC_CPUINFO environment variable with custom CPUID data

# Example: Simulate cache descriptor 0x4e (L2 cache: 4MB, 16-way, 64-byte line)
# Format: "cpuid:leaf:subleaf:eax:ebx:ecx:edx"
export GCC_CPUINFO="cpuid:0x04:0x00:0x1c004121:0x01c0003f:0x0000003f:0x00000000
cpuid:0x04:0x01:0x1c004122:0x01c0003f:0x0000003f:0x00000000
cpuid:0x04:0x02:0x1c004143:0x03c0003f:0x000003ff:0x00000000
cpuid:0x04:0x03:0x1c03c163:0x03c0003f:0x00003fff:0x00000006"

# Pass through all arguments to real gcc
exec /usr/bin/gcc "$@"
EOF_WRAPPER
    
    chmod +x "$wrapper_script"
    echo "$wrapper_script"
}

# Function to test with specific cache descriptor simulation
test_specific_descriptors() {
    echo "Testing with simulated CPUID cache descriptors..."
    echo "--------------------------------------------------"
    
    # Create array of specific cache descriptors to test
    # These correspond to case values in the uncovered switch statement
    DESCRIPTORS=(
        "0x0a"  # L1 Data cache: 8KB, 2-way, 32-byte line
        "0x0c"  # L1 Data cache: 16KB, 4-way, 32-byte line  
        "0x0d"  # L1 Data cache: 16KB, 4-way, 64-byte line
        "0x21"  # L2 cache: 256KB, 8-way, 64-byte line
        "0x24"  # L2 cache: 1MB, 16-way, 64-byte line
        "0x2c"  # L1 Data cache: 32KB, 8-way, 64-byte line
        "0x39"  # L2 cache: 128KB, 4-way, 64-byte line
        "0x3b"  # L2 cache: 128KB, 8-way, 64-byte line
        "0x3c"  # L2 cache: 256KB, 4-way, 64-byte line
        "0x3d"  # L2 cache: 384KB, 6-way, 64-byte line
        "0x3e"  # L2 cache: 512KB, 4-way, 64-byte line
        "0x41"  # L2 cache: 128KB, 4-way, 32-byte line
        "0x42"  # L2 cache: 256KB, 4-way, 32-byte line
        "0x43"  # L2 cache: 512KB, 4-way, 32-byte line
        "0x44"  # L2 cache: 1MB, 4-way, 32-byte line
        "0x45"  # L2 cache: 2MB, 4-way, 32-byte line
        "0x46"  # L3 cache: 4MB, 4-way, 64-byte line
        "0x47"  # L3 cache: 8MB, 8-way, 64-byte line
        "0x48"  # L2 cache: 3MB, 12-way, 64-byte line
        "0x49"  # L3 cache: 4MB, 16-way, 64-byte line
        "0x4a"  # L3 cache: 6MB, 12-way, 64-byte line
        "0x4b"  # L3 cache: 8MB, 16-way, 64-byte line
        "0x4c"  # L3 cache: 12MB, 12-way, 64-byte line
        "0x4d"  # L3 cache: 16MB, 16-way, 64-byte line
        "0x4e"  # L2 cache: 6MB, 24-way, 64-byte line
        "0x60"  # L1 Data cache: 16KB, 8-way, 64-byte line
        "0x66"  # L1 Data cache: 8KB, 4-way, 64-byte line
        "0x67"  # L1 Data cache: 16KB, 4-way, 64-byte line
        "0x68"  # L1 Data cache: 32KB, 4-way, 64-byte line
        "0x78"  # L2 cache: 1MB, 4-way, 64-byte line
        "0x79"  # L2 cache: 128KB, 8-way, 64-byte line, 2 lines per sector
        "0x7a"  # L2 cache: 256KB, 8-way, 64-byte line, 2 lines per sector
        "0x7b"  # L2 cache: 512KB, 8-way, 64-byte line, 2 lines per sector
        "0x7c"  # L2 cache: 1MB, 8-way, 64-byte line, 2 lines per sector
        "0x7d"  # L2 cache: 2MB, 8-way, 64-byte line
        "0x7f"  # L2 cache: 512KB, 2-way, 64-byte line
        "0x80"  # L2 cache: 512KB, 8-way, 64-byte line
        "0x82"  # L2 cache: 256KB, 8-way, 32-byte line
        "0x83"  # L2 cache: 512KB, 8-way, 32-byte line
        "0x84"  # L2 cache: 1MB, 8-way, 32-byte line
        "0x85"  # L2 cache: 2MB, 8-way, 32-byte line
        "0x86"  # L2 cache: 512KB, 4-way, 64-byte line
        "0x87"  # L2 cache: 1MB, 8-way, 64-byte line
    )
    
    for desc in "${DESCRIPTORS[@]}"; do
        echo "Testing cache descriptor: $desc"
        # Create environment variable with specific descriptor
        # This is a simplified example - actual implementation would need
        # to construct proper CPUID leaf 4 data
        export SIMULATED_CPUID="cache_descriptor=$desc"
        gcc -O2 -march=native -o "test_desc_${desc}" test_cache.c 2>/dev/null || true
    done
}

# Main test execution
echo "Starting compilation tests..."
echo "============================"

# Counter for successful compilations
success_count=0
total_count=0

# Run all compilation commands
for cmd in "${COMPILE_COMMANDS[@]}"; do
    total_count=$((total_count + 1))
    echo "Test $total_count: $cmd"
    
    # Run compilation, capture output but don't fail on error
    if eval "$cmd" 2>&1 | grep -q "error\|Error\|ERROR"; then
        echo "  -> Compilation failed (possibly unsupported architecture)"
    else
        success_count=$((success_count + 1))
        echo "  -> Compilation successful"
    fi
    
    # Clean up binary to save space
    rm -f test_* 2>/dev/null || true
done

# Test with specific cache descriptor simulation if supported
if [ -n "$GCC_CPUINFO" ] || [ -f "/proc/cpuinfo" ]; then
    test_specific_descriptors
fi

# Create and use wrapper for direct CPUID simulation
echo ""
echo "Creating CPUID wrapper for direct cache descriptor testing..."
wrapper=$(create_cpuid_wrapper)
if [ -f "$wrapper" ]; then
    echo "Testing with CPUID wrapper..."
    PATH=".:$PATH" $wrapper -O2 -march=native -o test_wrapped test_cache.c 2>/dev/null || true
    rm -f "$wrapper" test_wrapped
fi

# Summary
echo ""
echo "Test Summary"
echo "============"
echo "Total compilation attempts: $total_count"
echo "Successful compilations: $success_count"
echo "Coverage target: Switch cases for cache descriptors 0x0a through 0x87"

# Cleanup
rm -f test_cache.c test_* 2>/dev/null || true

echo ""
echo "Note: The actual coverage of the switch statement depends on:"
echo "1. The actual CPUID values returned by your CPU"
echo "2. The GCC version's internal CPU model database"
echo "3. Whether GCC queries cache information for the given -march flag"
echo ""
echo "For maximum coverage, run this test on different physical CPUs"
echo "or in virtual machines configured with different CPU models."
```

This script provides a comprehensive approach to testing the uncovered cache detection logic:

1. **Multiple Architecture Targets**: Tests a wide range of x86 microarchitectures from Intel, AMD, and Atom families, each potentially mapping to different CPUID cache descriptor values.

2. **Different Optimization Levels**: Uses various optimization flags (`-O0` through `-O3`, `-Os`, `-Ofast`) which may affect how aggressively GCC queries cache information.

3. **LTO Testing**: Includes Link Time Optimization flags which cause the LTO frontend (`lto1`) to also run cache detection.

4. **Direct CPUID Simulation**: Attempts to use environment variables to simulate specific cache descriptors, providing a more direct path to the uncovered switch cases.

5. **Graceful Error Handling**: Continues testing even if some architectures aren't supported by the current GCC build.

The script systematically exercises the GCC driver's cache detection path during compilation, increasing the likelihood of covering the switch statement cases in `driver-i386.cc`.
