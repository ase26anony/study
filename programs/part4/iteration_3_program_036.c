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
extern int external_helper(void) __attribute__((weak));

/* Volatile variable to prevent optimization */
volatile int global_counter = 0;

/* Function marked nothrow to provide context for TREE_NOTHROW flag */
static void process_data(void) __attribute__((nothrow));

/* Thread-local storage with non-constant initializer */
__thread int tls_var = 0;

/* Function to dynamically initialize TLS - may cause GCC to generate initialization stub */
static int init_tls(void) {
    return rand() % 100 + 1;
}

/* Complex static variable with side effects - may trigger hidden initialization function */
static int complex_static = (printf("Initializing complex_static\n"), 42);

/* External function definition (simulating multi-file compilation) */
int external_helper(void) {
    return complex_static * 2;
}

/* Nested function using setjmp/longjmp - often forces GCC to create artificial functions */
static void nested_function_with_jump(void) {
    jmp_buf env;
    int val = 0;
    
    /* Define nested function using GCC extension */
    __extension__ void nested(int x) {
        if (x > 10) {
            longjmp(env, 1);
        }
        tls_var += x;  /* Access TLS variable */
    }
    
    if (setjmp(env) == 0) {
        /* Call nested function multiple times */
        for (int i = 0; i < 15; i++) {
            nested(i);
            val += i;
        }
    } else {
        printf("longjmp called, tls_var = %d\n", tls_var);
    }
    
    global_counter += val;
}

/* Function with nothrow attribute containing the triggering constructs */
static void process_data(void) {
    static int first_call = 1;
    
    if (first_call) {
        /* Initialize TLS with dynamic value - may trigger hidden init function */
        tls_var = init_tls();
        printf("TLS initialized to: %d\n", tls_var);
        first_call = 0;
    }
    
    /* Use nested function with non-local jumps */
    nested_function_with_jump();
    
    /* Access complex static variable */
    complex_static += tls_var;
    
    /* Call external function */
    if (external_helper) {
        int result = external_helper();
        printf("External helper result: %d\n", result);
    }
}

/* Main function with observable I/O to prevent dead code elimination */
int main(void) {
    /* Seed random number generator */
    srand(42);
    
    printf("Program start - global_counter: %d\n", global_counter);
    
    /* Execute the triggering construct multiple times */
    for (int i = 0; i < 3; i++) {
        process_data();
        printf("Iteration %d: tls_var = %d, complex_static = %d\n", 
               i, tls_var, complex_static);
    }
    
    /* Final observable output */
    printf("Program end - global_counter: %d\n", global_counter);
    
    return 0;
}
