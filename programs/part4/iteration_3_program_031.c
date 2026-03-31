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
extern void __attribute__((weak)) weak_function(void);

/* Thread-local storage with non-constant initializer */
__thread int tls_var = 0;
/* Volatile variable to prevent optimization */
volatile int volatile_counter = 0;

/* Function with nothrow context */
void __attribute__((nothrow)) nested_function_context(void) {
    jmp_buf env;
    int ret;
    
    /* Complex static initializer with side effect */
    static int static_counter = (printf("Initializing static_counter\n"), 0);
    
    /* Nested function using GCC extension */
    auto void nested_func(void) __attribute__((nothrow));
    
    void nested_func(void) {
        /* Access TLS variable */
        tls_var++;
        
        /* Use volatile to prevent dead code elimination */
        volatile_counter++;
        
        /* Non-local jump - may cause GCC to generate internal helpers */
        if (tls_var < 3) {
            longjmp(env, 1);
        }
    }
    
    /* Set up jump buffer */
    ret = setjmp(env);
    if (ret == 0) {
        /* First call to nested function */
        nested_func();
    } else {
        /* After longjmp */
        printf("After longjmp, tls_var = %d\n", tls_var);
    }
    
    /* Call nested function again to ensure execution */
    if (tls_var < 5) {
        nested_func();
    }
}

/* Function that initializes TLS with external call */
int init_tls(void) {
    return rand() % 100;
}

/* External function definition (simulating multi-file) */
int external_helper(void) {
    return 42;
}

/* Main execution flow */
int main(void) {
    /* Seed random for TLS initialization */
    srand(42);
    
    /* Initialize TLS with dynamic value */
    tls_var = init_tls();
    printf("Initial TLS value: %d\n", tls_var);
    
    /* Call function with nested function and setjmp */
    nested_function_context();
    
    /* Access TLS variable in loop to ensure usage */
    for (int i = 0; i < 3; i++) {
        tls_var += external_helper();
        volatile_counter++;
    }
    
    /* Complex static initialization in main */
    static int main_static = (printf("Main static init\n"), tls_var + volatile_counter);
    
    /* Final output to prevent optimization */
    printf("Final values - TLS: %d, Volatile: %d, Static: %d\n", 
           tls_var, volatile_counter, main_static);
    
    /* Try to call weak function (may generate stub) */
    if (weak_function) {
        weak_function();
    }
    
    return 0;
}

/* Define the weak function to avoid linker errors */
void __attribute__((weak)) weak_function(void) {
    printf("Weak function called\n");
}

/* Additional construct: OpenMP offloading (if supported) */
#ifdef _OPENMP
void omp_target_example(void) {
    int arr[10] = {0};
    
    #pragma omp target map(tofrom: arr[0:10])
    {
        for (int i = 0; i < 10; i++) {
            arr[i] = i * 2;
        }
    }
    
    printf("OpenMP target result: %d\n", arr[5]);
}
#endif
