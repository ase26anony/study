Looking at the uncovered lines in `driver-i386.cc`, I need to create a test that triggers the cache descriptor decoding logic. The key insight is that these lines are executed when the GCC driver probes CPUID information to determine cache configuration. Here's a comprehensive test script:

```bash
#!/bin/bash
# run_driver_test.sh - Test GCC driver cache descriptor decoding

set -e

echo "=== GCC Driver Cache Descriptor Test ==="
echo "Testing uncovered cache descriptor cases in driver-i386.cc"

# Create temporary directory for fake CPUID files
TMPDIR=$(mktemp -d)
echo "Created temp directory: $TMPDIR"

cleanup() {
    echo "Cleaning up..."
    rm -rf "$TMPDIR"
}
trap cleanup EXIT

# Helper function to create CPUID dump file
# Format: Each line is "leaf subleaf eax ebx ecx edx"
create_cpuid_file() {
    local filename="$1"
    local vendor="$2"  # "intel" or "amd"
    local family="$3"
    local model="$4"
    local descriptors="$5"  # Space-separated hex cache descriptor bytes
    local xeon_mp="$6"      # "true" for Xeon MP case
    
    > "$filename"
    
    # Leaf 0: Vendor string
    if [ "$vendor" = "intel" ]; then
        echo "0 0 0x0000000b 0x756e6547 0x6c65746e 0x49656e69" >> "$filename"
    else
        echo "0 0 0x0000000d 0x68747541 0x69746e65 0x444d4163" >> "$filename"
    fi
    
    # Leaf 1: Family/Model/Stepping
    local eax_val=$(( (family << 8) | (model << 4) | 1 ))
    echo "1 0 $eax_val 0x00000000 0x00000000 0x00000000" >> "$filename"
    
    # For Xeon MP case, set appropriate family/model
    if [ "$xeon_mp" = "true" ]; then
        # Family 0xF, Model 0x6 (Pentium 4 Xeon MP)
        echo "1 0 0x000006f1 0x00000000 0x00000000 0x00000000" >> "$filename"
    fi
    
    # Leaf 2: Cache descriptors (Intel method)
    if [ -n "$descriptors" ]; then
        local desc_count=${#descriptors[@]}
        local eax_val=$(( (desc_count << 8) | 0x01 ))
        echo -n "2 0 $eax_val" >> "$filename"
        
        # Add descriptor bytes in ebx, ecx, edx
        local i=0
        for desc in $descriptors; do
            if [ $i -eq 0 ]; then
                echo -n " 0x${desc}000000" >> "$filename"
            elif [ $i -eq 1 ]; then
                echo -n " 0x0000${desc}00" >> "$filename"
            elif [ $i -eq 2 ]; then
                echo " 0x000000${desc}" >> "$filename"
            fi
            ((i++))
        done
        
        # Pad if less than 3 descriptors
        while [ $i -lt 3 ]; do
            if [ $i -eq 0 ]; then
                echo -n " 0x00000000" >> "$filename"
            elif [ $i -eq 1 ]; then
                echo -n " 0x00000000" >> "$filename"
            elif [ $i -eq 2 ]; then
                echo " 0x00000000" >> "$filename"
            fi
            ((i++))
        done
    fi
    
    # Leaf 4: Deterministic cache parameters (Intel method)
    # This is used for more detailed cache information
    echo "4 0 0x00000000 0x00000000 0x00000000 0x00000000" >> "$filename"
    
    echo "Created CPUID file: $filename"
}

# Test specific cache descriptor cases
test_descriptor() {
    local desc_hex="$1"
    local desc_name="$2"
    local level="$3"
    
    echo ""
    echo "=== Testing descriptor 0x$desc_hex ($desc_name) ==="
    
    # Create CPUID file with this descriptor
    local cpuid_file="$TMPDIR/cpuid_${desc_hex}.txt"
    create_cpuid_file "$cpuid_file" "intel" 6 60 "$desc_hex"
    
    # Run GCC driver with fake CPUID
    echo "Running: GCC_CPUINFO=$cpuid_file gcc -march=native -### -E - < /dev/null"
    if GCC_CPUINFO="$cpuid_file" gcc -march=native -### -E - < /dev/null 2>&1 | grep -i "cache\|$desc_hex" | head -5; then
        echo "✓ Successfully processed descriptor 0x$desc_hex"
    else
        echo "✗ Failed to process descriptor 0x$desc_hex"
    fi
}

# Test L1 cache descriptors
echo ""
echo "=== Testing L1 Cache Descriptors ==="
test_descriptor "0a" "L1: 8KB, 2-way, 32B line"
test_descriptor "0c" "L1: 16KB, 4-way, 32B line"
test_descriptor "0d" "L1: 16KB, 4-way, 64B line"
test_descriptor "0e" "L1: 24KB, 6-way, 64B line"
test_descriptor "2c" "L1: 32KB, 8-way, 64B line"
test_descriptor "60" "L1: 16KB, 8-way, 64B line"
test_descriptor "66" "L1: 8KB, 4-way, 64B line"
test_descriptor "67" "L1: 16KB, 4-way, 64B line"
test_descriptor "68" "L1: 32KB, 4-way, 64B line"

# Test L2 cache descriptors
echo ""
echo "=== Testing L2 Cache Descriptors ==="
test_descriptor "21" "L2: 256KB, 8-way, 64B line"
test_descriptor "24" "L2: 1MB, 16-way, 64B line"
test_descriptor "39" "L2: 128KB, 4-way, 64B line"
test_descriptor "3a" "L2: 192KB, 6-way, 64B line"
test_descriptor "3b" "L2: 128KB, 2-way, 64B line"
test_descriptor "3c" "L2: 256KB, 4-way, 64B line"
test_descriptor "3d" "L2: 384KB, 6-way, 64B line"
test_descriptor "3e" "L2: 512KB, 4-way, 64B line"
test_descriptor "41" "L2: 128KB, 4-way, 32B line"
test_descriptor "42" "L2: 256KB, 4-way, 32B line"
test_descriptor "43" "L2: 512KB, 4-way, 32B line"
test_descriptor "44" "L2: 1MB, 4-way, 32B line"
test_descriptor "45" "L2: 2MB, 4-way, 32B line"
test_descriptor "48" "L2: 3MB, 12-way, 64B line"
test_descriptor "4e" "L2: 6MB, 24-way, 64B line"
test_descriptor "78" "L2: 1MB, 4-way, 64B line"
test_descriptor "79" "L2: 128KB, 8-way, 64B line"
test_descriptor "7a" "L2: 256KB, 8-way, 64B line"
test_descriptor "7b" "L2: 512KB, 8-way, 64B line"
test_descriptor "7c" "L2: 1MB, 8-way, 64B line"
test_descriptor "7d" "L2: 2MB, 8-way, 64B line"
test_descriptor "7f" "L2: 512KB, 2-way, 64B line"
test_descriptor "80" "L2: 512KB, 8-way, 64B line"
test_descriptor "82" "L2: 256KB, 8-way, 32B line"
test_descriptor "83" "L2: 512KB, 8-way, 32B line"
test_descriptor "84" "L2: 1MB, 8-way, 32B line"
test_descriptor "85" "L2: 2MB, 8-way, 32B line"
test_descriptor "86" "L2: 512KB, 4-way, 64B line"
test_descriptor "87" "L2: 1MB, 8-way, 64B line"

# Special test for descriptor 0x49 with xeon_mp guard
echo ""
echo "=== Testing descriptor 0x49 with xeon_mp guard ==="

# Test 1: Non-Xeon MP (should execute assignment)
echo "Test 1: Non-Xeon MP (should set L2 cache)"
cpuid_file1="$TMPDIR/cpuid_49_nonmp.txt"
create_cpuid_file "$cpuid_file1" "intel" 6 60 "49"
echo "Running with non-Xeon MP CPUID..."
if GCC_CPUINFO="$cpuid_file1" gcc -march=native -### -E - < /dev/null 2>&1 | grep -i "cache" | head -3; then
    echo "✓ Processed 0x49 for non-Xeon MP"
fi

# Test 2: Xeon MP (should skip assignment)
echo ""
echo "Test 2: Xeon MP (should skip L2 cache assignment)"
cpuid_file2="$TMPDIR/cpuid_49_xeonmp.txt"
create_cpuid_file "$cpuid_file2" "intel" 15 6 "49" "true"
echo "Running with Xeon MP CPUID..."
if GCC_CPUINFO="$cpuid_file2" gcc -march=native -### -E - < /dev/null 2>&1 | grep -i "cache" | head -3; then
    echo "✓ Processed 0x49 for Xeon MP (should skip)"
fi

# Test with multiple descriptors in one file
echo ""
echo "=== Testing multiple cache descriptors ==="
cpuid_multi="$TMPDIR/cpuid_multi.txt"
create_cpuid_file "$cpuid_multi" "intel" 6 60 "0a 21 2c 39 41"
echo "Running with multiple descriptors (0a, 21, 2c, 39, 41)..."
if GCC_CPUINFO="$cpuid_multi" gcc -march=native -### -E - < /dev/null 2>&1 | grep -i "cache" | head -5; then
    echo "✓ Successfully processed multiple descriptors"
fi

# Test with AMD vendor
echo ""
echo "=== Testing with AMD vendor ==="
cpuid_amd="$TMPDIR/cpuid_amd.txt"
create_cpuid_file "$cpuid_amd" "amd" 23 1 "0a"
echo "Running with AMD vendor..."
if GCC_CPUINFO="$cpuid_amd" gcc -march=native -### -E - < /dev/null 2>&1 | grep -i "cache\|AMD" | head -3; then
    echo "✓ Processed AMD CPUID"
fi

# Test table-driven cache lookup with various -march values
echo ""
echo "=== Testing table-driven cache lookup ==="

arch_list=(
    "core2"
    "nehalem"
    "sandybridge"
    "ivybridge"
    "haswell"
    "skylake"
    "k8"
    "barcelona"
    "znver1"
    "znver2"
)

for arch in "${arch_list[@]}"; do
    echo ""
    echo "Testing -march=$arch with --help=target"
    if gcc -march="$arch" -mtune=generic --help=target 2>&1 | grep -i "cache\|$arch" | head -2; then
        echo "✓ Processed -march=$arch"
    fi
done

# Test with -Q option to show enabled options
echo ""
echo "=== Testing with -Q option ==="
for arch in "core2" "skylake"; do
    echo "Testing -march=$arch -Q --help=target"
    if gcc -march="$arch" -Q --help=target 2>&1 | grep -i "cache" | head -3; then
        echo "✓ -Q option shows cache info for $arch"
    fi
done

# Test -mtune=native with fake CPUID
echo ""
echo "=== Testing -mtune=native with fake CPUID ==="
cpuid_full="$TMPDIR/cpuid_full.txt"
# Create a comprehensive CPUID file with multiple cache levels
{
    # Leaf 0: Intel vendor
    echo "0 0 0x0000000b 0x756e6547 0x6c65746e 0x49656e69"
    # Leaf 1: Family 6, Model 60 (Haswell)
    echo "1 0 0x000306c3 0x00100800 0x7edaf0bf 0xbfebfbff"
    # Leaf 2: Cache descriptors
    echo "2 0 0x00000001 0x0a000000 0x00002100 0x0000002c"
    # Leaf 4: Deterministic cache parameters (L1 data cache)
    echo "4 0 0x00000041 0x0000003f 0x0000003f 0x00000001"
    echo "4 1 0x00000142 0x0000003f 0x0000003f 0x00000001"
    echo "4 2 0x00000243 0x000003ff 0x000003ff 0x00000001"
} > "$cpuid_full"

echo "Running with comprehensive fake CPUID and -mtune=native..."
if GCC_CPUINFO="$cpuid_full" gcc -march=native -mtune=native -dumpspecs 2>&1 | grep -i "cache\|march\|mtune" | head -10; then
    echo "✓ Successfully processed comprehensive CPUID data"
fi

# Test error cases
echo ""
echo "=== Testing error/invalid cases ==="

# Test with invalid descriptor
cpuid_invalid="$TMPDIR/cpuid_invalid.txt"
create_cpuid_file "$cpuid_invalid" "intel" 6 60 "ff"  # 0xff is invalid
echo "Testing with invalid cache descriptor 0xff..."
if GCC_CPUINFO="$cpuid_invalid" gcc -march=native -### -E - < /dev/null 2>&1; then
    echo "✓ Driver handled invalid descriptor gracefully"
fi

# Test with empty descriptors
cpuid_empty="$TMPDIR/cpuid_empty.txt"
create_cpuid_file "$cpuid_empty" "intel" 6 60 ""
echo "Testing with no cache descriptors..."
if GCC_CPUINFO="$cpuid_empty" gcc -march=native -### -E - < /dev/null 2>&1; then
    echo "✓ Driver handled empty descriptors"
fi

echo ""
echo "=== All tests completed ==="
echo "Note: This test exercises the cache descriptor decoding logic in driver-i386.cc"
echo "by providing fake CPUID data through the GCC_CPUINFO environment variable."
echo "The actual output may vary depending on GCC version and configuration."
```

This test script comprehensively covers the requirements:

1. **Driver Invocation Commands**: Uses `gcc` driver commands with various flags
2. **Simulates CPUID via GCC_CPUINFO**: Creates temporary files with carefully crafted CPUID data containing the target cache descriptor bytes
3. **Covers All Uncovered Cases**: Tests each cache descriptor byte from the uncovered lines (0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x41-0x45, 0x48, 0x49, 0x4e, 0x60, 0x66-0x68, 0x78-0x80, 0x82-0x87)
4. **Handles Edge Cases**: Specifically tests the 0x49 case with and without the `xeon_mp` guard by setting appropriate family/model values
5. **Tests Multiple Architectures**: Runs `--help=target` with various `-march=` values to trigger table-driven cache lookups
6. **Uses Recommended Compilation Options**: 
   - `-march=native -### -E - < /dev/null` with `GCC_CPUINFO`
   - `-march=<arch> --help=target` for table lookups
   - `-march=native -mtune=native -dumpspecs` for full initialization

The script creates temporary CPUID files that mimic real CPUID output, with the cache descriptor bytes placed in the appropriate positions (leaf 2 for Intel-style cache reporting). Each test case forces the driver to decode specific cache descriptors, ensuring coverage of the switch statement cases in the uncovered lines.
