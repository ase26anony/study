/* test_targhooks.c - Designed to trigger GCC's internal declaration generation */
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <pthread.h>

/* External function declaration to influence DECL_EXTERNAL handling */
extern int external_helper(void);

/* Weak attribute to influence linkage decisions */
int __attribute__((weak)) weak_function(int x) {
    return x * 2;
}

/* Nothrow function context */
void __attribute__((nothrow)) nested_function_context(void) {
    /* Thread-local storage with dynamic initialization */
    static __thread int tls_var = 0;
    volatile int counter = 0;
    jmp_buf env;
    
    /* Force compiler to generate initialization function for TLS */
    if (tls_var == 0) {
        tls_var = external_helper() + rand();
    }
    
    /* Nested function using setjmp - GCC may create artificial helper */
    if (setjmp(env) == 0) {
        /* Complex static initializer with side effects */
        static int complex_static = (printf("Initializing complex static\n"), 42);
        
        /* Access volatile to prevent optimization */
        counter = complex_static + tls_var;
        
        /* Simulate non-local jump context */
        if (counter > 30) {
            longjmp(env, 1);
        }
    }
}

/* Function with OpenMP pragma - may generate offloading stubs */
#pragma omp declare target
int target_function(int x) {
    return x * x;
}
#pragma omp end declare target

/* Multi-file simulation: external function definition */
int external_helper(void) {
    static int init_counter = 0;
    return ++init_counter;
}

/* Main function with observable execution flow */
int main(void) {
    int result = 0;
    
    /* Setup */
    srand(42);
    
    /* Loop to ensure execution of triggering constructs */
    for (int i = 0; i < 5; i++) {
        /* Call function with nested context */
        nested_function_context();
        
        /* Use OpenMP target region - may generate host/device stubs */
        #pragma omp target map(tofrom: result)
        {
            result += target_function(i);
        }
        
        /* Use weak function */
        result += weak_function(i);
    }
    
    /* Observable output to prevent dead code elimination */
    printf("Final result: %d\n", result);
    
    /* Additional volatile access */
    volatile int check = result;
    if (check > 100) {
        printf("Result exceeds threshold\n");
    }
    
    return 0;
}
