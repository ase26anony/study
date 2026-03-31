#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <pthread.h>

/* External function declaration to influence DECL_EXTERNAL handling */
extern int external_helper(void);

/* Weak attribute to simulate multi-file compilation */
int external_helper(void) __attribute__((weak));

/* Thread-local storage with dynamic initialization */
__thread int tls_var = 0;

/* Function with nothrow attribute for TREE_NOTHROW context */
void __attribute__((nothrow)) nested_function_context(void) {
    jmp_buf env;
    volatile int counter = 0;  /* Volatile to prevent optimization */
    
    /* Complex static initializer with side effects */
    static int complex_static = (printf("Static init\n"), 42);
    
    /* Initialize TLS with non-constant value */
    tls_var = rand() % 100;
    
    /* Nested function using GCC extension */
    auto void nested_func(void) __attribute__((nothrow));
    
    void nested_func(void) {
        counter++;
        
        /* Use setjmp/longjmp - often triggers compiler-generated helpers */
        if (setjmp(env) == 0) {
            /* Force compiler to generate setup code */
            if (counter < 3) {
                longjmp(env, 1);
            }
        }
        
        /* Access TLS variable */
        tls_var += complex_static;
    }
    
    /* Execute the nested function */
    nested_func();
    
    /* Call potentially external function */
    if (external_helper) {
        tls_var += external_helper();
    }
}

/* OpenMP target region to generate offloading stubs */
#ifdef _OPENMP
void omp_target_function(void) {
    #pragma omp target map(tofrom: tls_var)
    {
        tls_var *= 2;
    }
}
#endif

/* External helper function definition */
int external_helper(void) {
    return rand() % 50;
}

int main(void) {
    /* Setup as specified */
    srand(42);
    
    /* Initialize jmp_buf */
    printf("Starting program...\n");
    
    /* Execute the key construct multiple times */
    for (int i = 0; i < 5; i++) {
        nested_function_context();
        
        #ifdef _OPENMP
        omp_target_function();
        #endif
        
        /* Observable I/O with derived value */
        printf("Iteration %d, tls_var = %d\n", i, tls_var);
        
        /* Force volatile access */
        volatile int dummy = tls_var;
        (void)dummy;
    }
    
    /* Additional complexity with function pointers */
    void (*func_ptr)(void) = nested_function_context;
    func_ptr();
    
    printf("Program completed successfully.\n");
    return 0;
}
