/* main.c - Multi-file simulation to trigger GCC internal declaration generation */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <pthread.h>

/* External function declaration to influence DECL_EXTERNAL handling */
extern int external_helper(void);

/* Weak attribute to simulate multi-file linking */
int __attribute__((weak)) weak_function(int x) {
    return x * 2;
}

/* Function marked nothrow to provide context for TREE_NOTHROW */
void __attribute__((nothrow)) nothrow_function(void) {
    /* Empty - just for context */
}

/* Complex static initializer with side effects */
static int complex_init = (printf("Initializing complex static\n"), 42);

/* Thread-local storage with dynamic initialization */
__thread int tls_var = 0;

/* Function to initialize TLS dynamically */
int init_tls(void) {
    static int counter = 0;
    return ++counter + rand() % 100;
}

/* Nested function using setjmp/longjmp - GCC may create helper functions */
void trigger_internal_declaration(void) {
    jmp_buf env;
    volatile int trigger_flag = 1;  /* Volatile to prevent optimization */
    
    /* Nested function using GCC extension */
    auto void nested_func(void) __attribute__((nothrow));
    
    void nested_func(void) {
        if (setjmp(env) == 0) {
            /* First call - set up for potential longjmp */
            tls_var = init_tls();  /* Access TLS with dynamic init */
            printf("Nested function called, tls_var = %d\n", tls_var);
            
            /* Force compiler to consider non-local jump context */
            if (trigger_flag && rand() % 2) {
                /* Simulate condition that might longjmp */
                printf("Would longjmp here in real scenario\n");
            }
        } else {
            printf("Returned via longjmp\n");
        }
    }
    
    /* Call the nested function */
    nested_func();
    
    /* Access volatile variable to ensure it's not optimized out */
    printf("Trigger flag: %d\n", trigger_flag);
}

/* OpenMP pragma to potentially generate offloading stubs */
#ifdef _OPENMP
void omp_target_function(void) {
    int arr[100];
    
    #pragma omp target map(tofrom: arr[0:100])
    {
        #pragma omp parallel for
        for (int i = 0; i < 100; i++) {
            arr[i] = i * i;
        }
    }
    
    printf("OpenMP target result: %d\n", arr[50]);
}
#endif

/* External function definition (simulating separate compilation unit) */
int external_helper(void) {
    /* Access thread-local storage */
    static __thread int helper_tls = 0;
    helper_tls++;
    return helper_tls + complex_init;
}

int main(void) {
    /* Setup */
    srand(42);
    
    /* Call nothrow function for context */
    nothrow_function();
    
    /* Use weak function */
    printf("Weak function result: %d\n", weak_function(21));
    
    /* Trigger the key construct multiple times */
    for (int i = 0; i < 3; i++) {
        printf("\nIteration %d:\n", i + 1);
        trigger_internal_declaration();
        
        /* Access and modify TLS */
        tls_var = external_helper();
        printf("TLS var after external helper: %d\n", tls_var);
    }
    
    /* Use complex static initializer */
    printf("\nComplex static value: %d\n", complex_init);
    
    #ifdef _OPENMP
    /* Include OpenMP if available */
    omp_target_function();
    #endif
    
    /* Final observable output */
    volatile int final_result = tls_var + complex_init;
    printf("\nFinal observable result: %d\n", final_result);
    
    return 0;
}

/* Additional file simulation - this would normally be in a separate file */
/* Uncomment to compile separately and link:
// helper.c
#include <stdio.h>
int external_helper(void) {
    return rand() % 1000;
}
*/
