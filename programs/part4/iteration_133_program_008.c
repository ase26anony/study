Here's a comprehensive POSIX shell script that systematically tests the uncovered command-line parsing logic in `gcov-dump`:

```bash
#!/bin/sh
# gcov-dump-coverage-test.sh
# Test script for exercising gcov-dump command-line parsing logic
# Specifically targets lines 111-130 in gcov-dump.cc

set -u  # Treat unset variables as errors

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Configuration
GCOV_DUMP_BIN="gcov-dump"
TEST_GCDA="test.gcda"
TEST_GCDA2="test2.gcdA"  # Different case to test case-insensitive matching
NONEXISTENT_FILE="nonexistent.gcda"
OUTPUT_DIR="gcov_dump_test_output"
LOG_FILE="$OUTPUT_DIR/test.log"

# Function to print status messages
print_status() {
    echo -e "${GREEN}[TEST]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to check if a command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to check if file exists
file_exists() {
    [ -f "$1" ]
}

# Function to run gcov-dump and capture exit status
run_gcov_dump() {
    local args="$1"
    local description="$2"
    local expect_error="${3:-0}"
    
    print_status "Running: $description"
    echo "=== Test: $description ===" >> "$LOG_FILE"
    echo "Command: $GCOV_DUMP_BIN $args" >> "$LOG_FILE"
    
    if [ "$expect_error" -eq 1 ]; then
        # Capture stderr for error cases
        $GCOV_DUMP_BIN $args 2>> "$LOG_FILE"
        local exit_code=$?
        echo "Exit code: $exit_code" >> "$LOG_FILE"
    else
        # Normal execution, redirect output to log
        $GCOV_DUMP_BIN $args >> "$LOG_FILE" 2>&1
        local exit_code=$?
        echo "Exit code: $exit_code" >> "$LOG_FILE"
    fi
    
    echo "" >> "$LOG_FILE"
    return $exit_code
}

# Function to verify output contains expected pattern
verify_output() {
    local pattern="$1"
    local description="$2"
    
    if tail -20 "$LOG_FILE" | grep -q "$pattern"; then
        print_status "✓ $description: Found '$pattern'"
        return 0
    else
        print_warning "✗ $description: Pattern '$pattern' not found"
        return 1
    fi
}

# Main test execution
main() {
    # Create output directory
    mkdir -p "$OUTPUT_DIR"
    > "$LOG_FILE"  # Clear log file
    
    print_status "Starting gcov-dump command-line parsing tests"
    print_status "Target: Lines 111-130 in gcov-dump.cc"
    echo "Test started at: $(date)" >> "$LOG_FILE"
    
    # 1. Setup and Validation
    print_status "=== Phase 1: Setup and Validation ==="
    
    # Check if gcov-dump exists
    if ! command_exists "$GCOV_DUMP_BIN"; then
        print_error "$GCOV_DUMP_BIN not found in PATH"
        print_error "Please compile gcov-dump with:"
        print_error "  g++ -O0 -fprofile-arcs -ftest-coverage -o gcov-dump gcov-dump.cc -liberty -lz"
        print_error "Or install it via your package manager"
        exit 1
    fi
    
    # Print version for traceability
    print_status "Testing gcov-dump version:"
    $GCOV_DUMP_BIN -v 2>&1 | head -5
    
    # Check for required .gcda files
    if ! file_exists "$TEST_GCDA"; then
        print_warning "$TEST_GCDA not found"
        print_warning "To create test .gcda files, run:"
        cat << 'EOF'
  # Create a simple test program
  cat > test.c << 'PROG_EOF'
  int main() {
      int x = 0;
      for (int i = 0; i < 10; i++) {
          x += i;
      }
      return x > 0 ? 0 : 1;
  }
  PROG_EOF
  
  # Compile with coverage
  gcc -fprofile-arcs -ftest-coverage -o test_prog test.c
  
  # Run to generate .gcda
  ./test_prog
  
  # Copy for multiple file tests
  cp test.gcda test2.gcda
EOF
        print_warning "Continuing with tests that don't require .gcda files..."
        HAS_GCDA=0
    else
        HAS_GCDA=1
        print_status "Found $TEST_GCDA"
        
        # Create second .gcda if needed
        if ! file_exists "$TEST_GCDA2"; then
            cp "$TEST_GCDA" "$TEST_GCDA2" 2>/dev/null || true
        fi
    fi
    
    # 2. Exercise Core Flags (individual)
    print_status "\n=== Phase 2: Individual Flag Tests ==="
    
    if [ "$HAS_GCDA" -eq 1 ]; then
        # Baseline - no flags
        run_gcov_dump "$TEST_GCDA" "Baseline (no flags)"
        
        # Individual flags
        run_gcov_dump "-l $TEST_GCDA" "Flag -l (dump contents)"
        run_gcov_dump "-p $TEST_GCDA" "Flag -p (dump positions)"
        run_gcov_dump "-r $TEST_GCDA" "Flag -r (dump raw)"
        run_gcov_dump "-s $TEST_GCDA" "Flag -s (dump stable)"
        
        # Verify -l produces more verbose output
        if grep -q "Tag" "$LOG_FILE"; then
            print_status "✓ -l flag produced tag information"
        fi
    else
        print_warning "Skipping individual flag tests (no .gcda file)"
    fi
    
    # 3. Exercise Flag Combinations
    print_status "\n=== Phase 3: Flag Combination Tests ==="
    
    if [ "$HAS_GCDA" -eq 1 ]; then
        # Various combinations
        run_gcov_dump "-lp $TEST_GCDA" "Flags -l -p combined"
        run_gcov_dump "-pr $TEST_GCDA" "Flags -p -r combined"
        run_gcov_dump "-lprs $TEST_GCDA" "Flags -l -p -r -s combined"
        run_gcov_dump "-srlp $TEST_GCDA" "Flags -s -r -l -p (different order)"
        
        # Test with space separation
        run_gcov_dump "-l -p -r $TEST_GCDA" "Flags -l -p -r separated"
    else
        print_warning "Skipping flag combination tests (no .gcda file)"
    fi
    
    # 4. Exercise Help and Version (always work)
    print_status "\n=== Phase 4: Help and Version Tests ==="
    
    # Help - should print usage and exit
    run_gcov_dump "-h" "Flag -h (help)"
    verify_output "Usage:" "Help output"
    
    # Version - should print version and exit
    run_gcov_dump "-v" "Flag -v (version)"
    verify_output "gcov-dump" "Version output"
    
    # 5. Trigger Error Cases
    print_status "\n=== Phase 5: Error Case Tests ==="
    
    # Unknown flag -x
    print_status "Testing unknown flag -x (expecting error)"
    $GCOV_DUMP_BIN -x 2>&1 | tee -a "$LOG_FILE" | grep -q "unknown flag"
    if [ $? -eq 0 ]; then
        print_status "✓ Unknown flag -x triggered error message"
    else
        print_warning "✗ Unknown flag -x did not produce expected error"
    fi
    
    # Unknown flag -z
    print_status "Testing unknown flag -z (expecting error)"
    $GCOV_DUMP_BIN -z 2>&1 | tee -a "$LOG_FILE" | grep -q "unknown flag"
    if [ $? -eq 0 ]; then
        print_status "✓ Unknown flag -z triggered error message"
    else
        print_warning "✗ Unknown flag -z did not produce expected error"
    fi
    
    # Combination with unknown flag -lx
    print_status "Testing -lx (known flag followed by unknown)"
    $GCOV_DUMP_BIN -lx 2>&1 | tee -a "$LOG_FILE" | grep -q "unknown flag"
    if [ $? -eq 0 ]; then
        print_status "✓ -lx combination triggered error for 'x'"
    else
        print_warning "✗ -lx did not produce expected error"
    fi
    
    # 6. Exercise Multi-file Input
    print_status "\n=== Phase 6: Multi-file Input Tests ==="
    
    if [ "$HAS_GCDA" -eq 1 ] && file_exists "$TEST_GCDA2"; then
        # Multiple files with no flags
        run_gcov_dump "$TEST_GCDA $TEST_GCDA2" "Two .gcda files"
        
        # Multiple files with flags
        run_gcov_dump "-l $TEST_GCDA $TEST_GCDA2" "Two files with -l flag"
        run_gcov_dump "-l -p $TEST_GCDA $TEST_GCDA2" "Two files with -l -p flags"
    else
        print_warning "Skipping multi-file tests (missing second .gcda)"
    fi
    
    # 7. Exercise Error Handling
    print_status "\n=== Phase 7: File Error Handling Tests ==="
    
    # Non-existent file
    print_status "Testing non-existent file: $NONEXISTENT_FILE"
    $GCOV_DUMP_BIN "$NONEXISTENT_FILE" 2>&1 | tee -a "$LOG_FILE" | grep -q -i "error\|cannot\|no such"
    if [ $? -eq 0 ]; then
        print_status "✓ Non-existent file triggered error"
    else
        print_warning "✗ Non-existent file may not have produced error"
    fi
    
    # Mix of existing and non-existent files
    if [ "$HAS_GCDA" -eq 1 ]; then
        print_status "Testing mix of existing and non-existent files"
        $GCOV_DUMP_BIN "$TEST_GCDA $NONEXISTENT_FILE" 2>&1 | tee -a "$LOG_FILE"
    fi
    
    # 8. Summary
    print_status "\n=== Phase 8: Test Summary ==="
    
    echo -e "\n${GREEN}Test execution complete.${NC}"
    echo "Log file: $LOG_FILE"
    
    # Count test invocations
    TEST_COUNT=$(grep -c "=== Test:" "$LOG_FILE")
    print_status "Total test invocations: $TEST_COUNT"
    
    # Check coverage hints
    echo -e "\n${YELLOW}Coverage Measurement Instructions:${NC}"
    cat << 'EOF'
To measure coverage of the command-line parsing logic:

1. Compile gcov-dump with instrumentation:
   For basic parsing coverage:
     g++ -O0 -fprofile-arcs -ftest-coverage -o gcov-dump gcov-dump.cc -liberty -lz
   
   For optimized parsing logic coverage:
     g++ -O2 -fprofile-arcs -ftest-coverage -o gcov-dump gcov-dump.cc -liberty -lz
   
   For aggressive optimization test:
     g++ -O3 -fprofile-arcs -ftest-coverage -o gcov-dump gcov-dump.cc -liberty -lz

2. Run this test script:
     ./gcov-dump-coverage-test.sh

3. Generate coverage report:
     gcov gcov-dump.cc

4. Check gcov-dump.cc.gcov file:
     - Lines 111-130 should be marked as executed (not #####)
     - Specifically check the switch cases for 'h', 'v', 'l', 'p', 'r', 's', and 'default'

5. For line-by-line verification:
     grep -n -A1 -B1 "^[ 0-9][0-9]*:|^#####:" gcov-dump.cc.gcov | grep -A1 -B1 "11[1-9]:\|12[0-9]:"
EOF
    
    # Cleanup
    if [ -f "$TEST_GCDA2" ] && [ "$TEST_GCDA2" != "$TEST_GCDA" ]; then
        rm -f "$TEST_GCDA2"
    fi
    
    return 0
}

# Run main function
main "$@"
```

