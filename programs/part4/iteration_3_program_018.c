/* 
 * Program designed to trigger GCC's internal tree initialization
 * for compiler-generated declarations with specific flags:
 * TREE_STATIC, TREE_PUBLIC, DECL_EXTERNAL, TREE_USED, TREE_THIS_VOLATILE,
 * TREE_NOTHROW, DECL_ARTIFICIAL, DECL_IGNORED_P, DECL_VISIBILITY_SPECIFIED,
 * VISIBILITY_HIDDEN
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <pthread.h>

/* External function declaration to influence DECL_EXTERNAL handling */
extern int external_helper(void) __attribute__((weak));

/* Volatile variable to prevent optimization */
volatile int global_counter = 0;

/* Function marked nothrow to provide context for TREE_NOTHROW */
int __attribute__((nothrow)) safe_computation(int x) {
    return x * 2;
}

/* Thread-local storage with dynamic initialization */
__thread int tls_var = 0;

/* Function to initialize TLS dynamically */
int init_tls(void) {
    static int counter = 0;
    return ++counter + rand() % 100;
}

/* Complex static initializer with side effects */
static int complex_init = (printf("Initializing complex static...\n"), 42);

/* Nested function using setjmp (GCC extension) */
void nested_function_with_jmp(void) {
    jmp_buf env;
    volatile int local_state = 0;
    
    /* Define nested function using GCC's statement expression */
    auto void nested_helper(void) __attribute__((__always_inline__));
    
    void nested_helper(void) {
        local_state = 1;
        if (setjmp(env) == 0) {
            /* First call - setup */
            tls_var = init_tls();  /* Access TLS with dynamic init */
            longjmp(env, 1);
        } else {
            /* After longjmp */
            global_counter += tls_var;
        }
    }
    
    nested_helper();
}

/* OpenMP-like structure to potentially trigger offloading stubs */
#ifdef _OPENMP
#pragma omp declare target
int target_device_func(int x) {
    return x * x;
}
#pragma omp end declare target
#endif

/* Multi-file simulation using weak attributes */
int __attribute__((weak)) external_helper(void) {
    return complex_init + global_counter;
}

/* Main execution flow with observable I/O */
int main(void) {
    int result = 0;
    
    /* Setup */
    srand(42);
    printf("Program start\n");
    
    /* Force TLS initialization */
    tls_var = init_tls();
    printf("TLS initialized: %d\n", tls_var);
    
    /* Execute nested function with non-local jumps */
    for (int i = 0; i < 3; i++) {
        nested_function_with_jmp();
        printf("Iteration %d, global_counter: %d\n", i, global_counter);
    }
    
    /* Use volatile and external linkage */
    volatile int temp = safe_computation(complex_init);
    result = temp + external_helper();
    
    /* Complex static initialization with function call */
    static int runtime_init = (printf("Runtime init called\n"), 
                               rand() % 100 + complex_init);
    
    /* Observable output to prevent dead code elimination */
    printf("Final result: %d\n", result + runtime_init + tls_var);
    printf("Complex static value: %d\n", complex_init);
    printf("Runtime init value: %d\n", runtime_init);
    
    /* Additional construct: function pointer that might trigger 
       compiler-generated trampolines */
    void (* volatile fp)(void) = (void (*)(void))nested_function_with_jmp;
    if (global_counter > 0) {
        fp();
    }
    
    return 0;
}

/* Additional file simulation */
#ifndef MAIN_ONLY
/* Separate compilation unit simulation */
int secondary_helper(void) {
    /* This might trigger DECL_EXTERNAL in the main compilation */
    extern int global_counter;
    return global_counter * 2;
}
#endif
