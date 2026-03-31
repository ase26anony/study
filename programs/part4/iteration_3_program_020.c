/* 
 * Program designed to trigger GCC's internal tree initialization
 * for compiler-generated declarations with specific flags:
 * TREE_STATIC=1, TREE_PUBLIC=1, DECL_EXTERNAL=1, TREE_THIS_VOLATILE=1,
 * TREE_NOTHROW=1, DECL_ARTIFICIAL=1, DECL_IGNORED_P=1,
 * DECL_VISIBILITY_SPECIFIED=1, DECL_VISIBILITY=VISIBILITY_HIDDEN
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <pthread.h>

/* External function declaration to influence DECL_EXTERNAL handling */
extern int external_helper(void) __attribute__((weak));

/* Thread-local storage with dynamic initialization */
__thread int tls_var = 0;
static int init_counter = 0;

/* Function that forces non-constant initializer for TLS */
int get_initial_value(void) {
    return rand() % 100 + 1;
}

/* Function marked nothrow to provide context for TREE_NOTHROW */
void process_data(int value) __attribute__((nothrow));

/* Complex static initializer with side effects */
static int complex_static = (printf("Initializing complex_static\n"), 42);

/* Volatile variable to prevent optimization */
volatile int volatile_guard = 1;

/* Function using setjmp/longjmp for non-local jumps */
void nested_function_context(void) {
    jmp_buf env;
    int result;
    
    /* Nested function using GCC extension */
    __extension__ void nested_func(int x) {
        if (x > 50) {
            longjmp(env, 1);
        }
        tls_var += x;
    }
    
    result = setjmp(env);
    if (result == 0) {
        /* Call nested function multiple times */
        for (int i = 0; i < 10; i++) {
            nested_func(rand() % 100);
        }
    } else {
        printf("longjmp called, tls_var = %d\n", tls_var);
    }
}

/* Function with OpenMP pragma to generate offloading stubs */
#pragma omp declare target
int target_function(int x) {
    return x * 2 + complex_static;
}
#pragma omp end declare target

/* Multi-file simulation using weak attributes */
int external_helper(void) {
    return 123;  /* Actual definition if linked */
}

/* Nothrow function implementation */
void process_data(int value) {
    /* Access volatile to prevent dead code elimination */
    if (volatile_guard) {
        printf("Processing: %d\n", value);
    }
}

/* Main function with execution flow to trigger internal declarations */
int main(void) {
    int result = 0;
    
    /* Setup */
    srand(42);
    init_counter++;
    
    /* Initialize TLS with dynamic value - may generate initialization function */
    tls_var = get_initial_value();
    printf("Initial TLS value: %d\n", tls_var);
    
    /* Execute nested function with setjmp context */
    nested_function_context();
    
    /* Use OpenMP target region to generate host/device stubs */
    #pragma omp target map(tofrom: result)
    {
        result = target_function(tls_var);
    }
    
    /* Complex static initialization in loop context */
    static int loop_static = 0;
    if (loop_static == 0) {
        loop_static = (printf("Initializing loop_static\n"), tls_var);
    }
    
    /* Call external function (weakly linked) */
    if (external_helper) {
        result += external_helper();
    }
    
    /* Process result with nothrow function */
    process_data(result);
    
    /* Force visibility of all symbols */
    printf("Final result: %d\n", result);
    printf("Complex static: %d\n", complex_static);
    printf("Loop static: %d\n", loop_static);
    
    return 0;
}

/* Additional file simulation - separate translation unit hints */
#ifdef SIMULATE_MULTIFILE
/* This would be in a separate file in real multi-file compilation */
int external_helper(void) {
    return 456;
}
#endif
