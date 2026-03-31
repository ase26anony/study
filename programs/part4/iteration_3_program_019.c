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
#include <string.h>

/* External function to force external linkage context */
extern int external_helper(void);
extern volatile int global_counter;

/* Weak attribute to influence DECL_EXTERNAL handling */
int external_helper(void) __attribute__((weak));

/* Thread-local storage with dynamic initialization */
__thread int tls_var = 0;
__thread volatile int tls_counter = 0;

/* Function with nothrow attribute to provide context for TREE_NOTHROW */
static int __attribute__((nothrow)) 
nothrow_helper(int x) {
    return x * 2;
}

/* Complex static initializer with side effects */
static int init_value = (printf("Initializing static...\n"), 42);
static volatile int volatile_init = (rand(), 100);

/* Function that will trigger internal declaration generation */
static void trigger_internal_declaration(void) {
    jmp_buf env;
    volatile int local_volatile = 0;
    
    /* Nested function using GCC extension - often triggers artificial functions */
    __extension__ void nested_func(int val) {
        /* Use setjmp within nested function */
        if (setjmp(env) == 0) {
            /* Access TLS variable */
            tls_var += val;
            tls_counter++;
            
            /* Call external function if available */
            if (external_helper) {
                local_volatile = external_helper();
            }
            
            /* Force non-local jump */
            longjmp(env, 1);
        }
    }
    
    /* Execute the nested function */
    nested_func(10);
    
    /* Complex expression with volatile access */
    local_volatile = (volatile_init += tls_var) + init_value;
    
    printf("Nested function executed, tls_var = %d\n", tls_var);
}

/* Simulate multi-file compilation with extern declaration */
#ifndef NO_EXTERN_DECL
/* Forward declaration that might be external */
extern void potentially_external_function(void);
#endif

/* OpenMP pragma to potentially trigger offloading stubs */
#ifdef _OPENMP
#pragma omp declare target
int target_device_var = 0;
#pragma omp end declare target
#endif

/* Main execution flow */
int main(void) {
    int result = 0;
    
    /* Setup */
    srand(42);
    global_counter = 0;
    
    printf("Program start\n");
    
    /* Loop to ensure execution of triggering constructs */
    for (int i = 0; i < 3; i++) {
        /* Access TLS with non-constant initializer simulation */
        tls_var = nothrow_helper(i) + rand();
        
        /* Trigger the internal declaration generation */
        trigger_internal_declaration();
        
        /* Complex static initialization in loop context */
        static int loop_static = (printf("Loop iteration %d\n", i), 0);
        loop_static += i;
        
        /* Volatile operations to prevent optimization */
        volatile_init ^= loop_static;
        
        result += tls_var + volatile_init;
    }
    
    /* Try-catch simulation for exception handling context */
    {
        /* Simulate try block */
        volatile int try_block_var = 0;
        
        /* Use setjmp for non-local jumps */
        jmp_buf try_env;
        if (setjmp(try_env) == 0) {
            /* Call function that might use longjmp */
            trigger_internal_declaration();
            
            /* Access potentially external function */
            #ifndef NO_EXTERN_DECL
            if (&potentially_external_function) {
                /* This might trigger external linkage handling */
                try_block_var = 1;
            }
            #endif
        } else {
            printf("Non-local jump caught\n");
        }
        
        result += try_block_var;
    }
    
    /* OpenMP region if supported */
    #ifdef _OPENMP
    #pragma omp parallel
    {
        #pragma omp atomic
        target_device_var++;
    }
    printf("OpenMP executed, device var = %d\n", target_device_var);
    #endif
    
    printf("Final result: %d\n", result);
    printf("TLS counter: %d\n", tls_counter);
    
    return result != 0 ? 0 : 1;
}

/* Definition of potentially external function */
#ifndef NO_EXTERN_DECL
void potentially_external_function(void) {
    /* Empty but present to satisfy linker */
}
#endif

/* Definition of external helper */
int external_helper(void) {
    static int counter = 0;
    return counter++;
}

/* Global volatile variable */
volatile int global_counter = 0;

/* Additional complex static initialization */
static struct {
    int a;
    volatile int b;
    void (*func)(void);
} complex_static = {
    .a = (printf("Complex static init\n"), 99),
    .b = (srand(time(NULL)), 77),
    .func = NULL
};
