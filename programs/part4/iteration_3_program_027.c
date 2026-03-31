/* 
 * This program is designed to trigger GCC's internal tree initialization
 * for compiler-generated artificial functions with hidden visibility.
 * It combines multiple patterns that force GCC to create internal declarations:
 * 1. Thread-local storage with dynamic initialization
 * 2. Nested functions with setjmp/longjmp
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
extern int weak_function(void) __attribute__((weak));

/* Thread-local storage with non-constant initializer */
__thread int tls_var = 0;
/* Volatile to prevent optimization */
volatile int volatile_seed = 42;

/* Complex static initializer with side effects */
static int static_init = (printf("Initializing static variable\n"), 42);

/* Function marked nothrow to provide context for TREE_NOTHROW */
static int nested_with_setjmp(void) __attribute__((nothrow));

/* External function definition (simulates multi-file) */
int external_helper(void) {
    return rand() % 100;
}

/* Weak function definition */
int weak_function(void) {
    return 0xDEADBEEF;
}

/* Main triggering function with nested setjmp */
static int nested_with_setjmp(void) {
    jmp_buf env;
    int result = 0;
    
    /* Nested function using GCC extension */
    __extension__ void nested_func(int *res) {
        /* Access TLS variable to force TLS initialization */
        tls_var = external_helper();
        
        /* Use setjmp/longjmp - GCC often creates internal helpers for this */
        if (setjmp(env) == 0) {
            /* Modify result through pointer */
            *res = tls_var + volatile_seed;
            
            /* Simulate non-local jump */
            longjmp(env, 1);
        } else {
            /* After longjmp */
            *res += weak_function();
        }
    }
    
    /* Execute the nested function */
    nested_func(&result);
    
    return result;
}

/* Function with OpenMP pragma to potentially generate offloading stubs */
#ifdef _OPENMP
static void omp_target_example(void) {
    #pragma omp target teams distribute parallel for simd
    for (int i = 0; i < 100; i++) {
        /* Empty but forces stub generation */
    }
}
#endif

int main(void) {
    int final_result = 0;
    
    /* Setup - seed random, initialize jmp_buf */
    srand(volatile_seed);
    
    printf("Program start\n");
    printf("Static init value: %d\n", static_init);
    
    /* Execute the key construct multiple times */
    for (int i = 0; i < 3; i++) {
        /* Call function with nested setjmp and TLS access */
        int res = nested_with_setjmp();
        printf("Iteration %d, result: %d, TLS var: %d\n", 
               i, res, tls_var);
        
        /* Modify volatile to prevent dead code elimination */
        volatile_seed += res;
        final_result += res;
    }
    
    #ifdef _OPENMP
    /* Trigger OpenMP offloading stub generation if available */
    omp_target_example();
    printf("OpenMP offloading used\n");
    #endif
    
    /* Access TLS from main to ensure it's used */
    tls_var = final_result % 100;
    
    /* Observable I/O with result */
    printf("Final result: %d\n", final_result);
    printf("Final TLS value: %d\n", tls_var);
    
    return final_result > 0 ? 0 : 1;
}

/* Force inclusion of setjmp/longjmp runtime */
void* force_setjmp_refs[] = {
    (void*)setjmp,
    (void*)longjmp,
    (void*)memset  /* Often used in setjmp implementation */
};
