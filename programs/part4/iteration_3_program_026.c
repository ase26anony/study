/* 
 * Program to trigger GCC's internal tree initialization for artificial,
 * hidden visibility declarations with specific flags set.
 * Targets targhooks.cc lines 981-990.
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

/* External function to force external linkage considerations */
extern int external_helper(void);
int external_helper(void) { return rand() % 100; }

/* Thread-local storage with dynamic initialization */
__thread int tls_var = 0;
static int init_tls(void) { 
    return external_helper() + 1; 
}

/* Complex static initializer with side effects */
static volatile int complex_static = (printf("Initializing complex_static\n"), 42);

/* Function marked nothrow to provide context for TREE_NOTHROW */
void __attribute__((nothrow)) nested_function_context(void) {
    jmp_buf env;
    volatile int counter = 0;
    
    /* Nested function using GCC extension - often triggers internal helpers */
    auto void nested_func(void) __attribute__((always_inline));
    
    void nested_func(void) {
        if (counter++ == 0) {
            /* Use builtin setjmp/longjmp - GCC may create internal helpers */
            if (__builtin_setjmp(env) == 0) {
                /* Force compiler to consider non-local jumps */
                volatile int *ptr = &counter;
                *ptr += external_helper();
            }
        }
    }
    
    /* Execute the nested function */
    nested_func();
    
    /* Access TLS with dynamic initializer */
    tls_var = init_tls();
}

/* Weak symbol to influence DECL_EXTERNAL/TREE_PUBLIC handling */
int __attribute__((weak)) weak_function(int x) {
    return x * 2 + external_helper();
}

/* Function with OpenMP pragma - may generate offloading stubs */
#pragma GCC push_options
#pragma GCC optimize("O2")
void omp_target_function(void) {
    volatile int data[10];
    
    #pragma omp target map(tofrom: data[0:10])
    for (int i = 0; i < 10; i++) {
        data[i] = i * i + external_helper();
    }
    
    /* Use the data to prevent optimization */
    printf("OMP data[5] = %d\n", data[5]);
}
#pragma GCC pop_options

/* Multi-file simulation using sections */
static int section_var __attribute__((section(".my_section"))) = 123;

/* Another externally visible function */
int __attribute__((visibility("default"))) public_function(void) {
    return section_var + complex_static;
}

int main(void) {
    /* Setup */
    srand(42);
    jmp_buf main_env;
    
    printf("Starting program to trigger internal declaration generation...\n");
    
    /* Execute nested function context */
    nested_function_context();
    
    /* Access and modify TLS */
    tls_var += weak_function(10);
    printf("TLS variable value: %d\n", tls_var);
    
    /* Execute OMP function if supported */
    #ifdef _OPENMP
    omp_target_function();
    #endif
    
    /* Complex control flow with setjmp */
    volatile int setjmp_result = 0;
    
    if (setjmp(main_env) == 0) {
        /* First time through */
        setjmp_result = external_helper();
        
        /* Force consideration of longjmp path */
        volatile int *volatile ptr = &setjmp_result;
        if (*ptr > 50) {
            /* Simulate potential longjmp - compiler must consider this */
            memcpy(&main_env, &main_env, sizeof(jmp_buf));
        }
    } else {
        setjmp_result = 100;
    }
    
    /* Use all generated values to prevent dead code elimination */
    int result = tls_var + complex_static + setjmp_result + public_function();
    printf("Final result: %d\n", result);
    
    /* Access volatile variable */
    volatile int vol_check = complex_static;
    printf("Volatile check: %d\n", vol_check);
    
    return (result > 0) ? 0 : 1;
}

/* Force generation of initialization functions */
static int __attribute__((used)) force_init = (printf("Force init called\n"), 0);

/* Simulate multi-file scenario with late definition */
int late_defined_function(void);
int late_defined_function(void) {
    return tls_var * 2;
}
