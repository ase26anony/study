// test.c - A simple valid C program
int main(void) {
    return 0;
}

// error.c - A C program with a syntax error to trigger failure
int main(void) {
    // Missing semicolon to cause compilation error
    return 0
}

// custom.specs - A minimal custom spec file
*asm:
%{!m16:%{!m32:%{!mx32:%{!m64:-m64}}}}

// run_test.sh - Shell script to execute the coverage tests
#!/bin/bash

# Create necessary directories
mkdir -p ./dump /tmp/sysroot

echo "=== GCC Driver Cleanup Coverage Test ==="
echo "Testing the reset of global variables in driver::finalize"
echo ""

# Test 1: Set dumpdir, dumpbase, and save-temps flags
echo "Test 1: Setting dump and temp options"
gcc -O0 -save-temps -dumpdir ./dump -dumpbase mytest -dumpbase-ext .c -specs=custom.specs --sysroot=/tmp/sysroot test.c -o test_program 2>/dev/null
echo "Exit code: $?"
echo ""

# Test 2: Trigger help and version states
echo "Test 2: Triggering help and version states"
gcc --help=common --version -wrapper /bin/echo -ftime-report test.c 2>&1 > /dev/null
echo "Exit code: $?"
echo ""

# Test 3: Use @file to set at_file_supplied
echo "Test 3: Using @file for arguments"
echo "-O0 -save-temps" > args.txt
gcc @args.txt test.c -o test_program2 2>/dev/null
echo "Exit code: $?"
echo ""

# Test 4: Use -fuse-ld to set use_ld
echo "Test 4: Setting use_ld with -fuse-ld"
gcc -O0 -fuse-ld=gold test.c -o test_program3 2>/dev/null
echo "Exit code: $?"
echo ""

# Test 5: Use -time for timing report
echo "Test 5: Using -time for timing"
gcc -O0 -time test.c -o test_program4 2>/dev/null
echo "Exit code: $?"
echo ""

# Test 6: Test with -isysroot
echo "Test 6: Using -isysroot"
gcc -O0 -isysroot /tmp/sysroot test.c -o test_program5 2>/dev/null
echo "Exit code: $?"
echo ""

# Test 7: Multiple source files (batch compilation)
echo "Test 7: Batch compilation with multiple files"
echo "int foo(void) { return 1; }" > foo.c
gcc -O0 -save-temps -dumpdir ./dump -c test.c foo.c 2>/dev/null
echo "Exit code: $?"
echo ""

# Test 8: Different compilation stages
echo "Test 8: Different compilation stages"
gcc -E test.c -o test.i 2>/dev/null
echo "Preprocess exit code: $?"
gcc -S test.i -o test.s 2>/dev/null
echo "Compile exit code: $?"
gcc -c test.s -o test.o 2>/dev/null
echo "Assemble exit code: $?"
echo ""

# Test 9: Final failure test - This should trigger the cleanup block
echo "Test 9: FAILURE TEST - Should trigger cleanup with greatest_status = 1"
echo "This command combines multiple flags and should fail due to invalid option"
gcc -O0 -save-temps -dumpdir ./dump -dumpbase fail -dumpbase-ext .c \
    --sysroot=/tmp/sysroot -specs=custom.specs \
    -fuse-ld=gold -time -invalid-option error.c 2>&1 > /dev/null
echo "Exit code: $? (expected non-zero)"
echo ""

# Test 10: Another failure with syntax error
echo "Test 10: Syntax error failure"
gcc -O0 -save-temps -dumpdir ./dump -dumpbase syntaxfail error.c 2>&1 > /dev/null
echo "Exit code: $? (expected non-zero)"
echo ""

# Test 11: Test with print-subprocess-help
echo "Test 11: Testing print-subprocess-help"
gcc --help=target test.c 2>&1 > /dev/null
echo "Exit code: $?"
echo ""

# Test 12: Test verbose flag
echo "Test 12: Testing verbose flag"
gcc -v test.c -o test_program6 2>/dev/null
echo "Exit code: $?"
echo ""

# Test 13: Test with -pipe flag
echo "Test 13: Testing with -pipe flag"
gcc -O0 -pipe -save-temps test.c -o test_program7 2>/dev/null
echo "Exit code: $?"
echo ""

# Test 14: Test with -### (print commands)
echo "Test 14: Testing with -### (dry run)"
gcc -O0 -save-temps -dumpdir ./dump -### test.c 2>&1 > /dev/null
echo "Exit code: $?"
echo ""

echo "=== All tests completed ==="
echo "The cleanup block in driver::finalize should have been executed"
echo "multiple times, covering all the reset operations."

# Cleanup
rm -f test_program* test.i test.s test.o foo.c foo.o args.txt
rm -rf ./dump
