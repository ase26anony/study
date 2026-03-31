// test.c - A simple valid C program
int main(void) {
    return 0;
}

// error.c - A C program with a syntax error to trigger failure
int main(void) {
    // Missing semicolon to cause compilation error
    return 0
}

// custom.specs - A minimal spec file to influence spec_machine
*asm:
%{m32:--32} %{m64:--64}

*link:
%{m32:-m elf_i386} %{m64:-m elf_x86_64}

// run_test.sh - Shell script to execute the test sequence
#!/bin/bash

# Create necessary directories
mkdir -p ./dump /tmp/sysroot/include /tmp/sysroot/lib

# Test 1: Set dump and temp options with successful compilation
echo "Test 1: Setting dump/temp options with successful compilation"
gcc -O0 -save-temps -dumpdir ./dump -dumpbase mytest -dumpbase-ext .c \
    -specs=custom.specs --sysroot=/tmp/sysroot test.c -o test_program 2>&1
echo "Exit status: $?"

# Clean up from test 1
rm -f test_program test.i test.s test.o

# Test 2: Trigger help and version states
echo -e "\nTest 2: Triggering help and version states"
gcc --help=common --version -wrapper /bin/echo -ftime-report test.c 2>&1 > /dev/null
echo "Exit status: $?"

# Test 3: Use @file to supply arguments (sets at_file_supplied)
echo -e "\nTest 3: Using @file for arguments"
echo "-O0 -save-temps -dumpdir ./dump2 -dumpbase filetest" > args.txt
gcc @args.txt test.c -o test_program2 2>&1
echo "Exit status: $?"
rm -f args.txt test_program2 test.i test.s test.o

# Test 4: Test with -fuse-ld to set use_ld
echo -e "\nTest 4: Testing with -fuse-ld"
gcc -O0 -fuse-ld=bfd -save-temps test.c -o test_program3 2>&1
echo "Exit status: $?"
rm -f test_program3 test.i test.s test.o

# Test 5: Test time reporting to file
echo -e "\nTest 5: Testing time reporting"
gcc -O0 -time -save-temps test.c -o test_program4 2>&1
echo "Exit status: $?"
rm -f test_program4 test.i test.s test.o

# Test 6: Multiple source files compilation
echo -e "\nTest 6: Multiple source files (batch compilation)"
echo "int helper(void) { return 42; }" > helper.c
gcc -O0 -save-temps -dumpdir ./dump3 test.c helper.c -o test_program5 2>&1
echo "Exit status: $?"
rm -f helper.c test_program5 test.i test.s test.o

# Test 7: Different compilation stages
echo -e "\nTest 7: Testing different compilation stages"
gcc -E test.c -o test_preprocessed.i 2>&1
echo "Preprocess exit status: $?"
gcc -S test.c -o test_assembly.s 2>&1
echo "Assemble exit status: $?"
gcc -c test.c -o test_object.o 2>&1
echo "Compile only exit status: $?"
rm -f test_preprocessed.i test_assembly.s test_object.o

# Test 8: FINAL TEST - Trigger failure with complex options to hit the uncovered block
# This should set greatest_status to 1 and execute the cleanup logic
echo -e "\nTest 8: FINAL - Triggering failure with all options to cover reset block"
gcc -O0 -save-temps -dumpdir ./dump_final -dumpbase failtest -dumpbase-ext .c \
    --sysroot=/tmp/sysroot -specs=custom.specs -fuse-ld=gold \
    -time -wrapper /bin/echo -invalid-option-that-causes-failure error.c 2>&1 > /dev/null
echo "Final exit status: $? (should be non-zero)"

# Test 9: Alternative failure with valid options but invalid source
echo -e "\nTest 9: Alternative failure test"
gcc -O0 -save-temps -dumpdir ./dump_alt -dumpbase altfail \
    --sysroot=/tmp/sysroot nonexistent.c 2>&1 > /dev/null
echo "Alternative failure exit status: $?"

# Cleanup
rm -rf ./dump ./dump2 ./dump3 ./dump_final ./dump_alt
rm -f custom.specs test.c error.c

echo -e "\nTest sequence completed."
