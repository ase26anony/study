/* 
 * This program is designed to trigger GCC's internal tree initialization
 * for compiler-generated declarations with specific flags:
 * TREE_STATIC=1, TREE_PUBLIC=1, DECL_EXTERNAL=1, TREE_USED=1,
 * TREE_THIS_VOLATILE=1, TREE_NOTHROW=1, DECL_ARTIFICIAL=1,
 * DECL_IGNORED_P=1, DECL_VISIBILITY_SPECIFIED=1, VISIBILITY_HIDDEN
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <pthread.h>

/* External function declaration to influence DECL_EXTERNAL handling */
extern int external_helper(void);
/* Weak attribute to simulate multi-file compilation */
extern void __attribute__((weak)) weak_function(void);

/* Thread-local storage with dynamic initialization */
__thread int tls_var = 0;
__thread volatile int tls_volatile = 0;

/* Function with nothrow attribute to provide context for TREE_NOTHROW */
static int __attribute__((nothrow)) 
nothrow_helper(int x) {
    return x * 2;
}

/* Complex static initializer with side effects */
static int static_init = (printf("Initializing static variable\n"), 42);

/* External function definition (simulates multi-file) */
int external_helper(void) {
    return rand() % 100;
}

/* Weak function definition */
void weak_function(void) {
    printf("Weak function called\n");
}

/* Function using nested function with setjmp/longjmp */
static void test_nested_function(void) {
    jmp_buf env;
    volatile int counter = 0;
    
    /* Nested function using GCC extension */
    __extension__ void nested_func(void) {
        counter++;
        if (counter < 3) {
            longjmp(env, 1);
        }
    }
    
    if (setjmp(env) == 0) {
        nested_func();
    } else {
        printf("Jumped back, counter = %d\n", counter);
        nested_func();
    }
}

/* Function using TLS with complex access pattern */
static void test_tls(void) {
    /* Dynamic TLS initialization */
    tls_var = external_helper();
    tls_volatile = nothrow_helper(tls_var);
    
    /* Force TLS access through volatile pointer */
    volatile int *volatile ptr = &tls_volatile;
    printf("TLS value: %d\n", *ptr);
}

/* Function with OpenMP-like offloading pattern */
static void test_parallel_pattern(void) {
    int i;
    volatile int result = 0;
    
    /* Pattern that might trigger internal stub generation */
    for (i = 0; i < 10; i++) {
        /* Use both TLS and external function */
        tls_var += external_helper();
        
        /* Volatile access prevents optimization */
        result += tls_volatile;
        
        /* Nothrow function call */
        result += nothrow_helper(i);
    }
    
    printf("Parallel pattern result: %d\n", result);
}

/* Main function with execution flow to trigger all patterns */
int main(void) {
    /* Setup */
    srand(42);
    printf("Program start\n");
    
    /* Access static initializer */
    printf("Static init value: %d\n", static_init);
    
    /* Test nested function with non-local jumps */
    test_nested_function();
    
    /* Test TLS with dynamic initialization */
    test_tls();
    
    /* Test parallel/offloading-like pattern */
    test_parallel_pattern();
    
    /* Call weak function if available */
    if (weak_function) {
        weak_function();
    }
    
    /* Complex final computation using all patterns */
    {
        jmp_buf final_env;
        volatile int final_result = 0;
        
        __extension__ void final_nested(void) {
            tls_var = external_helper();
            tls_volatile = nothrow_helper(tls_var);
            final_result = tls_var + tls_volatile + static_init;
        }
        
        if (setjmp(final_env) == 0) {
            final_nested();
            /* Simulate longjmp */
            longjmp(final_env, 1);
        }
        
        printf("Final result: %d\n", final_result);
    }
    
    printf("Program end\n");
    return 0;
}
