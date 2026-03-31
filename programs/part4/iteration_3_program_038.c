/* 
 * This program is designed to trigger GCC's internal tree initialization
 * for compiler-generated declarations with specific flags:
 * TREE_STATIC=1, TREE_PUBLIC=1, DECL_EXTERNAL=1, TREE_USED=1,
 * TREE_THIS_VOLATILE=1, TREE_NOTHROW=1, DECL_ARTIFICIAL=1,
 * DECL_IGNORED_P=1, DECL_VISIBILITY_SPECIFIED=1, DECL_VISIBILITY=VISIBILITY_HIDDEN
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <pthread.h>

/* External function declaration to influence DECL_EXTERNAL handling */
extern int external_helper(void);
/* Weak attribute to simulate multi-file linking */
extern void __attribute__((weak)) weak_function(void);

/* Thread-local storage with dynamic initialization */
__thread int tls_var = 0;
__thread volatile int tls_volatile = 0;

/* Function marked nothrow to provide context for TREE_NOTHROW */
int __attribute__((nothrow)) nothrow_function(int x) {
    return x * 2;
}

/* Complex static initializer with side effects */
static int static_init = (printf("Initializing static variable\n"), 42);

/* External function definition (simulates multi-file) */
int external_helper(void) {
    return rand() % 100;
}

/* Nested function using setjmp/longjmp (GCC extension) */
void test_nested_function(void) {
    jmp_buf env;
    volatile int counter = 0;
    
    /* Define nested function using GCC's statement expression */
    auto void nested_func(void) __attribute__((__nothrow__));
    
    void nested_func(void) {
        counter++;
        tls_var = external_helper();
        tls_volatile = counter;
        
        if (counter < 3) {
            longjmp(env, counter);
        }
    }
    
    if (setjmp(env) == 0) {
        nested_func();
    } else {
        nested_func();
    }
}

/* Function using OpenMP pragma to generate offloading stubs */
#pragma GCC push_options
#pragma GCC optimize("O2")
void openmp_target_function(void) {
    int arr[100];
    
    #pragma omp target map(tofrom: arr[0:100])
    for (int i = 0; i < 100; i++) {
        arr[i] = i * i;
    }
    
    volatile int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += arr[i];
    }
    printf("OpenMP target sum: %d\n", sum);
}
#pragma GCC pop_options

/* Function with exception handling context */
void exception_context_function(void) {
    volatile int should_jump = 1;
    jmp_buf local_env;
    
    /* Nested function using __builtin_setjmp/__builtin_longjmp */
    __extension__ auto void local_nested(void) {
        tls_var++;
        if (should_jump && tls_var < 5) {
            __builtin_longjmp(local_env, 1);
        }
    };
    
    if (__builtin_setjmp(local_env) == 0) {
        local_nested();
    } else {
        local_nested();
    }
}

/* Main function with observable I/O */
int main(void) {
    /* Setup */
    srand(42);
    printf("Program start\n");
    
    /* Force initialization of TLS variables */
    tls_var = external_helper();
    tls_volatile = static_init;
    
    /* Execute nested function with non-local jumps */
    test_nested_function();
    printf("TLS var value: %d\n", tls_var);
    
    /* Execute nothrow function */
    int result = nothrow_function(tls_var);
    printf("Nothrow function result: %d\n", result);
    
    /* Execute exception context function */
    exception_context_function();
    printf("Exception context completed\n");
    
    /* Execute OpenMP target function if supported */
    #ifdef _OPENMP
    openmp_target_function();
    #else
    printf("OpenMP not enabled\n");
    #endif
    
    /* Access weak function to influence linkage */
    if (&weak_function != NULL) {
        weak_function();
    }
    
    /* Final observable output */
    volatile int final_result = tls_var + tls_volatile + static_init;
    printf("Final result: %d\n", final_result);
    
    return 0;
}

/* Define the weak function */
void __attribute__((weak)) weak_function(void) {
    printf("Weak function called\n");
}

/* Additional static initialization with function call */
static int complex_static = (atexit((void(*)())printf), 99);
