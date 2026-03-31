/* test_target.c - Main source file with various compiler directives */
#include <stdio.h>
#include <stdlib.h>

/* Pattern 1: Use pragmas to influence driver behavior */
#pragma GCC optimize("O0")
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

/* Pattern 2: Function with optimization attribute */
__attribute__((optimize("O3"))) 
int compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

/* Pattern 3: Constructor to ensure full compilation pipeline */
__attribute__((constructor))
static void init(void) {
    /* This ensures the driver processes constructor code */
    printf("Initializing...\n");
}

/* Pattern 4: Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup(void) {
    printf("Cleaning up...\n");
}

/* Pattern 5: Force some compiler work with inline assembly */
static void dummy_asm(void) {
    __asm__ volatile ("nop" : : : "memory");
}

/* Pattern 6: Trigger warnings that can be controlled */
__attribute__((unused))
static int unused_function(void) {
    return 42;
}

/* Main function with compile-time assertion */
int main(int argc, char **argv) {
    /* Pattern 7: Static assertion to engage type checking */
    _Static_assert(sizeof(int) == 4, "int must be 4 bytes");
    
    /* Pattern 8: Variable with cleanup attribute */
    int __attribute__((cleanup(free))) *ptr = malloc(sizeof(int));
    if (ptr) {
        *ptr = compute_factorial(5);
        printf("Factorial(5) = %d\n", *ptr);
        free(ptr);
    }
    
    dummy_asm();
    
    /* Pattern 9: Return different values based on input */
    if (argc > 1) {
        return atoi(argv[1]);
    }
    
    return 0;
}

/* Pattern 10: Conditional compilation with error-like code */
#if 0
/* This looks like an error but won't be compiled */
int syntax error here;
#endif
