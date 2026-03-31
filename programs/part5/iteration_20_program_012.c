I'll create a comprehensive test that targets the uncovered state reset block in gcc.cc. This test uses various GCC driver invocations to set and reset the internal state variables.

```bash
#!/bin/bash
# Test script to exercise GCC driver state initialization and cleanup
# Targets the reset block in gcc.cc lines 11228-11250

set -e

# Create a temporary directory for test files
TESTDIR=$(mktemp -d /tmp/gcc_test_XXXXXX)
trap "rm -rf $TESTDIR" EXIT

# Create a minimal valid C source file
cat > "$TESTDIR/test.c" << 'EOF'
int main(void) {
    return 0;
}
EOF

# Create a minimal C++ source file
cat > "$TESTDIR/test.cpp" << 'EOF'
int main() {
    return 0;
}
EOF

# Create an at-file with compiler arguments
cat > "$TESTDIR/args.txt" << 'EOF'
-O2
-Wall
-save-temps
-dumpdir="$TESTDIR/dumps"
-dumpbase="mytest"
-o "$TESTDIR/output"
EOF

# Create another at-file with different options
cat > "$TESTDIR/args2.txt" << 'EOF'
--help
--version
--target-help
EOF

# Create an invalid C file for error testing
cat > "$TESTDIR/invalid.c" << 'EOF'
int main( {
    return
}
EOF

# Function to run GCC with output suppressed
run_gcc() {
    "$@" >/dev/null 2>&1 || true
}

echo "Testing GCC driver state initialization and cleanup..."
echo "======================================================"

# Test 1: Basic compilation with state-setting flags
# This should allocate dumpdir, dumpbase, outbase
echo "Test 1: Basic compilation with state flags"
run_gcc gcc -save-temps -dumpdir "$TESTDIR/mydumps" -dumpbase myfile \
    -o "$TESTDIR/output1" "$TESTDIR/test.c" -c

# Test 2: Preprocessor run with dump options
# Uses -E to run preprocessor only, still triggers cleanup
echo "Test 2: Preprocessor with dump options"
run_gcc gcc -E -P -dD -save-temps=cwd -dumpdir "./" \
    -dumpbase "pptest" -o "$TESTDIR/preprocessed.i" "$TESTDIR/test.c"

# Test 3: Help and version flags (early exit paths)
echo "Test 3: Help and version flags"
run_gcc gcc --help
run_gcc g++ --version
run_gcc gcc --target-help

# Test 4: Compilation after help flags (tests state reset)
echo "Test 4: Compilation after help flags"
run_gcc gcc -c "$TESTDIR/test.c" -o "$TESTDIR/test.o"

# Test 5: Using at-files (triggers at_file_supplied)
echo "Test 5: Using at-files"
run_gcc gcc @"$TESTDIR/args.txt" "$TESTDIR/test.c"

# Test 6: C++ driver with different options
echo "Test 6: C++ driver tests"
run_gcc g++ -save-temps -dumpbase "cpptest" -x c++ "$TESTDIR/test.c" -E
run_gcc g++ -c "$TESTDIR/test.cpp" -o "$TESTDIR/test_cpp.o"

# Test 7: Error case (should affect greatest_status)
echo "Test 7: Error cases"
run_gcc gcc -c "$TESTDIR/invalid.c" -o "$TESTDIR/invalid.o" 2>/dev/null
run_gcc gcc -minvalid-option "$TESTDIR/test.c" 2>/dev/null

# Test 8: Multiple source types
echo "Test 8: Multiple source types"
run_gcc gcc -S "$TESTDIR/test.c" -o "$TESTDIR/test.s"
run_gcc gcc -c "$TESTDIR/test.s" -o "$TESTDIR/test_asm.o"

# Test 9: Assembly preprocessing
echo "Test 9: Assembly preprocessing"
run_gcc gcc -E "$TESTDIR/test.s" -o "$TESTDIR/test_preproc.s"

# Test 10: Complex combined flags
echo "Test 10: Complex flag combinations"
run_gcc gcc -v -save-temps=obj -dumpdir "$TESTDIR" \
    -dumpbase "complex" -dumpbase-ext ".dump" \
    -specs=/dev/null -B "$TESTDIR" \
    -c "$TESTDIR/test.c" -o "$TESTDIR/complex.o"

# Test 11: Reset with verbose flag
echo "Test 11: Verbose flag tests"
run_gcc gcc -v -c "$TESTDIR/test.c" 2>/dev/null
run_gcc gcc -### -E "$TESTDIR/test.c" 2>/dev/null

# Test 12: Multiple outputs and dumpdir variations
echo "Test 12: Multiple output variations"
run_gcc gcc -save-temps -dumpdir "$TESTDIR/dumps/" \
    -o "$TESTDIR/final.exe" "$TESTDIR/test.c" "$TESTDIR/test_cpp.o"
run_gcc gcc -dumpdir "" -dumpbase "" -c "$TESTDIR/test.c"

# Test 13: Using all at-file flags
echo "Test 13: At-file with all flags"
run_gcc gcc @"$TESTDIR/args2.txt"

# Test 14: Mixed successful and failed compilations
echo "Test 14: Mixed success/error cases"
run_gcc gcc -c "$TESTDIR/test.c" -o "$TESTDIR/good.o"
run_gcc gcc -invalid-flag "$TESTDIR/test.c" 2>/dev/null
run_gcc gcc -c "$TESTDIR/test.c" -o "$TESTDIR/good2.o"

# Test 15: Driver-specific time reporting
echo "Test 15: Time reporting flags"
run_gcc gcc -ftime-report -c "$TESTDIR/test.c" 2>/dev/null
run_gcc g++ -ftime-report -c "$TESTDIR/test.cpp" 2>/dev/null

# Test 16: Clean compilation to trigger final cleanup
echo "Test 16: Final clean compilations"
run_gcc gcc -O1 -Wall -Wextra -c "$TESTDIR/test.c" -o "$TESTDIR/clean.o"
run_gcc g++ -std=c++11 -c "$TESTDIR/test.cpp" -o "$TESTDIR/clean_cpp.o"

echo "======================================================"
echo "Test sequence completed. Check coverage of:"
echo "  - is_cpp_driver, at_file_supplied"
echo "  - print_help_list, print_version, verbose_only_flag"
echo "  - save_temps_flag, dumpdir, dumpbase, outbase"
echo "  - spec_machine, greatest_status"
echo "  - All pointer resets (free() calls)"
echo ""
echo "Temporary files in: $TESTDIR"
```

This test script systematically exercises the GCC driver through various code paths:

1. **State Variable Initialization**: Uses flags like `-save-temps`, `-dumpdir`, `-dumpbase` that allocate memory for the state variables.

2. **Cleanup Triggering**: Each invocation causes the driver to parse arguments, potentially process files, and execute cleanup where the reset block should be hit.

3. **Multiple Driver Types**: Uses both `gcc` and `g++` to potentially affect `is_cpp_driver`.

4. **At-File Processing**: Uses `@file` syntax to trigger `at_file_supplied`.

5. **Help/Version Flags**: Uses `--help`, `--version`, `--target-help` to set the corresponding flags.

6. **Error Cases**: Includes invalid syntax and unsupported options to affect `greatest_status`.

7. **Different Processing Modes**: Uses `-c` (compile), `-E` (preprocess), `-S` (assemble) to exercise different paths.

8. **Verbose Flags**: Uses `-v` and `-###` for `verbose_only_flag`.

9. **Mixed Success/Failure**: Sequences of successful and failed compilations to test state reset between invocations.

The script runs each test with output suppressed to avoid clutter, but all invocations should pass through the driver's initialization and cleanup code. The temporary directory is cleaned up automatically on exit.
