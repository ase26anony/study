/* 
 * Program designed to trigger GCC's internal tree initialization
 * for compiler-generated declarations with specific flags:
 * TREE_STATIC=1, TREE_PUBLIC=1, DECL_EXTERNAL=1, TREE_USED=1,
 * TREE_THIS_VOLATILE=1, TREE_NOTHROW=1, DECL_ARTIFICIAL=1,
 * DECL_IGNORED_P=1, DECL_VISIBILITY_SPECIFIED=1, VISIBILITY_HIDDEN
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

/* External function to force external linkage considerations */
extern int external_helper(void) __attribute__((weak));

/* Volatile variable to prevent optimization */
volatile int global_counter = 0;

/* Thread-local storage with dynamic initialization */
__thread int tls_var = 0;

/* Function with side effects for TLS initialization */
int init_tls(void) __attribute__((noinline));
int init_tls(void) {
    return rand() % 100 + 1;
}

/* Function marked nothrow to provide context for TREE_NOTHROW */
void process_data(void) __attribute__((nothrow));

/* Simulate multi-file compilation with extern declaration */
extern void hidden_helper(void);

/* Actual definition with hidden visibility */
void hidden_helper(void) __attribute__((visibility("hidden"))) {
    /* Complex static initializer with side effects */
    static int complex_static = (printf("Complex static init\n"), 42);
    
    /* Access TLS variable */
    tls_var += complex_static;
}

/* Nested function using setjmp/longjmp */
void nested_function_test(void) {
    jmp_buf env;
    volatile int local_state = 0;
    
    /* Define nested function using GCC extension */
    auto void nested_func(void) __attribute__((nothrow));
    
    void nested_func(void) {
        if (setjmp(env) == 0) {
            /* First call - setup */
            local_state = 1;
            
            /* Access TLS with dynamic initializer */
            __thread int local_tls = init_tls();
            tls_var = local_tls;
            
            /* Call external/hidden function */
            hidden_helper();
            
            /* Force longjmp to create complex control flow */
            longjmp(env, 1);
        } else {
            /* After longjmp */
            local_state = 2;
            
            /* Modify TLS variable */
            tls_var *= 2;
        }
    }
    
    /* Execute the nested function */
    nested_func();
    
    /* Use the result to prevent dead code elimination */
    global_counter += tls_var + local_state;
}

/* OpenMP-like offloading simulation */
#pragma GCC push_options
#pragma GCC optimize("O0")
void offload_simulation(void) {
    /* Static variable with complex initializer */
    static int offload_data = (printf("Offload init\n"), 0);
    
    /* Simulate device memory allocation */
    volatile int* device_ptr = (volatile int*)malloc(sizeof(int) * 10);
    
    if (device_ptr) {
        /* Initialize with pattern */
        for (int i = 0; i < 10; i++) {
            device_ptr[i] = i * tls_var;
        }
        
        /* Use the data */
        offload_data = device_ptr[5];
        
        free((void*)device_ptr);
    }
    
    global_counter += offload_data;
}
#pragma GCC pop_options

/* Main execution flow */
int main(void) {
    /* Setup */
    srand(42);
    jmp_buf main_env;
    
    printf("Starting program to trigger GCC internal declarations\n");
    
    /* First: Test with nested functions and TLS */
    nested_function_test();
    
    /* Second: Test with complex static initialization */
    offload_simulation();
    
    /* Third: Create situation requiring compiler-generated trampoline */
    {
        /* Function pointer that might need compiler helper */
        void (*func_ptr)(void) = hidden_helper;
        
        /* Volatile to prevent optimization */
        volatile int use_func = rand() % 2;
        
        if (use_func) {
            func_ptr();
        }
    }
    
    /* Fourth: Exception-like control flow with setjmp */
    if (setjmp(main_env) == 0) {
        /* Normal execution path */
        
        /* Re-initialize TLS with function call */
        __thread int main_tls = init_tls();
        
        /* Force compiler to generate initialization code */
        tls_var = main_tls * 2;
        
        /* Simulate error condition and longjmp */
        if (tls_var > 50) {
            longjmp(main_env, 1);
        }
    } else {
        /* Error recovery path */
        printf("Recovered from longjmp\n");
        
        /* Access and modify TLS */
        tls_var /= 2;
    }
    
    /* Final output to ensure all code paths are used */
    printf("Final values: global_counter=%d, tls_var=%d\n", 
           global_counter, tls_var);
    
    /* Try to call weak external function if available */
    if (external_helper) {
        global_counter += external_helper();
    }
    
    return global_counter > 0 ? 0 : 1;
}

/* Weak external function definition (simulates multi-file) */
int external_helper(void) {
    /* Static initialization with side effect */
    static int counter = (printf("External helper init\n"), 0);
    return ++counter;
}

/* Additional complex construct: static variable in inline function */
static inline void inline_with_static(void) {
    /* This may cause GCC to generate static initialization guard */
    static int guard_initialized = 0;
    
    if (!guard_initialized) {
        guard_initialized = 1;
        tls_var += 100;
    }
}

/* Force the inline function to be used */
void use_inline_function(void) {
    inline_with_static();
}
