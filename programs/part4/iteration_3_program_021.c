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

/* Volatile variable to prevent optimization */
volatile int global_counter = 0;

/* Thread-local storage with dynamic initialization */
__thread int tls_var = 0;

/* Function with nothrow attribute for TREE_NOTHROW context */
static void __attribute__((nothrow)) 
initialize_tls(void) {
    /* Dynamic TLS initialization - may trigger compiler-generated init function */
    tls_var = rand() % 100;
}

/* Function that uses setjmp/longjmp - may create artificial helper functions */
static int nested_function_with_jmp(void) {
    jmp_buf env;
    volatile int local_state = 0;
    
    /* Nested function using GCC extension */
    auto void nested_helper(void) {
        local_state = 1;
        longjmp(env, 1);
    }
    
    if (setjmp(env) == 0) {
        nested_helper();
        return 0;
    } else {
        return local_state;
    }
}

/* Complex static initializer with side effects */
static int complex_init = (printf("Initializing complex static...\n"), 42);

/* OpenMP target region (if supported) */
#ifdef _OPENMP
#pragma omp declare target
int target_var = 0;
#pragma omp end declare target
#endif

/* Multi-file simulation using weak attribute */
int external_helper(void) {
    return complex_init + 1;
}

/* Main execution flow with all triggering constructs */
int main(void) {
    int result = 0;
    
    /* Setup */
    srand(42);
    global_counter = 1;
    
    /* Execute TLS initialization (may generate hidden init function) */
    initialize_tls();
    
    /* Execute nested function with non-local jump */
    result += nested_function_with_jmp();
    
    /* Access TLS variable */
    tls_var += global_counter;
    result += tls_var;
    
    /* Use complex static initializer */
    result += complex_init;
    
    /* Simulate multi-file linking with weak external */
    if (external_helper) {
        result += external_helper();
    }
    
    /* OpenMP target region execution */
    #ifdef _OPENMP
    #pragma omp target map(tofrom: result)
    {
        target_var = result;
        result = target_var * 2;
    }
    #endif
    
    /* Force execution of all paths with observable output */
    printf("Final result: %d\n", result);
    printf("Global counter: %d\n", global_counter);
    printf("TLS variable: %d\n", tls_var);
    
    /* Additional volatile operations to prevent dead code elimination */
    {
        volatile int anti_opt = 0;
        for (int i = 0; i < 10; i++) {
            anti_opt += rand();
        }
        printf("Anti-optimization value: %d\n", anti_opt);
    }
    
    return result > 100 ? 0 : 1;
}

/* Additional function to create more tree nodes */
static void __attribute__((constructor, visibility("hidden")))
init_function(void) {
    /* This constructor may trigger hidden visibility declarations */
    global_counter = 1000;
}

/* Simulate exception handling context for TREE_NOTHROW */
void __attribute__((nothrow)) 
safe_operation(void) {
    /* Operations that shouldn't throw in C context */
    volatile int x = 0;
    x = rand();
    global_counter += x;
    
    /* Access TLS within nothrow context */
    tls_var = x % 50;
}
