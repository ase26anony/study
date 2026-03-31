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
/* Weak attribute to simulate multi-file compilation */
extern int maybe_defined_elsewhere(void) __attribute__((weak));

/* Thread-local storage with dynamic initialization - may trigger TLS init stubs */
__thread int tls_var = 0;
__thread volatile int tls_volatile = 0;

/* Function that will be marked nothrow */
static int nested_with_jump(void) __attribute__((nothrow));

/* Complex static initializer with side effects */
static int static_init = (printf("Initializing static variable\n"), 42);

/* Function using GCC nested function extension with setjmp */
static int nested_with_jump(void) {
    jmp_buf env;
    volatile int counter = 0; /* Prevent optimization */
    int result = 0;
    
    /* Nested function using GCC extension */
    auto void nested_func(void) __attribute__((nothrow));
    
    void nested_func(void) {
        counter++;
        if (counter == 1) {
            longjmp(env, 1);
        }
        /* Access TLS to potentially trigger TLS helper generation */
        tls_var = rand() % 100;
        tls_volatile = tls_var;
    }
    
    if (setjmp(env) == 0) {
        nested_func(); /* First call will longjmp */
    } else {
        nested_func(); /* Second call will execute normally */
        result = tls_var;
    }
    
    return result;
}

/* Function with OpenMP pragma to potentially generate offloading stubs */
static void omp_target_example(void) {
    int arr[10] = {0};
    volatile int sum = 0;
    
    #pragma omp target map(tofrom: arr)
    for (int i = 0; i < 10; i++) {
        arr[i] = i * 2;
    }
    
    for (int i = 0; i < 10; i++) {
        sum += arr[i];
    }
    
    printf("OpenMP target sum: %d\n", sum);
}

/* External helper function definition */
int external_helper(void) {
    return rand() % 256;
}

/* Weak function definition */
int maybe_defined_elsewhere(void) {
    return 1;
}

int main(void) {
    int result = 0;
    volatile int prevent_optimization = 0;
    
    /* Seed random for TLS initialization */
    srand(42);
    
    printf("Program start\n");
    printf("Static init value: %d\n", static_init);
    
    /* Execute nested function with non-local jump */
    result = nested_with_jump();
    printf("Nested function result: %d\n", result);
    
    /* Access and modify TLS variables */
    tls_var = external_helper();
    tls_volatile = tls_var * 2;
    printf("TLS var: %d, TLS volatile: %d\n", tls_var, tls_volatile);
    
    /* Call weak function */
    if (&maybe_defined_elsewhere) {
        prevent_optimization = maybe_defined_elsewhere();
    }
    
    /* Execute OpenMP target region if supported */
    #ifdef _OPENMP
    omp_target_example();
    #else
    printf("OpenMP not enabled\n");
    #endif
    
    /* Complex expression with volatile to ensure execution */
    prevent_optimization += (printf("Final output: %d\n", 
                                   result + tls_var + prevent_optimization), 0);
    
    return 0;
}
