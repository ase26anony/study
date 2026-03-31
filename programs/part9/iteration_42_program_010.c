// Using function attributes (cleaner syntax)
void foo() __attribute__((optimize("O0"))) {
    /* complex code */
}

// Using command line options for specific files
// gcc -O2 -c main.c -o main.o
// gcc -O0 -c foo.c -o foo.o  # Compile only foo.c with -O0
