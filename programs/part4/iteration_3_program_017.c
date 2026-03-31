/* test_targhooks.c - Designed to trigger GCC's internal tree initialization
   for compiler-generated declarations with specific flags:
   TREE_STATIC=1, TREE_PUBLIC=1, DECL_EXTERNAL=1, TREE_USED=1,
   TREE_THIS_VOLATILE=1, TREE_NOTHROW=1, DECL_ARTIFICIAL=1,
   DECL_IGNORED_P=1, DECL_VISIBILITY_SPECIFIED=1, DECL_VISIBILITY=VISIBILITY_HIDDEN
*/

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <pthread.h>

/* External function to force external linkage context */
extern int external_helper(void);
int external_helper(void) { return rand() % 100; }

/* Volatile variable to prevent optimization */
volatile int volatile_trigger = 0;

/* Thread-local storage with dynamic initialization */
__thread int tls_var = 0;

/* Function with nothrow attribute to provide context for TREE_NOTHROW */
void __attribute__((nothrow)) nested_function_context(void) {
    jmp_buf env;
    volatile int counter = 0;
    
    /* Nested function using GCC statement expression extension */
    auto void nested(void) __attribute__((__nothrow__));
    
    void nested(void) {
        counter++;
        if (counter < 3) {
            /* Use builtin longjmp to force compiler-generated helper */
            __builtin_longjmp(env, 1);
        }
    }
    
    if (__builtin_setjmp(env) == 0) {
        /* First call - will trigger nested function setup */
        nested();
    } else {
        /* After longjmp - force TLS access with side effects */
        tls_var = external_helper() + volatile_trigger;
    }
}

/* Static variable with complex initializer containing function call */
static int complex_static = (printf("Initializing complex_static\n"), 
                             external_helper(), 
                             42);

/* Weak symbol to influence DECL_EXTERNAL/TREE_PUBLIC handling */
int __attribute__((weak)) weakly_defined = 0;

/* OpenMP pragma to potentially generate offloading stubs */
#pragma omp declare target
int target_device_var = 0;
#pragma omp end declare target

/* Function with exception handling context (simulated for C) */
void exception_context_function(void) {
    /* Simulate try/catch with setjmp */
    jmp_buf try_env;
    volatile int exception_caught = 0;
    
    if (__builtin_setjmp(try_env) == 0) {
        /* "Try" block - use nested function with non-local jump */
        auto void try_block(void);
        
        void try_block(void) {
            /* Access TLS with side effect */
            tls_var += complex_static;
            
            /* Force artificial function generation through longjmp */
            if (tls_var > 100) {
                __builtin_longjmp(try_env, 1);
            }
        }
        
        try_block();
    } else {
        /* "Catch" block */
        exception_caught = 1;
        volatile_trigger = 1;
    }
    
    /* Use the result to prevent dead code elimination */
    weakly_defined += exception_caught;
}

int main(void) {
    /* Setup */
    srand(42);
    volatile_trigger = rand() % 2;
    
    printf("Starting test program...\n");
    printf("Initial complex_static value: %d\n", complex_static);
    printf("Initial weakly_defined value: %d\n", weakly_defined);
    
    /* Execute key constructs to trigger internal declaration generation */
    
    /* 1. Nested function with setjmp/longjmp */
    nested_function_context();
    
    /* 2. Exception handling context */
    exception_context_function();
    
    /* 3. TLS access with dynamic behavior */
    tls_var = external_helper() + volatile_trigger;
    printf("TLS variable after operations: %d\n", tls_var);
    
    /* 4. OpenMP target region to potentially generate stubs */
    #pragma omp target if(0)  /* if(0) ensures it compiles without OpenMP runtime */
    {
        target_device_var = tls_var;
    }
    
    /* 5. Complex static initialization with side effects */
    static int second_static = (printf("Initializing second_static\n"), 
                                tls_var * 2);
    
    /* Use all variables to prevent optimization */
    int result = complex_static + tls_var + weakly_defined + 
                 second_static + volatile_trigger;
    
    printf("Final result: %d\n", result);
    printf("Test completed.\n");
    
    return result != 0 ? 0 : 1;  /* Ensure non-zero exit if everything optimized away */
}

/* Compilation recommendations:
   gcc -O3 -fPIC -ftls-model=initial-exec -fnon-call-exceptions \
       -fvisibility=hidden -fopenmp -fdump-tree-all \
       -o test_targhooks test_targhooks.c
       
   Additional flags for analysis:
   -fdump-rtl-expand -fdump-tree-original -fdump-tree-optimized
*/
