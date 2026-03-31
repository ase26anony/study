/* Complex program to trigger GCC's internal tree initialization flags */
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

/* External function declaration to influence DECL_EXTERNAL handling */
extern int external_helper(void) __attribute__((weak));

/* Thread-local storage with dynamic initialization */
__thread int tls_var = 0;
static int init_counter = 0;

/* Function with side effects for TLS initialization */
static int get_initial_value(void) {
    init_counter++;
    return rand() % 100 + 1;
}

/* Another TLS variable with non-constant initializer */
__thread int tls_dynamic = 0;

/* Static variable with complex initializer containing side effects */
static int complex_static = (printf("Initializing complex_static\n"), 42);

/* Function marked nothrow to provide context for TREE_NOTHROW */
static void process_data(void) __attribute__((nothrow));

/* Global jmp_buf for non-local jumps */
static jmp_buf jump_buffer;
static volatile int jump_flag = 0;

/* External function simulation */
int external_helper(void) {
    return complex_static + tls_var;
}

/* Function using nested function with setjmp/longjmp */
static void nested_function_example(void) {
    volatile int local_state = 0;
    
    /* GCC nested function using statement expression */
    auto void nested_helper(void) __attribute__((__gnu_inline__));
    
    void nested_helper(void) {
        local_state = 1;
        
        if (setjmp(jump_buffer) == 0) {
            /* First time through */
            tls_var = get_initial_value();
            printf("Nested function set TLS to: %d\n", tls_var);
            
            /* Force a longjmp to trigger compiler-generated cleanup */
            if (jump_flag == 0) {
                jump_flag = 1;
                longjmp(jump_buffer, 1);
            }
        } else {
            /* After longjmp */
            tls_dynamic = external_helper();
            printf("After longjmp, tls_dynamic: %d\n", tls_dynamic);
        }
    }
    
    nested_helper();
}

/* Function with OpenMP-like offloading pattern */
static void parallel_region_simulation(void) {
    volatile int shared = 0;
    
    /* Simulate offloading stub generation */
    #pragma GCC push_options
    #pragma GCC optimize("O0")
    {
        /* Complex volatile operations */
        volatile int *ptr = &shared;
        *ptr = complex_static;
        
        /* Access TLS in volatile context */
        volatile int tls_temp = tls_var;
        tls_temp += tls_dynamic;
        
        printf("Parallel simulation result: %d\n", tls_temp);
    }
    #pragma GCC pop_options
}

/* Main execution flow */
int main(void) {
    /* Setup */
    srand(42);
    printf("Program start, init_counter: %d\n", init_counter);
    
    /* Initialize TLS with dynamic value */
    tls_dynamic = get_initial_value();
    printf("Initial tls_dynamic: %d\n", tls_dynamic);
    
    /* Execute nested function with non-local jumps */
    nested_function_example();
    
    /* Execute parallel simulation */
    parallel_region_simulation();
    
    /* Loop with varying conditions to prevent dead code elimination */
    for (int i = 0; i < 3; i++) {
        volatile int loop_var = i;
        
        if (loop_var % 2 == 0) {
            /* Access TLS in loop */
            tls_var += external_helper();
            printf("Loop iteration %d, tls_var: %d\n", i, tls_var);
        } else {
            /* Use complex static */
            complex_static += tls_dynamic;
            printf("Loop iteration %d, complex_static: %d\n", i, complex_static);
        }
        
        /* Force compiler to consider exception-like paths */
        if (loop_var == 2) {
            jump_flag = 0;
            if (setjmp(jump_buffer) == 0) {
                printf("Final setjmp context established\n");
            }
        }
    }
    
    /* Final observable output */
    printf("Final values - tls_var: %d, tls_dynamic: %d, complex_static: %d, init_counter: %d\n",
           tls_var, tls_dynamic, complex_static, init_counter);
    
    return 0;
}

/* Define the weak external function */
int external_helper(void) {
    /* This creates DECL_EXTERNAL handling during compilation */
    return tls_var * 2 + complex_static;
}

/* Additional function to create more tree nodes */
static void __attribute__((noinline)) 
create_more_internals(void) {
    /* Static with side-effect initializer */
    static int hidden_counter = (printf("Hidden counter init\n"), 0);
    
    /* Volatile access pattern */
    volatile int temp = hidden_counter;
    hidden_counter += tls_var;
    
    /* This should remain as dead code but influences tree building */
    if (0) {
        /* Force creation of internal labels/jump targets */
        goto hidden_label;
    hidden_label:
        temp = 1;
    }
}
