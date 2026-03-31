/* 
 * This program is designed to trigger GCC's internal tree initialization
 * for compiler-generated declarations with specific flags:
 * TREE_STATIC, TREE_PUBLIC, DECL_EXTERNAL, TREE_USED, TREE_THIS_VOLATILE,
 * TREE_NOTHROW, DECL_ARTIFICIAL, DECL_IGNORED_P, DECL_VISIBILITY_SPECIFIED,
 * and VISIBILITY_HIDDEN.
 *
 * Compile with: gcc -O3 -ftls-model=initial-exec -fPIC -fnon-call-exceptions \
 *                -fvisibility=hidden -fdump-tree-all -o trigger trigger.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

/* External function to force external linkage handling */
extern int external_helper(void) __attribute__((weak));

/* Thread-local storage with dynamic initialization */
__thread volatile int tls_var = 0;

/* Function marked nothrow to provide context for TREE_NOTHROW */
static int __attribute__((nothrow)) 
initialize_tls(void) {
    /* Complex static initializer with side effects */
    static int init_counter = (printf("Initializing TLS\n"), 0);
    
    /* Volatile variable to prevent optimization */
    volatile int seed = rand();
    tls_var = seed % 100;
    return tls_var;
}

/* Function using setjmp/longjmp in nested context */
static void nested_function_with_jmp(void) {
    jmp_buf env;
    volatile int counter = 0;
    
    /* Define a nested function using GCC extension */
    auto void nested_helper(void) __attribute__((nothrow));
    
    void nested_helper(void) {
        if (counter++ < 3) {
            printf("Nested helper: %d\n", counter);
            longjmp(env, 1);
        }
    }
    
    if (setjmp(env) == 0) {
        nested_helper();
    } else {
        /* Access TLS variable to force initialization */
        tls_var += initialize_tls();
    }
}

/* Simulate multi-file compilation with weak external */
int external_helper(void) {
    return rand() % 50;
}

/* Main function with execution flow to trigger all constructs */
int main(void) {
    int result = 0;
    volatile int i;
    
    /* Setup */
    srand(42);
    
    /* Force TLS initialization */
    result = initialize_tls();
    printf("TLS initialized with: %d\n", result);
    
    /* Loop to execute nested function multiple times */
    for (i = 0; i < 5; i++) {
        nested_function_with_jmp();
        
        /* Access external function */
        result += external_helper();
        
        /* Complex static variable with side effects */
        static int complex_static = (printf("Loop iteration %d\n", i), i * 10);
        result += complex_static;
    }
    
    /* Final observable output */
    printf("Final result: %d\n", result);
    
    /* Additional construct: static variable with function call initializer */
    static int finalizer = (printf("Program complete\n"), 0);
    (void)finalizer;
    
    return 0;
}

/* 
 * Additional file simulation using __attribute__((section))
 * This creates separate sections that might trigger internal declarations
 * during linking.
 */
__attribute__((section(".special")))
volatile int special_data = 0xDEADBEEF;

__attribute__((section(".init_array")))
void (*initializer)(void) = (void (*)(void))initialize_tls;
