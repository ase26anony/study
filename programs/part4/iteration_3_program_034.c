/* Complex program to trigger GCC internal declaration generation with
   specific tree flags: STATIC, PUBLIC, EXTERNAL, VOLATILE, NOTHROW,
   ARTIFICIAL, IGNORED, VISIBILITY_HIDDEN */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <pthread.h>

/* External function declaration to influence DECL_EXTERNAL handling */
extern int external_helper(void);
extern volatile int global_counter;

/* Weak symbol to simulate multi-file compilation */
int __attribute__((weak)) weak_function(int x) {
    return x * 2;
}

/* Function marked nothrow to provide context for TREE_NOTHROW */
void __attribute__((nothrow)) safe_operation(void) {
    /* Thread-local storage with dynamic initialization */
    static __thread int tls_var = 0;
    static int init_counter = 0;
    
    /* Dynamic TLS initialization - may trigger compiler-generated init function */
    if (init_counter++ == 0) {
        tls_var = rand() % 100;  /* Non-constant initializer */
    }
    
    /* Complex static initializer with side effects */
    static volatile int complex_static = (printf("Initializing complex_static\n"), 42);
    
    /* Nested function using GCC extension - often triggers internal helpers */
    auto void nested_func(void) __attribute__((always_inline));
    
    void nested_func(void) {
        jmp_buf env;
        volatile int local_volatile = complex_static;
        
        /* Use setjmp within nested function - may create trampoline */
        if (__builtin_setjmp(env) == 0) {
            tls_var += local_volatile;
            /* Simulate non-local jump context */
            if (tls_var > 50) {
                /* longjmp would go here in real usage */
                tls_var = 50;
            }
        }
    }
    
    /* Execute the nested function */
    nested_func();
    
    /* Use the TLS variable to prevent optimization */
    printf("TLS value: %d\n", tls_var);
}

/* Function that uses OpenMP-like constructs (simulated) */
#pragma GCC visibility push(hidden)
static void hidden_helper(void) {
    /* This function itself has hidden visibility */
    volatile int temp = rand();
    printf("Hidden helper: %d\n", temp);
}
#pragma GCC visibility pop

/* Main execution with multiple triggering patterns */
int main(void) {
    jmp_buf main_env;
    volatile int result = 0;
    
    /* Seed RNG for TLS initialization */
    srand(42);
    
    /* Setup for potential non-local jumps */
    if (setjmp(main_env) == 0) {
        /* Loop to ensure execution of triggering constructs */
        for (int i = 0; i < 3; i++) {
            printf("Iteration %d:\n", i);
            
            /* Call nothrow function containing nested function and TLS */
            safe_operation();
            
            /* Call hidden visibility function */
            hidden_helper();
            
            /* Use weak function to influence linkage */
            result += weak_function(i);
            
            /* Complex static initialization with external reference */
            static int counter = (printf("Counter init\n"), 
                                 external_helper ? 0 : 1);
            counter++;
            result += counter;
            
            /* Volatile access to prevent optimization */
            global_counter = result;
        }
        
        /* Simulate condition that might trigger longjmp in real scenario */
        if (result > 100) {
            /* longjmp(main_env, 1); */  /* Commented to avoid actual jump */
            printf("Would longjmp here\n");
        }
    }
    
    /* Final observable output */
    printf("Final result: %d\n", result);
    
    /* Additional construct: static variable with function call initializer */
    static int finalizer = (printf("Program finalizing\n"), 0);
    (void)finalizer;
    
    return 0;
}

/* Dummy definitions for external references */
int external_helper(void) {
    return rand() % 10;
}

volatile int global_counter = 0;

/* Simulated OpenMP target region using attribute */
__attribute__((target("arch=x86-64")))
void target_function(void) {
    /* This might generate offloading stubs */
    volatile double computation = 3.14159 * global_counter;
    printf("Target computation: %f\n", computation);
}
