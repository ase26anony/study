/* test_targhooks.c - Designed to trigger GCC's internal tree initialization
 * for compiler-generated declarations with specific flags:
 * TREE_STATIC=1, TREE_PUBLIC=1, DECL_EXTERNAL=1, TREE_USED=1,
 * TREE_THIS_VOLATILE=1, TREE_NOTHROW=1, DECL_ARTIFICIAL=1,
 * DECL_IGNORED_P=1, DECL_VISIBILITY_SPECIFIED=1, DECL_VISIBILITY=VISIBILITY_HIDDEN
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <pthread.h>

/* External function to force external linkage context */
extern int external_helper(void);
int external_helper(void) {
    return rand() % 100;
}

/* Volatile variable to prevent optimization */
volatile int volatile_trigger = 0;

/* Thread-local storage with dynamic initialization */
__thread int tls_var = 0;

/* Function with nothrow attribute to provide context for TREE_NOTHROW */
void __attribute__((nothrow)) setup_tls(void) {
    /* Dynamic initializer for TLS - may cause compiler to generate init stub */
    tls_var = external_helper() + 1;
}

/* Nested function using setjmp/longjmp - may cause GCC to create static helper */
void test_nested_jump(void) {
    jmp_buf env;
    volatile int counter = 0;
    
    /* Define nested function using GCC extension */
    void __attribute__((noinline)) nested_func(int x) {
        if (x > 0) {
            longjmp(env, x);
        }
    }
    
    if (setjmp(env) == 0) {
        /* First call - will longjmp back */
        nested_func(1);
    } else {
        /* After longjmp */
        counter = 1;
    }
    
    volatile_trigger = counter;
}

/* Complex static initializer with side effects */
static int complex_static = (printf("Initializing complex_static\n"), 42);

/* Weak symbol to simulate multi-file linking */
int __attribute__((weak)) weak_symbol(void) {
    return complex_static;
}

/* OpenMP target region if supported */
#ifdef _OPENMP
void test_offload(void) {
    int host_var = 10;
    #pragma omp target map(tofrom: host_var)
    {
        host_var += 5;
    }
    volatile_trigger += host_var;
}
#endif

/* Main execution flow with all triggering constructs */
int main(void) {
    int result = 0;
    
    /* Seed random for TLS initialization */
    srand(42);
    
    /* 1. Setup TLS with dynamic initialization */
    setup_tls();
    result += tls_var;
    
    /* 2. Execute nested function with non-local jumps */
    test_nested_jump();
    result += volatile_trigger;
    
    /* 3. Use complex static initializer */
    result += complex_static;
    
    /* 4. Use weak symbol */
    result += weak_symbol();
    
    /* 5. OpenMP offloading if available */
    #ifdef _OPENMP
    test_offload();
    #endif
    
    /* 6. Additional volatile operations */
    for (volatile int i = 0; i < 3; i++) {
        result += i;
    }
    
    /* Observable output to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Force use of external helper */
    if (result > 100) {
        printf("External helper: %d\n", external_helper());
    }
    
    return 0;
}
