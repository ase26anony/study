/* 
 * Program designed to trigger GCC's internal tree initialization
 * for compiler-generated declarations with specific flags:
 * TREE_STATIC=1, TREE_PUBLIC=1, DECL_EXTERNAL=1, TREE_THIS_VOLATILE=1,
 * TREE_NOTHROW=1, DECL_ARTIFICIAL=1, DECL_IGNORED_P=1,
 * DECL_VISIBILITY_SPECIFIED=1, DECL_VISIBILITY=VISIBILITY_HIDDEN
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <time.h>

/* External function declaration to influence DECL_EXTERNAL handling */
extern int external_helper(void) __attribute__((weak));

/* Thread-local storage with dynamic initialization */
__thread int tls_var = 0;

/* Function to initialize TLS - forces runtime initialization stub */
static int init_tls(void) {
    return rand() % 100;
}

/* Function marked nothrow to provide context for TREE_NOTHROW */
static void process_data(void) __attribute__((nothrow));

/* Complex static initializer with side effects */
static int complex_init = (printf("Initializing complex static...\n"), 42);

/* Volatile variable to prevent optimization */
volatile int volatile_counter = 0;

/* Multi-file simulation: weak external function */
int external_helper(void) {
    return 123;
}

/* Function using nested function with setjmp/longjmp */
void nested_function_with_jmp(void) {
    jmp_buf env;
    volatile int local_state = 0;
    
    /* Nested function using GCC extension */
    auto void nested_helper(void) __attribute__((nothrow));
    
    void nested_helper(void) {
        local_state = 1;
        /* Access TLS variable to force TLS initialization */
        tls_var = init_tls();
        
        /* Use volatile to prevent dead code elimination */
        volatile_counter++;
        
        /* Non-local jump */
        longjmp(env, 1);
    }
    
    if (setjmp(env) == 0) {
        nested_helper();
    } else {
        /* After longjmp */
        printf("After longjmp, tls_var = %d\n", tls_var);
    }
}

/* Function with OpenMP pragma to generate offloading stubs */
#pragma GCC push_options
#pragma GCC optimize("O2")
void omp_offload_function(void) {
    int arr[100];
    
    #pragma omp target map(tofrom: arr[0:100])
    for (int i = 0; i < 100; i++) {
        arr[i] = i * i + volatile_counter;
    }
    
    /* Use the result to prevent dead code elimination */
    printf("OpenMP result: %d\n", arr[50]);
}
#pragma GCC pop_options

/* Main execution flow */
int main(void) {
    /* Setup */
    srand(time(NULL));
    printf("Program start\n");
    
    /* Initialize TLS (forces runtime initialization stub) */
    tls_var = init_tls();
    printf("TLS initialized: %d\n", tls_var);
    
    /* Call external function (weak linkage) */
    int ext_result = external_helper();
    printf("External helper: %d\n", ext_result);
    
    /* Use complex static initializer */
    printf("Complex init value: %d\n", complex_init);
    
    /* Execute nested function with non-local jumps */
    for (int i = 0; i < 3; i++) {
        nested_function_with_jmp();
        volatile_counter++;
    }
    
    /* Execute OpenMP offloading function */
    #ifdef _OPENMP
    omp_offload_function();
    #endif
    
    /* Final observable output using all constructs */
    printf("Final values - TLS: %d, Volatile: %d, Complex: %d\n", 
           tls_var, volatile_counter, complex_init);
    
    return 0;
}

/* Force generation of additional compiler artifacts */
static void __attribute__((constructor)) init_function(void) {
    printf("Constructor running\n");
    tls_var = rand() % 50;
}

static void __attribute__((destructor)) cleanup_function(void) {
    printf("Destructor running, TLS was: %d\n", tls_var);
}
