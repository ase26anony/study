/* test_targhooks.c - Designed to trigger GCC's internal tree initialization
   for artificial, hidden visibility declarations (targhooks.cc lines 981-990) */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <omp.h>

/* External function to force external linkage handling */
extern int external_helper(void);
/* Weak symbol to influence DECL_EXTERNAL/TREE_PUBLIC handling */
int __attribute__((weak)) weak_var = 0;

/* Thread-local storage with dynamic initialization */
__thread int tls_var = 0;
static int init_tls(void) {
    return rand() % 100 + 1;
}

/* Complex static initializer with side effects */
static volatile int static_init = (printf("Initializing static...\n"), 42);

/* Function marked nothrow to provide context for TREE_NOTHROW */
void __attribute__((nothrow)) nested_with_jump(void) {
    jmp_buf env;
    volatile int counter = 0;
    
    /* Nested function using GCC statement expression */
    auto void nested() __attribute__((__gnu_inline__));
    void nested() {
        counter++;
        if (counter < 3) {
            longjmp(env, 1);
        }
    }
    
    if (setjmp(env) == 0) {
        nested();
    }
    
    /* Access TLS with dynamic initializer */
    tls_var = init_tls();
    printf("TLS value: %d\n", tls_var);
}

/* OpenMP target region to generate offloading stubs */
#pragma omp declare target
int target_func(int x) {
    return x * 2;
}
#pragma omp end declare target

/* Multi-file simulation: external function declaration */
int external_helper(void) {
    return weak_var + static_init;
}

int main(void) {
    /* Setup */
    srand(42);
    jmp_buf main_env;
    
    printf("Program start\n");
    
    /* Force compiler to generate internal declarations */
    
    /* 1. Nested function with non-local jump */
    nested_with_jump();
    
    /* 2. TLS access with dynamic initialization */
    __thread int local_tls = (printf("Initializing local TLS...\n"), rand());
    printf("Local TLS: %d\n", local_tls);
    
    /* 3. OpenMP target region */
    int result = 0;
    #pragma omp target map(tofrom: result)
    {
        result = target_func(42);
    }
    printf("OpenMP result: %d\n", result);
    
    /* 4. Complex control flow with volatile */
    volatile int flag = rand() > RAND_MAX/2;
    if (flag) {
        /* Use builtin setjmp/longjmp */
        void *buf = __builtin_alloca(256);
        if (__builtin_setjmp(buf) == 0) {
            printf("First setjmp\n");
            /* Simulate longjmp */
            __builtin_longjmp(buf, 1);
        } else {
            printf("After longjmp\n");
        }
    }
    
    /* 5. Call external function */
    weak_var = external_helper();
    printf("Weak var: %d\n", weak_var);
    
    /* 6. Static initialization with side effects */
    static volatile int late_static = (printf("Late static init\n"), weak_var + tls_var);
    
    /* Observable output */
    printf("Final static_init: %d\n", static_init);
    printf("Final tls_var: %d\n", tls_var);
    printf("Program complete\n");
    
    return 0;
}

/* Compilation recommendations:
   gcc -O3 -fnon-call-exceptions -fvisibility=hidden -ftls-model=initial-exec \
       -fPIC -fopenmp -fdump-tree-all -fdump-rtl-expand test_targhooks.c -o test
   
   Or for nested function support:
   gcc -O3 -fnon-call-exceptions -fvisibility=hidden -ftls-model=initial-exec \
       -fPIC -fopenmp -fdump-tree-original test_targhooks.c -o test
*/
