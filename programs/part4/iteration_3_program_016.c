/* 
 * Program to trigger GCC's internal tree initialization for artificial,
 * hidden visibility declarations with specific flags set.
 * Compile with: gcc -O3 -ftls-model=initial-exec -fPIC -fnon-call-exceptions -fvisibility=hidden -fdump-tree-all -o trigger trigger.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <pthread.h>

/* External function declaration to influence DECL_EXTERNAL handling */
extern int external_helper(void) __attribute__((weak));

/* Thread-local storage with dynamic initialization */
__thread volatile int tls_var = 0;
static int init_counter = 0;

/* Function to provide dynamic initialization for TLS */
static int __attribute__((noinline)) get_init_value(void) {
    return rand() % 100 + 1;
}

/* Nested function context structure */
struct nested_ctx {
    jmp_buf env;
    volatile int state;
};

/* Function marked nothrow to provide context for TREE_NOTHROW flag */
static void __attribute__((nothrow)) 
complex_initialization_routine(struct nested_ctx *ctx) {
    /* Static variable with side-effect initialization */
    static int static_counter = (printf("Static init executed\n"), 0);
    
    /* Volatile to prevent optimization */
    volatile int local_val = 0;
    
    /* Nested function using GCC extension */
    auto void nested_func(void) __attribute__((always_inline));
    
    void nested_func(void) {
        if (ctx->state == 0) {
            /* Use setjmp within nested function */
            if (__builtin_setjmp(ctx->env) == 0) {
                ctx->state = 1;
                /* Force compiler to generate helper */
                tls_var = get_init_value();
                local_val = tls_var;
                
                /* Complex expression that might need compiler-generated code */
                ctx->state += (local_val > 50) ? 2 : 1;
                
                /* Simulate non-local jump */
                __builtin_longjmp(ctx->env, 1);
            } else {
                ctx->state = 2;
            }
        }
    }
    
    /* Execute the nested function */
    nested_func();
    
    /* Access TLS with volatile to ensure it's not optimized out */
    tls_var += ctx->state;
    static_counter++;
}

/* Multi-file simulation using weak attribute */
int __attribute__((weak)) external_helper(void) {
    /* This provides a fallback if no external definition exists */
    return 42;
}

/* Main function with execution flow as specified */
int main(void) {
    struct nested_ctx ctx = {0};
    int result = 0;
    
    /* Setup: seed random and initialize */
    srand(42);
    init_counter = get_init_value();
    
    printf("Starting complex initialization sequence...\n");
    
    /* Loop to execute the key construct multiple times */
    for (int i = 0; i < 3; i++) {
        ctx.state = 0;
        
        /* Enter the key construct designed to trigger internal declaration generation */
        complex_initialization_routine(&ctx);
        
        /* Observable I/O to prevent dead code elimination */
        printf("Iteration %d: state=%d, tls_var=%d\n", 
               i, ctx.state, tls_var);
        
        result += ctx.state + tls_var;
        
        /* Call external function to influence linkage */
        if (external_helper() > 0) {
            result += external_helper();
        }
    }
    
    /* Final observable output */
    printf("Final result: %d\n", result);
    printf("Init counter: %d\n", init_counter);
    
    /* Additional construct: static initialization with function call */
    static int complex_static = (printf("Complex static init\n"), 
                                 external_helper() + rand() % 10);
    
    printf("Complex static value: %d\n", complex_static);
    
    return 0;
}

/* Simulate multi-file compilation by providing alternative definition */
#ifdef ALTERNATIVE_DEFINITION
/* Compile with: gcc -DALTERNATIVE_DEFINITION ... */
int external_helper(void) {
    /* Different implementation to test weak symbol resolution */
    return 100;
}
#endif
