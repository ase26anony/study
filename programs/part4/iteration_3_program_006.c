/* test_targhooks.c - Designed to trigger GCC's internal declaration generation */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <pthread.h>

/* External function declaration to influence DECL_EXTERNAL handling */
extern void external_helper(void);

/* Weak attribute to influence linkage decisions */
void weak_function(void) __attribute__((weak));

/* Function marked nothrow to provide context for TREE_NOTHROW */
void nothrow_function(void) __attribute__((nothrow));

/* Global volatile variable to prevent optimizations */
volatile int global_counter = 0;

/* Thread-local storage with dynamic initialization */
__thread int tls_var = 0;

/* Function that returns a non-constant value for TLS initialization */
int get_initial_value(void) {
    return rand() % 100;
}

/* Complex static initializer with side effects */
static int complex_static = (printf("Initializing complex_static\n"), 42);

/* Nested function using setjmp/longjmp (GCC extension) */
void test_nested_function(void) {
    jmp_buf env;
    volatile int local_state = 0;
    
    /* Define nested function using GCC's statement expression */
    auto void nested_func(void) __attribute__((nothrow));
    
    void nested_func(void) {
        local_state = 1;
        if (setjmp(env) == 0) {
            /* First call - setup */
            tls_var = get_initial_value();
            global_counter++;
        } else {
            /* longjmp return */
            tls_var += 10;
        }
    }
    
    /* Execute the nested function */
    nested_func();
    
    /* Simulate non-local jump context */
    if (local_state == 1 && global_counter < 3) {
        longjmp(env, 1);
    }
}

/* Function with OpenMP pragma to generate offloading stubs */
#pragma GCC push_options
#pragma GCC optimize("O2")
void omp_target_function(void) {
    int arr[100];
    
    #pragma omp target map(tofrom: arr[0:100])
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 2 + tls_var;
    }
    
    /* Use the result to prevent dead code elimination */
    printf("OMP result: %d\n", arr[50]);
}
#pragma GCC pop_options

/* Implementation of weak function */
void weak_function(void) {
    printf("Weak function called\n");
}

/* Implementation of nothrow function */
void nothrow_function(void) {
    /* Access TLS variable to force initialization */
    tls_var = get_initial_value();
    
    /* Call nested function */
    test_nested_function();
}

/* Dummy external helper implementation */
void external_helper(void) {
    /* Empty but referenced to prevent removal */
    global_counter++;
}

int main(void) {
    /* Seed random number generator */
    srand(42);
    
    /* Initialize TLS with dynamic value */
    tls_var = get_initial_value();
    
    printf("Starting test program\n");
    printf("Initial TLS value: %d\n", tls_var);
    printf("Complex static value: %d\n", complex_static);
    
    /* Call nothrow function to provide TREE_NOTHROW context */
    nothrow_function();
    
    /* Call weak function */
    if (weak_function) {
        weak_function();
    }
    
    /* Execute OpenMP target region */
    #ifdef _OPENMP
    omp_target_function();
    #endif
    
    /* Force multiple executions with different conditions */
    for (int i = 0; i < 3; i++) {
        test_nested_function();
        printf("Iteration %d: TLS = %d, Global = %d\n", 
               i, tls_var, global_counter);
    }
    
    /* Call external function */
    external_helper();
    
    /* Final output to ensure all code paths are live */
    printf("Final TLS value: %d\n", tls_var);
    printf("Test completed successfully\n");
    
    return 0;
}
