```bash
#!/bin/bash
set -e

# Configuration
GCOV_TOOL="./gcov-tool"
TEST_DIR="./test_gcda_files"
RESULTS_DIR="./gcov_tool_results"
mkdir -p "$RESULTS_DIR"

# Create a simple test program to generate .gcda files if needed
create_test_gcda() {
    echo "Creating test .gcda files..."
    mkdir -p "$TEST_DIR"
    
    # Create a minimal C program
    cat > "$TEST_DIR/test.c" << 'EOF'
#include <stdio.h>
void func1() { printf("func1\n"); }
void func2() { printf("func2\n"); }
int main() { func1(); func2(); return 0; }
EOF
    
    # Try to compile with coverage and run (if gcc is available)
    if command -v gcc &> /dev/null; then
        cd "$TEST_DIR"
        gcc -fprofile-arcs -ftest-coverage -O0 test.c -o test_prog
        ./test_prog > /dev/null 2>&1
        cd - > /dev/null
        echo "Generated .gcda files in $TEST_DIR"
    else
        # Create dummy .gcda files if gcc not available
        echo "gcc not found, creating dummy .gcda files"
        for i in {1..3}; do
            echo "dummy data" > "$TEST_DIR/test$i.gcda"
        done
    fi
}

# Run a gcov-tool command and check exit code
run_test() {
    local test_name="$1"
    local expected_exit="$2"
    shift 2
    
    echo "Running test: $test_name"
    echo "Command: $GCOV_TOOL $*"
    
    # Capture output and exit code
    if $GCOV_TOOL "$@" > "$RESULTS_DIR/${test_name}.out" 2> "$RESULTS_DIR/${test_name}.err"; then
        local exit_code=0
    else
        local exit_code=$?
    fi
    
    # Check if exit code matches expectation
    if [ "$exit_code" -eq "$expected_exit" ]; then
        echo "  ✓ PASSED (exit code: $exit_code)"
        return 0
    else
        echo "  ✗ FAILED (expected: $expected_exit, got: $exit_code)"
        echo "    Stderr:"
        cat "$RESULTS_DIR/${test_name}.err"
        return 1
    fi
}

# Initialize
create_test_gcda
echo "========================================="
echo "Testing gcov-tool overlap command options"
echo "========================================="

# Test 1: Help and version flags (overall infrastructure)
echo ""
echo "=== Basic command tests ==="
run_test "help" 0 --help
run_test "version" 0 --version

# Test 2: Individual short options
echo ""
echo "=== Individual short option tests ==="
run_test "verbose" 0 overlap -v "$TEST_DIR"
run_test "func_level" 0 overlap -f "$TEST_DIR"
run_test "fullname" 0 overlap -F "$TEST_DIR"
run_test "obj_level" 0 overlap -o "$TEST_DIR"
run_test "hot_only" 0 overlap -h "$TEST_DIR"
run_test "threshold_normal" 0 overlap -t 0.5 "$TEST_DIR"

# Test 3: -t with various threshold values
echo ""
echo "=== Threshold edge cases ==="
run_test "threshold_small" 0 overlap -t 0.001 "$TEST_DIR"
run_test "threshold_large" 0 overlap -t 100.0 "$TEST_DIR"
run_test "threshold_zero" 0 overlap -t 0 "$TEST_DIR"

# Test 4: Option combinations (different orders)
echo ""
echo "=== Option combination tests ==="
run_test "combo1" 0 overlap -v -f -o "$TEST_DIR"
run_test "combo2" 0 overlap -f -F -h "$TEST_DIR"
run_test "combo3" 0 overlap -F -o -t 0.3 "$TEST_DIR"
run_test "combo_reverse" 0 overlap -t 0.7 -h -o -F -f -v "$TEST_DIR"

# Test 5: Invalid option (to trigger default case)
echo ""
echo "=== Invalid option test ==="
run_test "invalid_option" 1 overlap -x "$TEST_DIR"

# Test 6: Complex combination of all covered options
echo ""
echo "=== Final comprehensive test ==="
run_test "comprehensive" 0 overlap -v -f -F -o -h -t 0.75 "$TEST_DIR"

# Test 7: Multiple .gcda files as arguments
echo ""
echo "=== Multiple file arguments ==="
if [ -f "$TEST_DIR/test.gcda" ]; then
    run_test "multiple_files" 0 overlap -v "$TEST_DIR/test.gcda" "$TEST_DIR/test.c.gcda"
else
    # Use dummy files if real ones don't exist
    run_test "multiple_files" 0 overlap -v "$TEST_DIR"/*.gcda 2>/dev/null || true
fi

# Summary
echo ""
echo "========================================="
echo "Test Summary"
echo "========================================="
echo "All tests completed. Outputs saved in: $RESULTS_DIR"
echo ""
echo "To collect coverage data for gcov-tool itself:"
echo "1. Ensure gcov-tool was built with:"
echo "   CFLAGS=\"-O0 -g -fprofile-arcs -ftest-coverage\""
echo "2. Run this script to generate .gcda files for gcov-tool"
echo "3. Use gcov on gcov-tool.cc to see coverage results"
echo ""
echo "Example coverage report command:"
echo "   gcov gcov-tool.cc"
```
