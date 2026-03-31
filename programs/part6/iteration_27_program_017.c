Here's a comprehensive shell script that covers all the specified requirements for testing the uncovered lines in `gcov-tool.cc`:

```bash
#!/bin/bash
set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== Starting gcov-tool overlap flag coverage test ===${NC}"

# Create a temporary directory for test files
TEST_DIR=$(mktemp -d)
cd "$TEST_DIR"
echo "Working in temporary directory: $TEST_DIR"

# Create a simple C program with conditional branches
cat > test.c << 'EOF'
#include <stdio.h>
#include <stdlib.h>

int factorial(int n) {
    if (n <= 1) {
        return 1;
    } else {
        return n * factorial(n - 1);
    }
}

int fibonacci(int n) {
    if (n <= 1) {
        return n;
    } else {
        return fibonacci(n - 1) + fibonacci(n - 2);
    }
}

int main(int argc, char *argv[]) {
    int num = 5;
    if (argc > 1) {
        num = atoi(argv[1]);
    }
    
    printf("Factorial of %d: %d\n", num, factorial(num));
    
    if (num < 10) {
        printf("Fibonacci of %d: %d\n", num, fibonacci(num));
    } else {
        printf("Skipping Fibonacci for large number\n");
    }
    
    return 0;
}
EOF

# Function to compile with coverage
compile_with_coverage() {
    local suffix=$1
    local opt_level=$2
    echo "Compiling test program with -O${opt_level} (suffix: ${suffix})..."
    gcc -fprofile-arcs -ftest-coverage -O${opt_level} test.c -o "test_prog_${suffix}"
}

# Compile multiple versions with different optimization levels
compile_with_coverage "base" "0"
compile_with_coverage "opt" "2"

# Function to run program and generate .gcda files
generate_gcda() {
    local prog_name=$1
    local run_id=$2
    local arg=$3
    
    echo "Running ${prog_name} (run ${run_id}) with arg: ${arg:-default}"
    
    # Create unique directory for this run's .gcda files
    mkdir -p "run_${run_id}"
    cd "run_${run_id}"
    
    # Run the program
    if [ -n "$arg" ]; then
        "../${prog_name}" "$arg"
    else
        "../${prog_name}"
    fi
    
    # Rename .gcda files to avoid conflicts
    for gcda in *.gcda; do
        if [ -f "$gcda" ]; then
            mv "$gcda" "../${prog_name}_${run_id}.gcda"
        fi
    done
    
    cd ..
    rm -rf "run_${run_id}"
}

# Generate multiple .gcda files with different execution paths
echo -e "\n${GREEN}=== Generating profile data ===${NC}"

# Generate data for base version
generate_gcda "test_prog_base" "1" ""
generate_gcda "test_prog_base" "2" "3"
generate_gcda "test_prog_base" "3" "7"

# Generate data for optimized version
generate_gcda "test_prog_opt" "1" ""
generate_gcda "test_prog_opt" "2" "4"

# Copy .gcno files to have consistent naming
cp test_prog_base.gcno test.gcno 2>/dev/null || true

# List generated files
echo -e "\nGenerated files:"
ls -la *.gcda *.gcno 2>/dev/null || true

# Function to test gcov-tool with specific flags
test_overlap_flags() {
    local test_name=$1
    shift
    local args=$@
    
    echo -e "\n${GREEN}=== Testing: ${test_name} ===${NC}"
    echo "Command: gcov-tool overlap $args"
    
    # Run gcov-tool and capture output
    if gcov-tool overlap $args 2>&1; then
        echo "✓ Success"
    else
        echo "✗ Failed (exit code: $?)"
    fi
}

# Test individual flags
echo -e "\n${GREEN}=== Testing individual flags ===${NC}"

test_overlap_flags "Verbose flag (-v)" -v test_prog_base_1.gcda
test_overlap_flags "Function level flag (-f)" -f test_prog_base_1.gcda test_prog_base_2.gcda
test_overlap_flags "Fullname flag (-F)" -F test_prog_base_1.gcda
test_overlap_flags "Object level flag (-o)" -o test_prog_base_1.gcda
test_overlap_flags "Hot only flag (-h)" -h test_prog_base_1.gcda test_prog_base_2.gcda
test_overlap_flags "Threshold flag (-t 0.5)" -t 0.5 test_prog_base_1.gcda
test_overlap_flags "Threshold flag (-t 1.0)" -t 1.0 test_prog_base_1.gcda
test_overlap_flags "Threshold flag (-t 10.5)" -t 10.5 test_prog_base_1.gcda

# Test flag combinations
echo -e "\n${GREEN}=== Testing flag combinations ===${NC}"

test_overlap_flags "Combination: -f -o" -f -o test_prog_base_1.gcda test_prog_base_2.gcda
test_overlap_flags "Combination: -F -h -t 1.0" -F -h -t 1.0 test_prog_base_1.gcda test_prog_base_2.gcda
test_overlap_flags "Combination: -v -f -F -o -h -t 5.0" -v -f -F -o -h -t 5.0 test_prog_base_1.gcda test_prog_base_2.gcda test_prog_base_3.gcda
test_overlap_flags "Combination: -f -t 0.8 -v" -f -t 0.8 -v test_prog_base_1.gcda test_prog_base_2.gcda

# Test with multiple input files (different optimization levels)
echo -e "\n${GREEN}=== Testing with multiple input files ===${NC}"

test_overlap_flags "Multiple files (3)" -v test_prog_base_1.gcda test_prog_base_2.gcda test_prog_base_3.gcda
test_overlap_flags "Multiple files with -f -o" -f -o test_prog_base_1.gcda test_prog_base_2.gcda test_prog_opt_1.gcda test_prog_opt_2.gcda

# Test invalid flag to trigger usage
echo -e "\n${GREEN}=== Testing invalid flag (to trigger usage) ===${NC}"
echo "Command: gcov-tool overlap -Z"
if gcov-tool overlap -Z 2>&1; then
    echo "✗ Unexpected success"
else
    echo "✓ Correctly triggered usage/error (exit code: $?)"
fi

# Test with no input files (should also trigger usage or error)
echo -e "\n${GREEN}=== Testing with no input files ===${NC}"
echo "Command: gcov-tool overlap -v"
if gcov-tool overlap -v 2>&1; then
    echo "✗ Unexpected success"
else
    echo "✓ Correctly triggered error (exit code: $?)"
fi

# Additional edge cases
echo -e "\n${GREEN}=== Testing edge cases ===${NC}"

# Test with threshold 0.0
test_overlap_flags "Threshold 0.0" -t 0.0 test_prog_base_1.gcda

# Test with threshold 100.0
test_overlap_flags "Threshold 100.0" -t 100.0 test_prog_base_1.gcda

# Test combination with just threshold and verbose
test_overlap_flags "-t 0.3 -v" -t 0.3 -v test_prog_base_1.gcda test_prog_base_2.gcda

# Test with all boolean flags together
test_overlap_flags "All boolean flags" -f -F -o -h -v test_prog_base_1.gcda test_prog_base_2.gcda

# Cleanup
echo -e "\n${GREEN}=== Test complete ===${NC}"
echo "Temporary directory: $TEST_DIR"
echo "You can manually remove it with: rm -rf $TEST_DIR"

# Optional: keep files for inspection
read -p "Remove temporary directory? (y/n): " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    cd ..
    rm -rf "$TEST_DIR"
    echo "Directory removed."
else
    echo "Directory kept at: $TEST_DIR"
fi
```

This script provides comprehensive coverage of the uncovered lines by:

1. **Generating valid GCOV data**: Creates a C program with conditional branches, compiles it with coverage instrumentation (`-fprofile-arcs -ftest-coverage`), and runs it multiple times with different arguments to generate distinct `.gcda` files.

2. **Testing all individual flags**: Tests `-v`, `-f`, `-F`, `-o`, `-h`, and `-t` with various threshold values (0.5, 1.0, 10.5).

3. **Testing flag combinations**: Tests various combinations like `-f -o`, `-F -h -t 1.0`, and `-v -f -F -o -h -t 5.0`.

4. **Triggering the default case**: Includes a test with invalid flag `-Z` to trigger `overlap_usage()`.

5. **Using multiple input files**: Tests with 2-4 different `.gcda` files, including files from differently optimized builds.

6. **Handling verbose output**: The `-v` flag is tested both alone and in combination with other flags.

7. **Edge cases**: Tests threshold values at boundaries (0.0, 100.0) and tests with no input files.

The script creates a self-contained test environment in a temporary directory and cleans up after itself (with user confirmation). Each test case clearly shows what's being tested and captures the output for verification.
