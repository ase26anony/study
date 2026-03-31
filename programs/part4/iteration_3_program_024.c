/* targhooks_trigger.c - Program to trigger GCC's internal tree initialization */
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

/* External function declaration to influence DECL_EXTERNAL handling */
extern int external_helper(void) __attribute__((weak));

/* Thread-local storage with dynamic initialization */
__thread int tls_var = 0;
static int init_counter = 0;

/* Function with side effects for static initialization */
int init_with_side_effects(void) {
    printf("Initializing with side effects...\n");
    return ++init_counter;
}

/* Static variable with complex initializer */
static int complex_static = (init_with_side_effects(), 42);

/* Volatile variable to prevent optimization */
volatile int volatile_guard = 1;

/* Function marked nothrow to provide context for TREE_NOTHROW */
void __attribute__((nothrow)) nested_function_context(void) {
    jmp_buf env;
    volatile int local_state = 0;
    
    /* Nested function using GCC extension */
    __extension__ void nested_func(void* arg) {
        int* ptr = (int*)arg;
        if (setjmp(env) == 0) {
            *ptr = complex_static + tls_var;
        } else {
            *ptr = rand();  /* Use external function */
        }
    }
    
    int result = 0;
    
    /* Execute nested function logic */
    if (volatile_guard) {
        nested_func(&result);
        
        /* Simulate longjmp scenario */
        if (result == 42) {
            longjmp(env, 1);
        }
    }
    
    printf("Nested function result: %d\n", result);
}

/* Weak external function definition */
int external_helper(void) {
    return rand() % 100;
}

/* Main execution flow */
int main(void) {
    /* Setup phase */
    srand(42);
    jmp_buf main_env;
    
    printf("Program start - targeting targhooks.cc lines 981-990\n");
    
    /* Force TLS initialization with side effects */
    tls_var = external_helper();
    printf("TLS variable initialized to: %d\n", tls_var);
    
    /* Loop to ensure execution of triggering constructs */
    for (int i = 0; i < 3; i++) {
        printf("\nIteration %d:\n", i);
        
        /* Enter nothrow context with nested function */
        nested_function_context();
        
        /* Access complex static variable */
        volatile_guard = complex_static % 2;
        
        /* Use setjmp in main context as well */
        if (setjmp(main_env) == 0) {
            if (i == 1) {
                longjmp(main_env, 1);
            }
        }
        
        /* Force external function call */
        int ext_result = external_helper();
        printf("External helper result: %d\n", ext_result);
    }
    
    /* OpenMP-like region simulation (triggers stub generation) */
    #ifdef _OPENMP
    #pragma omp parallel
    {
        printf("OpenMP thread\n");
    }
    #endif
    
    /* Final observable output */
    printf("\nFinal state:\n");
    printf("  TLS var: %d\n", tls_var);
    printf("  Static init counter: %d\n", init_counter);
    printf("  Complex static: %d\n", complex_static);
    
    return 0;
}

/* Additional file simulation for multi-file compilation context */
#ifdef MULTI_FILE
/* Separate compilation unit simulation */
int external_helper(void) {
    static int counter = 0;
    return counter++ + rand();
}
#endif
