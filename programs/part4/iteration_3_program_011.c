/* 
 * This program is designed to trigger GCC's internal tree initialization
 * for compiler-generated declarations with specific flags:
 * TREE_STATIC, TREE_PUBLIC, DECL_EXTERNAL, TREE_USED, TREE_THIS_VOLATILE,
 * TREE_NOTHROW, DECL_ARTIFICIAL, DECL_IGNORED_P, DECL_VISIBILITY_SPECIFIED,
 * and VISIBILITY_HIDDEN.
 *
 * Compile with: gcc -O3 -fPIC -ftls-model=initial-exec -fnon-call-exceptions \
 *                -fvisibility=hidden -fdump-tree-all -o trigger trigger.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <pthread.h>

/* External function to force external linkage context */
extern int external_helper(void) __attribute__((weak));

/* Volatile variable to prevent optimization */
volatile int global_volatile = 0;

/* Thread-local storage with dynamic initialization */
__thread int tls_var = 0;

/* Function to initialize TLS dynamically */
int init_tls(void) {
    return rand() % 100 + 1;
}

/* Nested function using setjmp/longjmp (GCC extension) */
void nested_function_with_jmp(void) {
    jmp_buf env;
    volatile int counter = 0;
    
    /* Define nested function using GCC's statement expression */
    auto void nested_helper(void) __attribute__((nothrow));
    
    void nested_helper(void) {
        counter++;
        if (counter < 3) {
            longjmp(env, 1);
        }
    }
    
    if (setjmp(env) == 0) {
        nested_helper();
    } else {
        nested_helper();
    }
    
    global_volatile = counter;
}

/* Function with complex static initializer */
void function_with_complex_static(void) {
    static int complex_static = (printf("Initializing complex static\n"), 
                                 rand() % 50 + 1);
    
    /* Use volatile to ensure side effects aren't optimized away */
    volatile int result = complex_static + tls_var;
    (void)result;
}

/* Function marked nothrow containing the key constructs */
void __attribute__((nothrow)) trigger_function(void) {
    /* Initialize TLS with dynamic value */
    tls_var = init_tls();
    
    /* Use nested function with non-local jumps */
    nested_function_with_jmp();
    
    /* Complex static initialization */
    function_with_complex_static();
    
    /* Call external function if available */
    if (external_helper) {
        tls_var += external_helper();
    }
}

/* Simulate multi-file compilation with weak external function */
int external_helper(void) {
    return 42;
}

/* Main function with setup and execution */
int main(void) {
    /* Setup */
    srand(42);
    jmp_buf main_env;
    
    printf("Starting program to trigger internal declaration generation...\n");
    
    /* Execute the triggering function multiple times */
    for (int i = 0; i < 5; i++) {
        trigger_function();
        
        /* Use setjmp/longjmp in main to add complexity */
        if (setjmp(main_env) == 0) {
            if (i % 2 == 0) {
                longjmp(main_env, 1);
            }
        }
        
        /* Access TLS variable */
        printf("Iteration %d: tls_var = %d, global_volatile = %d\n", 
               i, tls_var, global_volatile);
        
        /* Modify TLS */
        tls_var += i;
    }
    
    /* Final observable output */
    printf("Final TLS value: %d\n", tls_var);
    printf("Program completed successfully.\n");
    
    return 0;
}
