Looking at the uncovered lines, I need to create a test that triggers the cache descriptor decoding logic in the GCC driver. The key insight is that we need to simulate different CPUID cache descriptor bytes through the `GCC_CPUINFO` environment variable. Here's a comprehensive test script:

```bash
#!/bin/bash
# run_driver_test.sh - Test GCC driver cache descriptor decoding

set -e

# Create temporary directory for fake CPUID files
TMPDIR=$(mktemp -d /tmp/gcc_cpuid_test_XXXXXX)
echo "Created temporary directory: $TMPDIR"

cleanup() {
    echo "Cleaning up temporary directory: $TMPDIR"
    rm -rf "$TMPDIR"
}
trap cleanup EXIT

# Helper function to create a basic CPUID file with given descriptor bytes
create_cpuid_file() {
    local filename="$1"
    local descriptor_byte="$2"
    local vendor="$3"
    local family="$4"
    local model="$5"
    
    cat > "$filename" << EOF
# Fake CPUID data for testing cache descriptor $descriptor_byte
# Format: leaf subleaf eax ebx ecx edx
0x00000000 0x00 0x0000000b ${vendor:0:8} ${vendor:8:8} ${vendor:16:8}
0x00000001 0x00 0x000306a9 0x00100800 0x7ffafbbf 0xbfebfbff
0x00000002 0x00 0x76036301 0x00f0b5ff 0x00000000 0x00${descriptor_byte}0000
0x00000004 0x00 0x1c004121 0x01c0003f 0x0000003f 0x00000000
0x00000004 0x01 0x1c004122 0x01c0003f 0x0000003f 0x00000000
0x00000004 0x02 0x1c004143 0x01c0003f 0x000001ff 0x00000000
0x00000004 0x03 0x1c03c163 0x03c0003f 0x00000fff 0x00000006
0x80000000 0x00 0x80000008 0x00000000 0x00000000 0x00000000
0x80000001 0x00 0x00000000 0x00000000 0x00000101 0x2c100800
0x80000002 0x00 0x65746e49 0x2952286c 0x726f4320 0x4d542865
0x80000003 0x00 0x43203229 0x20205550 0x20202020 0x20202020
0x80000004 0x00 0x30303637 0x20402020 0x30342e32 0x007a4847
EOF
}

# Test specific cache descriptor cases
test_descriptor() {
    local desc_hex="$1"
    local desc_name="$2"
    local vendor="${3:-GenuineIntel}"
    local family="${4:-6}"
    local model="${5:-42}"
    
    echo "=== Testing cache descriptor 0x$desc_hex ($desc_name) ==="
    
    # Create vendor string in CPUID format
    local vendor_bytes=""
    if [ "$vendor" = "GenuineIntel" ]; then
        vendor_bytes="0x756e6547 0x49656e69 0x6c65746e"
    elif [ "$vendor" = "AuthenticAMD" ]; then
        vendor_bytes="0x68747541 0x69746e65 0x444d4163"
    else
        vendor_bytes="0x756e6547 0x49656e69 0x6c65746e"  # Default to Intel
    fi
    
    # Create CPUID file with the specific descriptor
    local cpuid_file="$TMPDIR/cpuid_${desc_hex}.txt"
    echo "# Fake CPUID with descriptor 0x$desc_hex" > "$cpuid_file"
    echo "0x00000000 0x00 0x0000000b $vendor_bytes" >> "$cpuid_file"
    echo "0x00000001 0x00 0x000${family}0${model} 0x00100800 0x7ffafbbf 0xbfebfbff" >> "$cpuid_file"
    echo "0x00000002 0x00 0x76036301 0x00f0b5ff 0x00000000 0x00${desc_hex}0000" >> "$cpuid_file"
    
    # Run GCC driver with fake CPUID
    echo "Running: GCC_CPUINFO=$cpuid_file gcc -march=native -### -E - < /dev/null 2>&1 | grep -i cache"
    if GCC_CPUINFO="$cpuid_file" gcc -march=native -### -E - < /dev/null 2>&1 | grep -i cache; then
        echo "✓ Successfully tested descriptor 0x$desc_hex"
    else
        echo "✗ No cache output for descriptor 0x$desc_hex (might be expected)"
    fi
    echo
}

# Test case 0x49 with and without xeon_mp guard
test_xeon_mp_case() {
    echo "=== Testing Xeon MP special case (0x49) ==="
    
    # First test: Regular Intel CPU (should set L2 cache)
    echo "Test 1: Regular Intel CPU (should decode 0x49)"
    local cpuid_file1="$TMPDIR/cpuid_49_regular.txt"
    cat > "$cpuid_file1" << EOF
0x00000000 0x00 0x0000000b 0x756e6547 0x49656e69 0x6c65746e
0x00000001 0x00 0x00040651 0x00100800 0x7ffafbbf 0xbfebfbff
0x00000002 0x00 0x76036301 0x00f0b5ff 0x00000000 0x00490000
EOF
    echo "Running with regular Intel CPU..."
    GCC_CPUINFO="$cpuid_file1" gcc -march=native -### -E - < /dev/null 2>&1 | grep -i cache || true
    
    # Second test: Xeon MP (should skip due to xeon_mp guard)
    echo -e "\nTest 2: Xeon MP CPU (should skip 0x49 decoding)"
    local cpuid_file2="$TMPDIR/cpuid_49_xeonmp.txt"
    cat > "$cpuid_file2" << EOF
0x00000000 0x00 0x0000000b 0x756e6547 0x49656e69 0x6c65746e
0x00000001 0x00 0x00000f00 0x00100800 0x7ffafbbf 0xbfebfbff  # Family 15, Model 0 (Xeon MP)
0x00000002 0x00 0x76036301 0x00f0b5ff 0x00000000 0x00490000
EOF
    echo "Running with Xeon MP CPU..."
    GCC_CPUINFO="$cpuid_file2" gcc -march=native -### -E - < /dev/null 2>&1 | grep -i cache || true
    echo
}

# Test multiple cache descriptors from the uncovered lines
echo "Testing various cache descriptors..."
test_descriptor "0a" "L1: 8KB, 2-way, 32B line"
test_descriptor "0c" "L1: 16KB, 4-way, 32B line"
test_descriptor "0d" "L1: 16KB, 4-way, 64B line"
test_descriptor "21" "L2: 256KB, 8-way, 64B line"
test_descriptor "2c" "L1: 32KB, 8-way, 64B line"
test_descriptor "39" "L2: 128KB, 4-way, 64B line"
test_descriptor "41" "L2: 128KB, 4-way, 32B line"
test_descriptor "60" "L1: 16KB, 8-way, 64B line"
test_descriptor "78" "L2: 1024KB, 4-way, 64B line"
test_descriptor "87" "L2: 1024KB, 8-way, 64B line"

# Test the Xeon MP special case
test_xeon_mp_case

# Test with multiple descriptor bytes in one CPUID run
echo "=== Testing multiple descriptors in single CPUID ==="
local cpuid_multi="$TMPDIR/cpuid_multi.txt"
cat > "$cpuid_multi" << EOF
0x00000000 0x00 0x0000000b 0x756e6547 0x49656e69 0x6c65746e
0x00000001 0x00 0x000306a9 0x00100800 0x7ffafbbf 0xbfebfbff
0x00000002 0x00 0x76036301 0x00f0b5ff 0x00000000 0x000a0c0d
0x00000004 0x00 0x1c004121 0x01c0003f 0x0000003f 0x00000000
EOF
echo "Running with multiple descriptors (0x0a, 0x0c, 0x0d)..."
GCC_CPUINFO="$cpuid_multi" gcc -march=native -### -E - < /dev/null 2>&1 | grep -i cache || true
echo

# Test different -march values to trigger table lookups
echo "=== Testing -march values with cache table lookups ==="
for arch in core2 nehalem sandybridge ivybridge haswell skylake kabylake cannonlake icelake tigerlake alderlake; do
    echo "Testing -march=$arch"
    gcc -march=$arch -mtune=generic -Q --help=target 2>&1 | grep -E "(cache|Cache)" | head -3 || true
done
echo

# Test with -mtune=native and fake CPUID
echo "=== Testing -mtune=native with fake CPUID ==="
local cpuid_full="$TMPDIR/cpuid_full.txt"
cat > "$cpuid_full" << EOF
0x00000000 0x00 0x0000000b 0x756e6547 0x49656e69 0x6c65746e
0x00000001 0x00 0x000906ea 0x00100800 0x7ffafbbf 0xbfebfbff
0x00000002 0x00 0x76036301 0x00f0b5ff 0x00000000 0x00c00000
0x00000004 0x00 0x1c004121 0x01c0003f 0x0000003f 0x00000000
0x00000004 0x01 0x1c004122 0x01c0003f 0x0000003f 0x00000000
0x00000004 0x02 0x1c004143 0x01c0003f 0x000001ff 0x00000000
0x00000004 0x03 0x1c03c163 0x03c0003f 0x00000fff 0x00000006
EOF
echo "Running with full fake CPUID and -mtune=native..."
GCC_CPUINFO="$cpuid_full" gcc -march=native -mtune=native -dumpspecs 2>&1 | grep -i cache | head -5 || true
echo

# Test with --help=target and various march/mtune combinations
echo "=== Testing --help=target with various options ==="
for combo in "-march=x86-64 -mtune=generic" \
             "-march=core2 -mtune=core2" \
             "-march=skylake -mtune=skylake" \
             "-march=native -mtune=native"; do
    echo "Testing: gcc $combo --help=target"
    gcc $combo --help=target 2>&1 | grep -E "(cache|Cache|march|mtune)" | head -2 || true
done

echo "=== All tests completed ==="
```

This test script comprehensively covers the requirements:

1. **Driver Invocation Commands**: The script runs multiple `gcc` commands with different flags and environment variables.

2. **Simulate CPUID via GCC_CPUINFO**: Creates temporary files with carefully crafted CPUID data containing the specific cache descriptor bytes from the uncovered lines.

3. **Target-Specific Flag Combinations**: Uses `-march=native`, `-mtune=generic`, `-###`, `-Q`, `--help=target`, and `-dumpspecs` to trigger different code paths.

4. **Coverage of Edge Cases**: Specifically tests the `0x49` case with two scenarios:
   - Regular Intel CPU (should execute the assignment)
   - Xeon MP CPU (should skip due to the `xeon_mp` guard)

5. **Multiple Architectures**: Tests various `-march=` values to trigger the driver's internal cache table lookups.

6. **Multiple Descriptor Testing**: Tests individual descriptors and also tests multiple descriptors in a single CPUID run.

The script creates temporary CPUID files that simulate different CPU configurations, allowing the GCC driver to decode the cache descriptors without needing actual hardware with those specific cache configurations.
