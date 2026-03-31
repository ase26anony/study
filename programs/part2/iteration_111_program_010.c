/* test_coverage.c - Main test file to trigger dumpdir/dumpbase allocation */
/* This file uses various techniques to ensure GCC driver allocates dumpdir, 
   dumpbase, dumpbase_ext, and outbase strings, which are later freed in 
   driver::finalize() */

/* Technique 1: Use pragmas to influence driver behavior */
#pragma GCC optimize("O0")
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

/* Technique 2: Create multiple compilation units via #include */
/* This simulates multiple input files being processed together */
#include "test_aux1.h"
#include "test_aux2.h"

/* Technique 3: Use attributes that affect code generation */
__attribute__((noinline)) 
__attribute__((optimize("O3"))) 
static int compute_factorial(int n) {
    if (n <= 1) return 1;
    return n * compute_factorial(n - 1);
}

/* Technique 4: Create a constructor to ensure full compilation pipeline */
__attribute__((constructor))
static void init_coverage(void) {
    /* This ensures the driver processes the entire compilation */
    volatile int marker = 42;
    (void)marker;
}

/* Technique 5: Use static assertion to engage type checking */
_Static_assert(sizeof(int) == 4, "int must be 4 bytes");

/* Technique 6: Generate warnings that can be controlled */
#ifdef GENERATE_WARNING
#warning "This is a test warning for driver error handling"
#endif

/* Main function with various optimizations */
__attribute__((optimize("O2")))
int main(void) {
    /* Use different optimization levels in different scopes */
    #pragma GCC optimize("O1")
    int result = compute_factorial(5);
    
    /* Create unused variable to potentially trigger warning */
    #ifdef TRIGGER_WARNING
    int unused = 0;
    #endif
    
    /* Use inline assembly to ensure assembly generation */
    __asm__ volatile ("nop" : : : "memory");
    
    return result == 120 ? 0 : 1;
}

#pragma GCC diagnostic pop
