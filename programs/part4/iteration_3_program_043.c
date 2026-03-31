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
void __attribute__((nothrow)) nested_function_context(void) {
    /* Nested function using GCC extension */
    __extension__ void nested_func(int x) {
        jmp_buf env;
        
        /* Use setjmp within nested function - may trigger internal helpers */
        if (setjmp(env) == 0) {
            /* Access TLS variable to force TLS initialization */
            tls_var = external_helper() + x;
            
            /* Force the compiler to consider longjmp path */
            if (volatile_trigger > 100) {
                longjmp(env, 1);
            }
        }
    }
    
    /* Execute the nested function */
    nested_func(42);
}

/* Static variable with complex initializer containing side effects */
static int complex_static = (printf("Initializing complex_static\n"), 
                            external_helper(), 
                            255);

/* Weak symbol to influence DECL_EXTERNAL handling */
__attribute__((weak)) int weak_symbol = 1234;

/* Function that uses OpenMP-like offloading pattern */
#pragma GCC push_options
#pragma GCC optimize("O2")
void target_region_simulation(void) {
    /* Simulate offloading stub generation */
    static int device_data = 0;
    
    /* Nested function that might be transformed */
    auto void device_stub(void) __attribute__((nothrow));
    
    void device_stub(void) {
        /* Complex operations that might require helper functions */
        jmp_buf buf;
        volatile int counter = 0;
        
        if (__builtin_setjmp(buf) == 0) {
            for (int i = 0; i < 10; i++) {
                counter += tls_var + complex_static;
                
                /* Force compiler to generate internal state tracking */
                if (counter > 1000 && volatile_trigger) {
                    __builtin_longjmp(buf, 1);
                }
            }
        }
        
        /* Access weak symbol to influence linkage */
        device_data = weak_symbol + counter;
    }
    
    device_stub();
    
    /* Mark the result as used */
    volatile_trigger = device_data & 1;
}
#pragma GCC pop_options

/* Multi-file simulation using sections */
__attribute__((section(".data.special"))) 
int section_var __attribute__((used)) = 0xDEADBEEF;

/* Another external declaration to influence TREE_PUBLIC/DECL_EXTERNAL */
extern void __attribute__((visibility("hidden"))) hidden_external(void);
void hidden_external(void) {
    /* Empty but affects symbol table */
}

int main(void) {
    /* Setup */
    srand(42);
    jmp_buf main_env;
    
    printf("Starting coverage test...\n");
    
    /* Execute the nested function context */
    nested_function_context();
    
    /* Execute target region simulation */
    target_region_simulation();
    
    /* Force TLS initialization and usage */
    tls_var = external_helper();
    printf("TLS variable value: %d\n", tls_var);
    
    /* Use complex static */
    printf("Complex static value: %d\n", complex_static);
    
    /* Use section variable */
    section_var = tls_var + complex_static;
    printf("Section variable modified: 0x%X\n", section_var);
    
    /* Call hidden external */
    hidden_external();
    
    /* Setjmp/longjmp pattern in main */
    if (setjmp(main_env) == 0) {
        /* Force volatile access */
        if (volatile_trigger == 0) {
            volatile_trigger = 1;
            
            /* Simulate error path that might trigger internal helpers */
            if (rand() % 5 == 0) {
                longjmp(main_env, 1);
            }
        }
    }
    
    /* Final observable output */
    int result = tls_var + complex_static + section_var + volatile_trigger;
    printf("Final result: %d\n", result);
    printf("Test completed.\n");
    
    return result & 0xFF;
}

/* Additional function to influence linkage */
__attribute__((constructor)) 
void init_function(void) {
    /* Force early initialization */
    weak_symbol = external_helper();
}

/* Compilation recommendations:
 * gcc -O3 -ftls-model=initial-exec -fnon-call-exceptions \
 *     -fvisibility=hidden -fPIC -fdump-tree-all \
 *     -o coverage_test coverage_test.c
 * 
 * For OpenMP variant (if supported):
 * gcc -O3 -fopenmp -ftls-model=initial-exec -fvisibility=hidden \
 *     -fdump-rtl-expand -o coverage_test_omp coverage_test.c
 */
