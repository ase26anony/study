/* Complex program to trigger GCC's internal tree initialization for
   artificial, hidden visibility declarations with specific flags */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <pthread.h>

/* External function declaration to influence DECL_EXTERNAL handling */
extern int external_helper(void) __attribute__((weak));

/* Forward declaration for multi-file simulation */
static void nested_function_context(void) __attribute__((nothrow));

/* Thread-local storage with dynamic initialization */
__thread int tls_var = 0;
static int init_counter = 0;

/* Function with side effects for TLS initialization */
static int get_init_value(void) {
    return ++init_counter + rand() % 100;
}

/* Complex static initializer with side effects */
static int complex_static = (printf("Initializing complex_static\n"), 42);

/* Volatile variable to prevent optimization */
volatile int volatile_guard = 1;

/* Function that simulates external linkage behavior */
int external_helper(void) {
    return rand() % 256;
}

/* Nested function context using GCC extensions */
static void nested_function_context(void) {
    jmp_buf env;
    int local_counter = 0;
    
    /* Nested function using GCC's statement expression */
    auto void nested_func(void) __attribute__((nothrow));
    
    void nested_func(void) {
        /* Access TLS variable to force TLS initialization */
        tls_var = get_init_value();
        
        /* Use volatile to prevent dead code elimination */
        if (volatile_guard) {
            local_counter++;
        }
        
        /* Potential longjmp target */
        if (local_counter > 3) {
            longjmp(env, 1);
        }
    }
    
    /* Set up non-local jump context */
    if (setjmp(env) == 0) {
        /* Execute nested function multiple times */
        for (int i = 0; i < 5; i++) {
            nested_func();
            
            /* Call external function to influence linkage */
            if (external_helper() > 128) {
                printf("External helper returned high value\n");
            }
        }
    } else {
        printf("longjmp executed\n");
    }
    
    /* Force TLS access in different context */
    printf("TLS value: %d\n", tls_var);
}

/* OpenMP-like parallel region simulation */
static void parallel_region_simulation(void) {
    /* Simulate some parallel computation that might trigger
       internal function generation for synchronization */
    int private_var = 0;
    
    for (int i = 0; i < 10; i++) {
        /* Complex expression with side effects */
        private_var += (printf("Iteration %d\n", i), i * 2);
        
        /* Access TLS in loop */
        tls_var += private_var % 7;
    }
    
    printf("Parallel simulation result: %d\n", private_var);
}

/* Main function with execution flow to trigger all constructs */
int main(void) {
    /* Setup */
    srand(42);
    printf("Program start\n");
    
    /* Initialize TLS with dynamic value */
    tls_var = get_init_value();
    printf("Initial TLS value: %d\n", tls_var);
    
    /* Execute nested function context */
    nested_function_context();
    
    /* Execute parallel simulation */
    parallel_region_simulation();
    
    /* Complex static variable usage */
    printf("Complex static value: %d\n", complex_static + tls_var);
    
    /* Force visibility of all constructs */
    if (volatile_guard) {
        /* Call external helper through function pointer
           to potentially trigger stub generation */
        int (*func_ptr)(void) = external_helper;
        printf("Function pointer result: %d\n", func_ptr());
    }
    
    /* Final TLS access */
    printf("Final TLS value: %d\n", tls_var);
    printf("Init counter: %d\n", init_counter);
    
    return 0;
}

/* Additional function in different "compilation unit" style */
static void __attribute__((constructor)) init_function(void) {
    printf("Constructor called\n");
    /* Force early TLS initialization */
    volatile int dummy = tls_var;
    (void)dummy;
}

/* Weak definition to satisfy linker */
__attribute__((weak)) int external_helper(void) {
    return 123; /* Default implementation */
}
