/* test_targhooks.c - Designed to trigger GCC's internal tree initialization
   for artificial, hidden visibility declarations (targhooks.cc lines 981-990) */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <omp.h>

/* External function declaration to influence DECL_EXTERNAL/TREE_PUBLIC */
extern int external_helper(void);
int external_helper(void) { return rand() % 100; }

/* Volatile variable to prevent optimization */
volatile int volatile_trigger = 1;

/* Thread-local storage with dynamic initialization */
__thread int tls_var = 0;

/* Function with nothrow attribute to provide context for TREE_NOTHROW */
static void __attribute__((nothrow)) 
nested_with_jump(int base) {
    jmp_buf env;
    volatile int counter = base;
    
    /* Nested function using GCC's statement expression extension */
    auto void nested() __attribute__((__nothrow__));
    
    void nested() {
        if (setjmp(env) == 0) {
            /* Access TLS variable - may trigger TLS initialization helper */
            tls_var = external_helper() + counter;
            
            /* Complex static initializer with side effects */
            static int complex_static = (printf("[static init] "), 
                                         fflush(stdout), 
                                         external_helper() + 1);
            
            /* Force longjmp to simulate non-local jump */
            if (counter++ < 3) {
                longjmp(env, 1);
            }
        }
    }
    
    /* Execute the nested function */
    nested();
    
    /* Use the results to prevent dead code elimination */
    printf("tls_var=%d, counter=%d\n", tls_var, counter);
}

/* OpenMP target region to generate offloading stubs */
#pragma omp declare target
int device_var = 0;
#pragma omp end declare target

/* Weak symbol to influence linkage decisions */
int __attribute__((weak)) weak_function(int x) {
    return x * 2;
}

int main(void) {
    /* Setup */
    srand(42);
    jmp_buf main_env;
    
    printf("Starting test for targhooks.cc lines 981-990...\n");
    
    /* Loop with volatile condition to ensure execution */
    for (volatile int i = 0; i < volatile_trigger + 1; i++) {
        /* Execute nested function with setjmp/longjmp */
        nested_with_jump(i * 10);
        
        /* OpenMP target region - may generate host/device stubs */
        #pragma omp target if(0)  /* if(0) to run on host but still analyze */
        {
            device_var = i;
        }
        
        /* Call weak function */
        int result = weak_function(i);
        printf("weak_function(%d) = %d\n", i, result);
    }
    
    /* Multi-threaded TLS access */
    #pragma omp parallel num_threads(2)
    {
        tls_var = omp_get_thread_num() + 100;
        printf("Thread %d: tls_var = %d\n", 
               omp_get_thread_num(), tls_var);
    }
    
    /* Final observable output */
    printf("Test completed successfully.\n");
    printf("Final TLS value in main thread: %d\n", tls_var);
    
    return 0;
}

/* Compilation recommendations:
   gcc -O3 -fnon-call-exceptions -fvisibility=hidden -ftls-model=initial-exec \
        -fopenmp -fdump-tree-all -fdump-rtl-expand test_targhooks.c -o test
   
   Expected coverage trigger:
   The combination of:
   1. TLS with implicit initialization (potential __tls_init artificial function)
   2. Nested function with setjmp/longjmp (trampoline helpers)
   3. OpenMP target pragmas (offloading stubs)
   4. Weak symbols and external linkage
   5. Nothrow context and volatile usage
   
   Should force GCC to create internal declarations with:
   - TREE_STATIC, TREE_PUBLIC, DECL_EXTERNAL = 1
   - TREE_USED, TREE_THIS_VOLATILE, TREE_NOTHROW = 1  
   - DECL_ARTIFICIAL, DECL_IGNORED_P = 1
   - DECL_VISIBILITY_SPECIFIED = 1, VISIBILITY = HIDDEN
*/