This script provides:

1. **Comprehensive Flag Testing**:
   - Individual flags: `-l`, `-p`, `-r`, `-s`
   - Flag combinations: `-lp`, `-pr`, `-lprs`, `-srlp`
   - Help (`-h`) and version (`-v`) paths
   - Unknown flags: `-x`, `-z`, `-lx`

2. **File Handling Tests**:
   - Single `.gcda` file
   - Multiple `.gcda` files
   - Non-existent file error handling

3. **Robust Environment Checks**:
   - Validates `gcov-dump` binary existence
   - Checks for required `.gcda` files with helpful instructions
   - Captures all output to log file

4. **Coverage Integration**:
   - Clear instructions for compiling with different optimization levels
   - Structured test phases for systematic coverage
   - Output verification for flag effects

5. **Execution Flow**:
   - Follows the specified 8-phase sequence
   - Provides clear visual feedback with colors
   - Generates comprehensive log for debugging

To use this script:
1. Save it as `gcov-dump-coverage-test.sh`
2. Make it executable: `chmod +x gcov-dump-coverage-test.sh`
3. Generate test `.gcda` files (instructions in script)
4. Run: `./gcov-dump-coverage-test.sh`

The script will systematically exercise all paths through the switch statement at lines 111-130, ensuring complete coverage of the command-line parsing logic.
