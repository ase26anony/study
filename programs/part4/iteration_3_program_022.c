/* Compile with: gcc -O3 -fPIC -ftls-model=initial-exec -fnon-call-exceptions -fvisibility=hidden -fdump-tree-all -o test_program test_program.c */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <pthread.h>

/* External function declaration to influence DECL_EXTERNAL handling */
extern int external_helper(void) __attribute__((weak));

/* Thread-local storage with dynamic initialization */
__thread int tls_var = 0;
static int init_counter = 0;

/* Function with nothrow attribute to provide context for TREE_NOTHROW */
static int __attribute__((nothrow)) initialize_tls(void) {
    return ++init_counter;
}

/* Complex static initializer with side effects */
static int complex_init = (printf("Initializing complex static...\n"), 42);

/* Function that will trigger compiler-generated helper */
static volatile int trigger_internal_declaration(void) {
    jmp_buf env;
    volatile int result = 0;
    
    /* Nested function using GCC extension - often triggers internal helpers */
    __extension__ void nested_func(int x) {
        if (x > 0) {
            /* Use setjmp within nested function - may require trampoline */
            if (setjmp(env) == 0) {
                /* Access TLS variable */
                tls_var = initialize_tls();
                result = x + tls_var + complex_init;
                
                /* Simulate non-local jump */
                longjmp(env, 1);
            }
        }
    }
    
    /* Execute the nested function */
    nested_func(rand() % 100);
    
    return result;
}

/* Multi-file simulation using weak attribute */
int __attribute__((weak)) external_helper(void) {
    /* This may be overridden in another compilation unit */
    return rand() % 256;
}

/* OpenMP-like structure to potentially trigger offloading stubs */
struct parallel_region {
    void (*fn)(void*);
    void *data;
};

static void parallel_helper(void *arg) {
    volatile int *ptr = (volatile int*)arg;
    *ptr = external_helper();
}

/* Main execution flow with observable I/O */
int main(void) {
    volatile int final_result = 0;
    
    /* Setup */
    srand(42);
    printf("Program start\n");
    
    /* Initialize TLS with dynamic value */
    tls_var = initialize_tls();
    printf("TLS initialized to: %d\n", tls_var);
    
    /* Loop containing the key construct */
    for (int i = 0; i < 3; i++) {
        printf("Iteration %d: ", i);
        
        /* This construct should trigger internal declaration generation */
        volatile int iter_result = trigger_internal_declaration();
        
        /* Access TLS variable in loop */
        tls_var += iter_result % 10;
        
        /* Simulate parallel execution context */
        struct parallel_region region = {
            .fn = parallel_helper,
            .data = &iter_result
        };
        region.fn(region.data);
        
        final_result += iter_result + tls_var;
        printf("result = %d, tls = %d\n", iter_result, tls_var);
    }
    
    /* Observable output that depends on all operations */
    printf("Final result: %d\n", final_result);
    printf("Static init value: %d\n", complex_init);
    printf("Init counter: %d\n", init_counter);
    
    return final_result > 0 ? 0 : 1;
}

/* Additional function in different "linkage context" */
static void __attribute__((constructor)) init_function(void) {
    /* This may create additional internal declarations */
    printf("Constructor called\n");
    complex_init = external_helper();
}
