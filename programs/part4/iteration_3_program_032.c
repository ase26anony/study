/* 
 * This program is designed to trigger GCC's internal tree initialization
 * for compiler-generated declarations with specific flags:
 * TREE_STATIC=1, TREE_PUBLIC=1, DECL_EXTERNAL=1, TREE_USED=1,
 * TREE_THIS_VOLATILE=1, TREE_NOTHROW=1, DECL_ARTIFICIAL=1,
 * DECL_IGNORED_P=1, DECL_VISIBILITY_SPECIFIED=1, VISIBILITY_HIDDEN
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <pthread.h>

/* External function declaration to influence DECL_EXTERNAL handling */
extern int external_helper(void);
/* Weak attribute to simulate multi-file compilation */
extern int maybe_defined_elsewhere(void) __attribute__((weak));

/* Thread-local storage with dynamic initialization */
__thread int tls_var = 0;
__thread volatile int tls_volatile = 0;

/* Function that will be marked nothrow */
static int nested_with_jump(void) __attribute__((nothrow));

/* Global jmp_buf for non-local jumps */
static jmp_buf jump_buffer;

/* Function with side effects for static initialization */
static int init_with_side_effects(void) {
    printf("Initializing with side effects\n");
    return rand() % 100;
}

/* Static variable with complex initializer - may trigger hidden init function */
static int complex_static = (printf("Complex static init\n"), init_with_side_effects());

/* External function definition (simulates multi-file) */
int external_helper(void) {
    return 42;
}

/* Weak function definition */
int maybe_defined_elsewhere(void) {
    return 99;
}

/* Nested function using setjmp/longjmp pattern */
static int nested_with_jump(void) {
    volatile int counter = 0;
    int result = 0;
    
    /* GCC may create artificial functions for setjmp context */
    if (setjmp(jump_buffer) == 0) {
        /* First call - setup */
        counter++;
        tls_var = external_helper();
        tls_volatile = complex_static;
        
        /* Nested function using GCC extension */
        auto void nested(void) __attribute__((nothrow));
        
        void nested(void) {
            /* Access TLS and volatile variables */
            tls_var += tls_volatile;
            counter++;
            
            /* Force non-local jump - may trigger internal function generation */
            if (counter < 3) {
                longjmp(jump_buffer, 1);
            }
        }
        
        /* Call nested function */
        nested();
    } else {
        /* After longjmp */
        result = tls_var + tls_volatile;
    }
    
    return result;
}

/* OpenMP target region (if supported) */
#ifdef _OPENMP
#pragma omp declare target
int target_device_var = 0;
#pragma omp end declare target
#endif

int main(void) {
    int final_result = 0;
    
    /* Seed random for volatile behavior */
    srand(0);
    
    /* Setup TLS */
    tls_var = maybe_defined_elsewhere();
    tls_volatile = rand();
    
    /* Execute the nested function construct */
    final_result = nested_with_jump();
    
    /* Use OpenMP if available */
    #ifdef _OPENMP
    #pragma omp parallel
    {
        #pragma omp single
        {
            printf("OpenMP thread %d\n", omp_get_thread_num());
        }
    }
    
    /* Target region may create host/device stubs */
    #pragma omp target map(tofrom: final_result)
    {
        target_device_var = final_result;
        final_result += 1;
    }
    #endif
    
    /* Ensure all constructs are used and observable */
    printf("Result: %d\n", final_result);
    printf("TLS var: %d\n", tls_var);
    printf("Complex static: %d\n", complex_static);
    
    /* Additional volatile access to prevent optimization */
    volatile int prevent_opt = tls_volatile;
    printf("Volatile: %d\n", prevent_opt);
    
    return 0;
}
