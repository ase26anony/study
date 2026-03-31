/* 
 * This program is designed to trigger GCC's internal tree initialization
 * for compiler-generated artificial functions with hidden visibility.
 * It combines multiple patterns that force GCC to create internal declarations:
 * 1. Thread-local storage with dynamic initialization
 * 2. Nested functions with non-local jumps using setjmp/longjmp
 * 3. Complex static initializers with side effects
 * 4. External linkage hints and volatile variables
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

/* External function declaration to influence DECL_EXTERNAL handling */
extern int external_helper(void);
/* Weak attribute to simulate multi-file compilation scenario */
extern int maybe_defined_elsewhere(void) __attribute__((weak));

/* Complex static initializer with side effects - may trigger initialization function */
static int complex_static = (printf("Initializing complex_static\n"), 42);

/* Thread-local storage with non-constant initializer - often requires runtime init stub */
__thread int tls_var = 0;

/* Volatile variable to prevent optimization */
volatile int volatile_counter = 0;

/* Function marked nothrow to provide context for TREE_NOTHROW flag */
int __attribute__((nothrow)) nothrow_function(int x) {
    return x * 2;
}

/* External helper function definition */
int external_helper(void) {
    return rand() % 100;
}

/* Function using nested function with setjmp - may create trampoline or helper */
void test_nested_with_jump(void) {
    jmp_buf env;
    int result = 0;
    
    /* Nested function using GCC extension */
    auto void nested_func(void) __attribute__((nothrow));
    
    void nested_func(void) {
        volatile_counter++;
        if (setjmp(env) == 0) {
            /* First call - do some work */
            tls_var = external_helper();
            printf("Nested function: tls_var = %d\n", tls_var);
            
            /* Simulate error condition and longjmp */
            if (tls_var > 50) {
                longjmp(env, 1);
            }
        } else {
            /* After longjmp */
            printf("After longjmp, volatile_counter = %d\n", volatile_counter);
        }
    }
    
    /* Call the nested function */
    nested_func();
    
    /* Access TLS variable */
    result = tls_var + complex_static;
    printf("Result from nested test: %d\n", result);
}

/* Function using OpenMP-like offloading pattern (simulated) */
#pragma GCC push_options
#pragma GCC optimize("O2")
void simulate_offloading(void) {
    /* Static variable with complex initializer in loop */
    static int init_counter = 0;
    
    /* This pattern might trigger internal helper generation */
    for (int i = 0; i < 3; i++) {
        static int loop_static = (init_counter++, printf("Loop static init #%d\n", init_counter), 100 + i);
        
        /* Use nothrow function */
        int processed = nothrow_function(loop_static + tls_var);
        
        printf("Offload simulation iter %d: %d\n", i, processed);
        
        /* Modify TLS - forces runtime initialization consideration */
        tls_var += processed % 10;
    }
}
#pragma GCC pop_options

/* Main function with observable I/O and execution of all patterns */
int main(void) {
    /* Setup */
    srand(42);
    printf("Program start\n");
    
    /* Initialize TLS variable through external function */
    tls_var = external_helper();
    printf("Initial tls_var: %d\n", tls_var);
    
    /* Test weak symbol (simulating multi-file scenario) */
    if (&maybe_defined_elsewhere) {
        printf("Weak function might be defined elsewhere\n");
    }
    
    /* Execute nested function with jump */
    test_nested_with_jump();
    
    /* Execute offloading simulation */
    simulate_offloading();
    
    /* Final computation with all components */
    int final_result = complex_static + tls_var + volatile_counter;
    printf("Final result: %d\n", final_result);
    
    /* Ensure all code paths are live */
    if (final_result > 0) {
        printf("Program completed successfully\n");
    } else {
        printf("Unexpected result\n");
    }
    
    return 0;
}

/* Dummy definition for weak function if not linked elsewhere */
int maybe_defined_elsewhere(void) {
    return 1;
}
