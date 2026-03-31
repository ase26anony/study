```bash
#!/bin/bash

# Script to exercise gcov-tool overlap subcommand argument parsing
# Targets uncovered lines in gcov-tool.cc (lines 534-554)

set -euo pipefail

# Configuration
GCOV_TOOL="${GCOV_TOOL:-./gcov-tool}"
TEST_DIR="${TEST_DIR:-./test_gcda_files}"
SUMMARY_FILE="${SUMMARY_FILE:-test_summary.log}"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Initialize counters
total_tests=0
passed_tests=0
failed_tests=0

# Function to run a test and check result
run_test() {
    local description="$1"
    local expected_exit="${2:-0}"
    local command="$3"
    
    ((total_tests++))
    
    echo -e "\n${YELLOW}Test $total_tests: $description${NC}"
    echo "Command: $command"
    
    # Execute the command
    eval "$command" > /dev/null 2>&1
    local exit_code=$?
    
    if [[ $exit_code -eq $expected_exit ]]; then
        echo -e "${GREEN}✓ PASSED${NC}"
        ((passed_tests++))
        return 0
    else
        echo -e "${RED}✗ FAILED (exit code: $exit_code, expected: $expected_exit)${NC}"
        ((failed_tests++))
        return 1
    fi
}

# Function to create dummy .gcda directory structure
setup_test_files() {
    echo "Setting up test directory structure..."
    
    # Create test directory
    mkdir -p "$TEST_DIR"
    mkdir -p "$TEST_DIR/subdir"
    
    # Create dummy .gcda files (empty files if real ones don't exist)
    touch "$TEST_DIR/test1.gcda"
    touch "$TEST_DIR/test2.gcda"
    touch "$TEST_DIR/subdir/test3.gcda"
    
    # Also create a dummy directory without .gcda files
    mkdir -p "$TEST_DIR/empty_dir"
    
    echo "Test directory created at: $TEST_DIR"
}

# Function to test help and version flags
test_help_version() {
    echo -e "\n${YELLOW}=== Testing help and version flags ===${NC}"
    
    run_test "Show help" 0 "$GCOV_TOOL --help"
    run_test "Show version" 0 "$GCOV_TOOL --version"
    run_test "Show overlap help" 0 "$GCOV_TOOL overlap --help"
}

# Function to test individual short options
test_individual_options() {
    echo -e "\n${YELLOW}=== Testing individual short options ===${NC}"
    
    # Test -v (verbose)
    run_test "Test -v option" 0 "$GCOV_TOOL overlap -v $TEST_DIR"
    
    # Test -f (function level)
    run_test "Test -f option" 0 "$GCOV_TOOL overlap -f $TEST_DIR"
    
    # Test -F (fullname)
    run_test "Test -F option" 0 "$GCOV_TOOL overlap -F $TEST_DIR"
    
    # Test -o (object level)
    run_test "Test -o option" 0 "$GCOV_TOOL overlap -o $TEST_DIR"
    
    # Test -h (hot only)
    run_test "Test -h option" 0 "$GCOV_TOOL overlap -h $TEST_DIR"
    
    # Test -t with valid threshold
    run_test "Test -t 0.5 option" 0 "$GCOV_TOOL overlap -t 0.5 $TEST_DIR"
}

# Function to test -t edge cases
test_t_edge_cases() {
    echo -e "\n${YELLOW}=== Testing -t edge cases ===${NC}"
    
    # Very small number
    run_test "Test -t 0.001" 0 "$GCOV_TOOL overlap -t 0.001 $TEST_DIR"
    
    # Large number
    run_test "Test -t 100.0" 0 "$GCOV_TOOL overlap -t 100.0 $TEST_DIR"
    
    # Zero
    run_test "Test -t 0" 0 "$GCOV_TOOL overlap -t 0 $TEST_DIR"
    
    # Negative number (should still parse, though may not make semantic sense)
    run_test "Test -t -1.0" 0 "$GCOV_TOOL overlap -t -1.0 $TEST_DIR"
}

# Function to test option combinations
test_option_combinations() {
    echo -e "\n${YELLOW}=== Testing option combinations ===${NC}"
    
    # Test two options
    run_test "Test -v -f combination" 0 "$GCOV_TOOL overlap -v -f $TEST_DIR"
    run_test "Test -f -F combination" 0 "$GCOV_TOOL overlap -f -F $TEST_DIR"
    
    # Test three options
    run_test "Test -v -f -o combination" 0 "$GCOV_TOOL overlap -v -f -o $TEST_DIR"
    run_test "Test -F -o -h combination" 0 "$GCOV_TOOL overlap -F -o -h $TEST_DIR"
    
    # Test with -t and other options
    run_test "Test -v -t 0.3 -f combination" 0 "$GCOV_TOOL overlap -v -t 0.3 -f $TEST_DIR"
    run_test "Test -f -F -t 0.8 -h combination" 0 "$GCOV_TOOL overlap -f -F -t 0.8 -h $TEST_DIR"
    
    # Test different order
    run_test "Test options in different order (-f -v -o)" 0 "$GCOV_TOOL overlap -f -v -o $TEST_DIR"
    run_test "Test options in different order (-t 0.6 -F -h)" 0 "$GCOV_TOOL overlap -t 0.6 -F -h $TEST_DIR"
}

# Function to test invalid option
test_invalid_option() {
    echo -e "\n${YELLOW}=== Testing invalid option ===${NC}"
    
    # Test invalid short option (should trigger default case and call overlap_usage)
    # Note: overlap_usage() typically exits with non-zero, but exact code may vary
    # We'll accept any non-zero exit code
    run_test "Test invalid option -x" "!" "$GCOV_TOOL overlap -x $TEST_DIR"
    
    # Test invalid argument to -t (non-numeric)
    run_test "Test -t with non-numeric argument" "!" "$GCOV_TOOL overlap -t invalid $TEST_DIR"
}

# Function to test complex combination (all options)
test_complex_combination() {
    echo -e "\n${YELLOW}=== Testing complex combination (all options) ===${NC}"
    
    # Test all covered options together
    run_test "Test all options combined" 0 \
        "$GCOV_TOOL overlap -v -f -F -o -h -t 0.75 $TEST_DIR"
    
    # Test with multiple positional arguments
    run_test "Test with multiple gcda directories" 0 \
        "$GCOV_TOOL overlap -v -f -t 0.5 $TEST_DIR $TEST_DIR/subdir $TEST_DIR/empty_dir"
}

# Function to test with actual .gcda files if available
test_with_real_gcda() {
    echo -e "\n${YELLOW}=== Testing with actual .gcda files (if available) ===${NC}"
    
    # Look for actual .gcda files in common locations
    local gcda_dirs=()
    
    # Check current directory
    if compgen -G "*.gcda" > /dev/null; then
        gcda_dirs+=(".")
    fi
    
    # Check common build directories
    if [[ -d "obj" ]] && compgen -G "obj/*.gcda" > /dev/null; then
        gcda_dirs+=("obj")
    fi
    
    if [[ -d "build" ]] && compgen -G "build/*.gcda" > /dev/null; then
        gcda_dirs+=("build")
    fi
    
    if [[ ${#gcda_dirs[@]} -gt 0 ]]; then
        for dir in "${gcda_dirs[@]}"; do
            run_test "Test with real .gcda files in $dir" 0 \
                "$GCOV_TOOL overlap -v -f $dir"
            break  # Just test with first found directory
        done
    else
        echo "No actual .gcda files found, using dummy files only"
    fi
}

# Main execution
main() {
    echo "=== gcov-tool overlap argument parsing tests ==="
    echo "Using gcov-tool: $GCOV_TOOL"
    
    # Check if gcov-tool exists
    if [[ ! -x "$GCOV_TOOL" ]]; then
        echo -e "${RED}Error: gcov-tool not found or not executable at $GCOV_TOOL${NC}"
        echo "Please set GCOV_TOOL environment variable to the correct path"
        exit 1
    fi
    
    # Setup test files
    setup_test_files
    
    # Run test suites
    test_help_version
    test_individual_options
    test_t_edge_cases
    test_option_combinations
    test_invalid_option
    test_with_real_gcda
    test_complex_combination
    
    # Print summary
    echo -e "\n${YELLOW}=== Test Summary ===${NC}"
    echo "Total tests: $total_tests"
    echo -e "${GREEN}Passed: $passed_tests${NC}"
    
    if [[ $failed_tests -gt 0 ]]; then
        echo -e "${RED}Failed: $failed_tests${NC}"
    else
        echo -e "${GREEN}Failed: $failed_tests${NC}"
    fi
    
    # Save summary to file
    {
        echo "Test executed: $(date)"
        echo "gcov-tool path: $GCOV_TOOL"
        echo "Total tests: $total_tests"
        echo "Passed: $passed_tests"
        echo "Failed: $failed_tests"
    } > "$SUMMARY_FILE"
    
    echo "Detailed summary saved to: $SUMMARY_FILE"
    
    # Exit with appropriate code
    if [[ $failed_tests -gt 0 ]]; then
        exit 1
    else
        exit 0
    fi
}

# Run main function
main "$@"
```

This shell script provides comprehensive testing of the uncovered argument parsing logic in `gcov-tool.cc`:

1. **Individual option testing**: Each short option (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) is tested separately
2. **Edge cases for `-t`**: Tests with 0.001, 100.0, 0, and -1.0
3. **Option combinations**: Tests various combinations and orders of options
4. **Invalid option**: Tests `-x` to trigger the `default:` case
5. **Complex combination**: Tests all options together as requested
6. **Help and version flags**: Tests `--help` and `--version`
7. **Real .gcda files**: Attempts to use actual .gcda files if available

The script creates a dummy directory structure with empty .gcda files to ensure the argument parsing logic is reached even if no real coverage data exists. It provides clear output with color coding and a summary of test results.
