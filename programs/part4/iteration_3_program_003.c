/* 
 * This program is designed to trigger GCC's internal tree initialization
 * for compiler-generated declarations with specific flags:
 * TREE_STATIC, TREE_PUBLIC, DECL_EXTERNAL, TREE_THIS_VOLATILE, 
 * TREE_NOTHROW, DECL_ARTIFICIAL, DECL_IGNORED_P, 
 * DECL_VISIBILITY_SPECIFIED, VISIBILITY_HIDDEN
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <pthread.h>

/* External function declaration to influence DECL_EXTERNAL handling */
extern int external_helper(void);
/* Weak symbol to simulate multi-file compilation */
extern int weak_symbol __attribute__((weak));

/* Thread-local storage with dynamic initialization */
__thread volatile int tls_var = 0;
static int init_counter = 0;

/* Function with side effects for TLS initialization */
static int get_initial_value(void) {
    return rand() % 100 + 1;
}

/* Function marked nothrow to provide context for TREE_NOTHROW */
static void process_data(void) __attribute__((nothrow));

/* Complex static initializer with side effects */
static int complex_static = (printf("Initializing complex_static\n"), 42);

/* Nested function using setjmp/longjmp (GCC extension) */
static void nested_function_with_jmp(void) {
    jmp_buf env;
    volatile int local_counter = 0;
    
    /* Use GCC's statement expression for nested function */
    auto void nested_helper(void) __attribute__((nothrow));
    
    void nested_helper(void) {
        local_counter++;
        if (local_counter < 3) {
            longjmp(env, 1);
        }
    }
    
    if (setjmp(env) == 0) {
        nested_helper();
    }
    
    tls_var += local_counter;
}

/* Function using OpenMP pragma to generate offloading stubs */
#pragma omp declare target
static int target_function(int x) {
    return x * 2;
}
#pragma omp end declare target

/* Main execution with observable I/O */
int main(void) {
    int result = 0;
    
    /* Setup */
    srand(42);
    jmp_buf main_env;
    
    /* Initialize TLS with dynamic value */
    tls_var = get_initial_value();
    printf("Initial TLS value: %d\n", tls_var);
    
    /* Execute nested function construct */
    for (int i = 0; i < 2; i++) {
        nested_function_with_jmp();
        printf("Loop iteration %d, TLS = %d\n", i, tls_var);
    }
    
    /* Use OpenMP target region */
    #pragma omp target map(tofrom: result)
    {
        result = target_function(tls_var);
    }
    
    /* Access complex static variable */
    result += complex_static;
    
    /* Simulate external linkage usage */
    if (&weak_symbol != NULL) {
        result += weak_symbol;
    }
    
    /* Final observable output */
    printf("Final result: %d\n", result);
    
    /* Force volatile access */
    volatile int final_check = result;
    if (final_check > 100) {
        printf("Result exceeds threshold\n");
    }
    
    return 0;
}

/* Definition of external function */
int external_helper(void) {
    return rand() % 50;
}

/* Definition of weak symbol */
int weak_symbol = 99;

/* Function implementation */
static void process_data(void) {
    /* This function cannot throw (nothrow attribute) */
    tls_var = (tls_var * 3) / 2;
}
