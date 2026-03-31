/* test_targhooks.c - Designed to trigger GCC's internal tree initialization */
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

/* External function to force external linkage considerations */
extern int external_helper(void);
int external_helper(void) { return rand() % 100; }

/* Thread-local storage with dynamic initialization */
__thread int tls_var = 0;
static int init_tls(void) {
    tls_var = external_helper() + 1;
    return tls_var;
}

/* Function marked nothrow to provide context for TREE_NOTHROW */
static void process_data(void) __attribute__((nothrow));

/* Complex static initializer with side effects */
static volatile int complex_static = (printf("Initializing complex static...\n"), 
                                     external_helper(), 42);

/* Nested function using setjmp/longjmp for non-local jumps */
void trigger_internal_declaration(void) {
    jmp_buf env;
    volatile int counter = 0;
    
    /* Nested function using GCC extension */
    auto void nested_func(void) __attribute__((nothrow));
    
    void nested_func(void) {
        if (setjmp(env) == 0) {
            /* First call - setup */
            counter++;
            
            /* Access TLS variable to force initialization */
            if (tls_var == 0) {
                tls_var = init_tls();
            }
            
            /* Use complex static */
            counter += complex_static;
            
            /* Simulate error and longjmp */
            if (counter > 40) {
                longjmp(env, 1);
            }
        } else {
            /* After longjmp */
            printf("After longjmp, tls_var = %d\n", tls_var);
        }
    }
    
    /* Execute nested function */
    nested_func();
    
    /* Force another execution path */
    if (counter > 0) {
        longjmp(env, 1);
    }
}

/* Multi-file simulation using weak attribute */
__attribute__((weak)) void weak_function(void) {
    printf("Weak function called\n");
}

/* Main function with observable I/O */
int main(void) {
    int result = 0;
    
    /* Setup */
    srand(42);
    printf("Program start\n");
    
    /* Force initialization of complex static */
    result += complex_static;
    
    /* Call weak function to influence linkage */
    weak_function();
    
    /* Trigger the key construct multiple times */
    for (int i = 0; i < 3; i++) {
        printf("Iteration %d: ", i);
        trigger_internal_declaration();
        
        /* Modify TLS to force re-initialization consideration */
        tls_var += external_helper();
    }
    
    /* Observable output */
    printf("Final result: %d, TLS value: %d\n", result, tls_var);
    
    /* Additional construct: static variable with function call initializer */
    static int func_init_var = (printf("Function-initialized static\n"), 
                               external_helper());
    result += func_init_var;
    
    printf("Program end with value: %d\n", result);
    return 0;
}

/* Simulate multi-file compilation by having unused external declarations */
#ifdef SIMULATE_MULTIFILE
/* In a real multi-file scenario, this would be in another source file */
int unused_external_function(void) {
    return 0;
}
#endif
