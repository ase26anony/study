#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <pthread.h>

/* External function to force external linkage context */
extern void external_helper(void);
void external_helper(void) {
    /* Empty but forces external linkage context */
}

/* Volatile variable to prevent optimization */
volatile int global_counter = 0;

/* Weak attribute to influence linkage decisions */
__attribute__((weak)) int weak_function(int x) {
    return x * 2;
}

/* Function marked nothrow to provide context for TREE_NOTHROW */
int __attribute__((nothrow)) safe_operation(int a, int b) {
    /* Thread-local storage with non-constant initializer */
    static __thread int tls_var = 0;
    
    /* This forces runtime TLS initialization function */
    tls_var += (rand() % 100);
    
    /* Complex static initializer with side effects */
    static int complex_static = (printf("Initializing complex_static\n"), 42);
    
    /* Nested function using GCC extension - often creates trampolines */
    if (global_counter < 3) {
        __label__ exit_label;
        jmp_buf env;
        
        auto void nested_func(void) __attribute__((always_inline));
        void nested_func(void) {
            /* Use builtin setjmp/longjmp for compiler internal handling */
            if (__builtin_setjmp(env) == 0) {
                /* Access TLS variable to ensure it's used */
                tls_var += a + b + complex_static;
                /* Call weak function */
                tls_var += weak_function(tls_var);
                __builtin_longjmp(env, 1);
            }
        }
        
        nested_func();
    }
    
    return tls_var + complex_static;
}

/* Another function with different TLS model */
void __attribute__((noinline)) tls_heavy_function(void) {
    /* TLS with more complex initialization */
    __thread static int tls_array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    __thread static int tls_dynamic = (rand() % 1000);
    
    for (int i = 0; i < 10; i++) {
        tls_array[i] += tls_dynamic;
    }
    
    /* Force the compiler to generate TLS helper functions */
    volatile int* ptr = tls_array;
    while (*ptr == 0) { /* Just to create control flow */ }
}

/* Main function with observable I/O */
int main(void) {
    /* Seed random for TLS initialization */
    srand(42);
    
    /* Setup jmp_buf */
    jmp_buf main_env;
    
    printf("Starting program...\n");
    
    /* Loop to execute the triggering constructs multiple times */
    for (int i = 0; i < 5; i++) {
        global_counter = i;
        
        /* Execute the function with nested function and TLS */
        int result = safe_operation(i, i * 2);
        printf("Iteration %d, result: %d\n", i, result);
        
        /* Execute TLS-heavy function */
        tls_heavy_function();
        
        /* Call external function */
        external_helper();
        
        /* Use setjmp/longjmp to potentially trigger internal helpers */
        if (setjmp(main_env) == 0) {
            if (i == 2) {
                longjmp(main_env, 1);
            }
        }
    }
    
    /* Final observable output */
    printf("Program completed successfully.\n");
    
    /* Access weak function to ensure it's not eliminated */
    int weak_result = weak_function(42);
    printf("Weak function result: %d\n", weak_result);
    
    return 0;
}
