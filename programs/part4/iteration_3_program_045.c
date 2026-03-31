/* 
 * This program is designed to trigger GCC's internal tree initialization
 * for compiler-generated artificial functions with hidden visibility.
 * It combines multiple patterns that force GCC to create internal declarations:
 * 1. Thread-local storage with dynamic initialization
 * 2. Nested functions with non-local jumps using setjmp/longjmp
 * 3. Complex static initializers with side effects
 * 4. External linkage and volatile variables
 * 5. Exception handling context via nothrow attribute
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

/* External function declaration to influence DECL_EXTERNAL handling */
extern int external_helper(void);
/* Weak attribute to simulate multi-file compilation */
extern int maybe_defined_elsewhere(void) __attribute__((weak));

/* Complex static initializer with side effects - may generate initialization function */
static volatile int complex_static = (printf("Initializing complex_static\n"), rand() % 100);

/* TLS with dynamic initialization - often generates runtime initialization stubs */
__thread int tls_var = (printf("Initializing TLS\n"), 42);

/* Global jmp_buf for non-local jumps */
static jmp_buf jump_buffer;

/* Function marked nothrow to provide context for TREE_NOTHROW flag */
static void process_data(void) __attribute__((nothrow));

/* External function definition (simulates multi-file) */
int external_helper(void) {
    return rand() % 50;
}

/* Weak function definition */
int maybe_defined_elsewhere(void) {
    return 100;
}

/* Main processing function with nothrow attribute */
static void process_data(void) {
    volatile int counter = 0;  /* Volatile to prevent optimization */
    
    /* Nested function using GCC extension - may create artificial static helper */
    __extension__ void nested_func(int x) {
        if (x > 10) {
            /* Non-local jump - forces GCC to create trampoline or helper */
            longjmp(jump_buffer, 1);
        }
        counter += x;
    }
    
    /* Execute nested function in a loop */
    for (int i = 0; i < 5; i++) {
        nested_func(i + tls_var);  /* Access TLS variable */
    }
    
    /* Access complex static */
    counter += complex_static;
    
    printf("Counter: %d\n", counter);
}

int main(void) {
    int result = 0;
    
    /* Seed random for dynamic initialization */
    srand(42);
    
    /* Setup jump point for non-local returns */
    if (setjmp(jump_buffer) == 0) {
        /* First call to external function */
        result += external_helper();
        
        /* Call weak function */
        result += maybe_defined_elsewhere();
        
        /* Modify TLS variable */
        tls_var += result;
        
        /* Process data with nested function */
        process_data();
        
        /* Access TLS again */
        printf("TLS value: %d\n", tls_var);
        
        /* Complex operation that might trigger internal function generation */
        {
            static int initialized = 0;
            if (!initialized) {
                /* This static initialization with side effects may generate helper */
                static int side_effect_var = (printf("Side effect initialization\n"), 99);
                initialized = side_effect_var;
            }
        }
    } else {
        printf("Longjmp executed\n");
    }
    
    /* Final observable output */
    printf("Final result: %d\n", result + complex_static + tls_var);
    
    return 0;
}
