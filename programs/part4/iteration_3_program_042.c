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
#include <pthread.h>

/* External function declaration to influence DECL_EXTERNAL handling */
extern int external_helper(void) __attribute__((weak));

/* Thread-local storage with dynamic initialization */
__thread volatile int tls_var = 0;
static int init_counter = 0;

/* Function with side effects for complex static initialization */
static int init_with_side_effects(void) __attribute__((noinline, noreturn));
static int init_with_side_effects(void) {
    /* This forces GCC to generate initialization code */
    printf("Initializing with side effects\n");
    exit(0);
    return 0; /* Never reached */
}

/* Complex static variable with side effects */
static int complex_static = (printf("Complex static init\n"), 42);

/* Function marked nothrow to provide context for TREE_NOTHROW */
static void process_data(void) __attribute__((nothrow));

/* jmp_buf for non-local jumps */
static jmp_buf jump_buffer;

/* Volatile variable to prevent optimization */
volatile int volatile_guard = 1;

/* External-like function defined in same file (simulating multi-file) */
int external_helper(void) {
    return rand() % 100;
}

/* Nested function using GCC extension - often triggers internal helpers */
static void nested_function_context(void) {
    /* Use GCC's nested function extension */
    __extension__ auto void nested_func(int x) {
        if (x > 0) {
            /* Use setjmp within nested function - may trigger helper generation */
            if (setjmp(jump_buffer) == 0) {
                /* Access TLS variable */
                tls_var = x * 2;
                
                /* Call external function */
                volatile_guard = external_helper();
                
                /* Force non-local jump */
                if (x > 50) {
                    longjmp(jump_buffer, 1);
                }
            }
        }
    }
    
    /* Execute the nested function */
    for (int i = 0; i < 10; i++) {
        nested_func(i * volatile_guard);
    }
}

/* Function with OpenMP-like structure (without actual OpenMP for portability) */
static void parallel_like_computation(void) {
    /* Structure that might trigger stub generation */
    struct computation_ctx {
        void (*func)(void*);
        void *data;
    };
    
    static struct computation_ctx ctx = {0};
    
    /* Simulate offloading-like pattern */
    if (ctx.func) {
        ctx.func(ctx.data);
    }
}

/* Main execution flow */
int main(void) {
    int result = 0;
    
    /* Setup */
    srand(42);
    init_counter++;
    
    printf("Program start - counter: %d\n", init_counter);
    
    /* Execute various patterns that may trigger internal declaration generation */
    
    /* 1. Nested function with setjmp context */
    if (volatile_guard) {
        nested_function_context();
    }
    
    /* 2. TLS access with apparent dynamic initialization */
    tls_var = (int)((long)&tls_var % 1000); /* Non-constant address-based init */
    printf("TLS var: %d\n", tls_var);
    
    /* 3. Complex static initialization execution */
    result += complex_static;
    
    /* 4. External function call */
    result += external_helper();
    
    /* 5. Try-catch like structure for exception context */
    {
        /* Simulate try block */
        volatile int try_block = 1;
        while (try_block--) {
            /* Force potential internal function generation */
            parallel_like_computation();
            
            /* Use volatile to prevent dead code elimination */
            if (volatile_guard) {
                /* Access through pointer to force memory operations */
                int *ptr = &result;
                *ptr += tls_var;
            }
        }
    }
    
    /* 6. Additional pattern: function pointer with volatile */
    void (* volatile func_ptr)(void) = (void (*)(void))&process_data;
    
    /* Execute if non-NULL */
    if (func_ptr != NULL) {
        /* This might trigger additional internal handling */
        result += 1;
    }
    
    /* Observable output to prevent optimization */
    printf("Final result: %d\n", result);
    printf("Volatile guard: %d\n", volatile_guard);
    
    /* Force reference to init function to keep it in tree */
    if (result > 1000) {
        init_with_side_effects();
    }
    
    return result % 256;
}

/* Empty nothrow function definition */
static void process_data(void) {
    /* Empty but referenced */
}
