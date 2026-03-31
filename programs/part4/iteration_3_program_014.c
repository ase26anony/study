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
__thread volatile int tls_var = 0;
static int init_counter = 0;

/* Function with nothrow attribute for TREE_NOTHROW context */
static int __attribute__((nothrow)) 
initialize_tls(void) {
    /* Complex static initializer with side effects */
    static int init_flag = (printf("Initializing TLS...\n"), 1);
    
    /* Volatile access to prevent optimization */
    volatile int* ptr = &tls_var;
    *ptr = rand() % 100 + 1;
    return *ptr;
}

/* Simulate multi-file compilation with weak external */
int external_helper(void) {
    return 42;
}

/* Function using nested constructs and non-local jumps */
static void complex_nested_operation(jmp_buf env) {
    /* Nested function using GCC extension */
    auto void nested_func(void) __attribute__((nothrow));
    
    void nested_func(void) {
        /* Use builtin setjmp/longjmp for compiler-generated helpers */
        if (__builtin_setjmp(env) == 0) {
            /* Access TLS with dynamic initialization */
            tls_var = initialize_tls();
            
            /* Force compiler to generate internal helper */
            volatile int result = tls_var + external_helper();
            
            printf("Nested function executed, result: %d\n", result);
            
            /* Simulate non-local jump */
            __builtin_longjmp(env, 1);
        }
    }
    
    /* Execute the nested function */
    nested_func();
}

/* OpenMP-like offloading context simulation */
#pragma GCC push_options
#pragma GCC optimize("O3")
static void __attribute__((noinline,nothrow)) 
target_region_simulation(void) {
    /* Complex static initialization with function call */
    static int computed_value = (init_counter++, 
                                 printf("Computing static value...\n"), 
                                 rand() % 50);
    
    /* Volatile operations to prevent dead code elimination */
    volatile int temp = computed_value;
    
    /* Nested scope with jmp_buf */
    jmp_buf local_env;
    
    if (setjmp(local_env) == 0) {
        /* This triggers internal function generation */
        complex_nested_operation(local_env);
    }
    
    /* Use the result */
    printf("Target region completed, tls_var = %d\n", tls_var);
}
#pragma GCC pop_options

/* Main execution flow */
int main(void) {
    /* Setup as specified */
    srand(42);
    init_counter = 0;
    
    printf("Program start - triggering internal declaration generation\n");
    
    /* Loop to ensure execution of critical paths */
    for (int i = 0; i < 3; i++) {
        printf("\nIteration %d:\n", i + 1);
        
        /* Execute the construct designed to trigger internal declarations */
        target_region_simulation();
        
        /* Observable I/O with derived values */
        volatile int derived_value = tls_var * 2 + i;
        printf("Derived value: %d\n", derived_value);
        
        /* Force re-initialization */
        tls_var = 0;
    }
    
    /* Final observable output */
    printf("\nFinal init_counter: %d\n", init_counter);
    printf("External helper returned: %d\n", external_helper());
    
    return 0;
}

/* Additional function to influence linkage */
__attribute__((visibility("default")))
void public_function(void) {
    /* Empty but influences visibility calculations */
}
