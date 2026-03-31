/* 
 * This program is designed to trigger GCC's internal tree initialization
 * for compiler-generated declarations with specific flags:
 * TREE_STATIC, TREE_PUBLIC, DECL_EXTERNAL, TREE_USED, TREE_THIS_VOLATILE,
 * TREE_NOTHROW, DECL_ARTIFICIAL, DECL_IGNORED_P, DECL_VISIBILITY_SPECIFIED,
 * VISIBILITY_HIDDEN
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

/* External function declaration to influence DECL_EXTERNAL handling */
extern int external_helper(void);
/* Weak symbol to simulate multi-file compilation */
extern void __attribute__((weak)) weak_function(void);

/* Thread-local storage with dynamic initialization */
__thread volatile int tls_var = 0;
static int init_counter = 0;

/* Function with side effects for TLS initialization */
static int __attribute__((noinline)) init_tls(void) {
    int val = rand() % 100;
    printf("Initializing TLS with: %d\n", val);
    return val;
}

/* External function definition (simulates multi-file) */
int external_helper(void) {
    static int counter = 0;
    return ++counter;
}

/* Nested function using setjmp/longjmp - GCC may create internal helpers */
static void __attribute__((nothrow)) nested_function_with_jmp(void) {
    jmp_buf env;
    volatile int local_state = 0;
    
    /* Nested function using GCC extension */
    auto void nested_inner(void) {
        local_state = 1;
        if (setjmp(env) == 0) {
            /* First call */
            printf("setjmp called, local_state = %d\n", local_state);
        } else {
            /* longjmp return */
            printf("longjmp returned, local_state = %d\n", local_state);
        }
    }
    
    nested_inner();
    
    /* Simulate non-local jump */
    if (local_state == 1) {
        longjmp(env, 1);
    }
}

/* Complex static initializer with side effects */
static int complex_init = (printf("Complex static init\n"), external_helper());

/* Function with OpenMP pragma - may generate offloading stubs */
static void __attribute__((noinline)) omp_target_function(void) {
    volatile int result = 0;
    
    #pragma omp target map(tofrom: result) if(0)
    {
        /* This won't actually execute due to if(0), but may generate stubs */
        result = 42;
    }
    
    printf("OMP target result (placeholder): %d\n", result);
}

/* Main execution flow */
int main(void) {
    /* Setup */
    srand(42);
    init_counter++;
    
    /* Force TLS initialization with dynamic initializer */
    tls_var = init_tls();
    printf("TLS variable value: %d\n", tls_var);
    
    /* Access complex static initializer */
    printf("Complex init value: %d\n", complex_init);
    
    /* Call external helper */
    printf("External helper: %d\n", external_helper());
    
    /* Execute nested function with setjmp/longjmp */
    nested_function_with_jmp();
    
    /* Call OMP function (generates stubs even if not executed) */
    omp_target_function();
    
    /* Use weak function if available */
    if (weak_function) {
        weak_function();
    } else {
        printf("Weak function not available\n");
    }
    
    /* Volatile operations to prevent optimization */
    volatile int prevent_opt = 0;
    for (int i = 0; i < 10; i++) {
        prevent_opt += rand() % 10;
    }
    printf("Prevent optimization value: %d\n", prevent_opt);
    
    /* Final observable output */
    printf("Program completed successfully\n");
    printf("Final TLS value: %d, Init counter: %d\n", tls_var, init_counter);
    
    return 0;
}

/* Define the weak function (simulating another compilation unit) */
void __attribute__((weak)) weak_function(void) {
    printf("Weak function implementation called\n");
}
