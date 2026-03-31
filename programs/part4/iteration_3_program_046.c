/* test_targhooks.c - Designed to trigger GCC's internal tree initialization
   for compiler-generated declarations with specific flags:
   TREE_STATIC=1, TREE_PUBLIC=1, DECL_EXTERNAL=1, TREE_USED=1,
   TREE_THIS_VOLATILE=1, TREE_NOTHROW=1, DECL_ARTIFICIAL=1,
   DECL_IGNORED_P=1, DECL_VISIBILITY_SPECIFIED=1, DECL_VISIBILITY=VISIBILITY_HIDDEN
   
   Compile with: gcc -O3 -ftls-model=initial-exec -fPIC -fnon-call-exceptions \
                   -fvisibility=hidden -fdump-tree-all -o test_targhooks test_targhooks.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <pthread.h>

/* External function to force external linkage context */
extern int external_helper(void) __attribute__((weak));

/* Volatile variable to prevent optimization */
volatile int volatile_trigger = 1;

/* Function with nothrow attribute to provide context for TREE_NOTHROW */
static void __attribute__((nothrow)) 
nothrow_context_function(void) {
    /* Empty - provides context for the flag */
}

/* Complex static initializer with side effects */
static int static_with_side_effect = (printf("Initializing static_with_side_effect\n"), 42);

/* TLS with dynamic initialization - may generate initialization stub */
__thread int tls_dynamic = (rand() % 100) + 1;

/* Another TLS variable to increase chance of stub generation */
__thread int tls_secondary = 0;

/* Function that uses nested function with setjmp/longjmp */
void trigger_nested_function(void) {
    jmp_buf env;
    volatile int counter = 0;
    
    /* Nested function using GCC extension */
    __extension__ void nested_func(void) {
        if (counter++ < 3) {
            longjmp(env, 1);
        }
    }
    
    if (setjmp(env) == 0) {
        nested_func();
    }
    
    /* Access TLS to potentially trigger more internal declarations */
    tls_secondary = tls_dynamic * 2;
}

/* Function with OpenMP pragma to potentially generate offloading stubs */
#pragma omp declare target
int target_function(int x) {
    return x * x + 1;
}
#pragma omp end declare target

/* Multi-file simulation using weak attribute */
int __attribute__((weak)) external_helper(void) {
    /* This weak symbol simulates multi-file compilation */
    return tls_dynamic + static_with_side_effect;
}

/* Main function with execution flow as specified */
int main(void) {
    int result = 0;
    
    /* Setup phase */
    printf("Starting test program...\n");
    srand(42);
    
    /* Force initialization of TLS */
    printf("Initial TLS value: %d\n", tls_dynamic);
    
    /* Enter loop with key construct */
    for (int i = 0; i < 2; i++) {
        /* Call function with nested setjmp usage */
        trigger_nested_function();
        
        /* Call nothrow function for context */
        nothrow_context_function();
        
        /* Use volatile to prevent dead code elimination */
        if (volatile_trigger) {
            /* Access and modify TLS */
            tls_dynamic += i;
            printf("Loop %d: TLS = %d\n", i, tls_dynamic);
        }
        
        /* Call weak external function */
        result += external_helper();
    }
    
    /* Use OpenMP target region if supported */
    #ifdef _OPENMP
    #pragma omp target map(tofrom: result)
    {
        result = target_function(result);
    }
    printf("After OpenMP target: %d\n", result);
    #endif
    
    /* Observable I/O with derived value */
    printf("Final result: %d\n", result);
    printf("Static with side effect: %d\n", static_with_side_effect);
    printf("TLS secondary: %d\n", tls_secondary);
    
    /* Additional complexity: function pointer that could trigger
       compiler-generated trampolines */
    void (* volatile fp)(void) = trigger_nested_function;
    if (volatile_trigger) {
        fp();
    }
    
    return 0;
}

/* Additional function in different "linkage context" to influence
   DECL_EXTERNAL and TREE_PUBLIC handling */
void __attribute__((visibility("default"))) 
public_function(void) {
    /* This public function with different visibility may cause
       the compiler to generate hidden counterparts */
    printf("Public function called\n");
}
