/* test_targhooks.c - Program to trigger GCC's internal declaration generation */
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <pthread.h>

/* External function declaration to influence DECL_EXTERNAL handling */
extern int external_helper(void) __attribute__((weak));

/* Thread-local storage with non-constant initializer - may generate TLS init function */
__thread volatile int tls_var = 0;
__thread int tls_init = (printf("TLS init side effect\n"), 42);

/* Static variable with side-effect initialization - may generate static init function */
static int static_with_side_effects = (rand(), 123);

/* Function marked nothrow to provide context for TREE_NOTHROW flag */
static void process_data(void) __attribute__((nothrow));

/* Complex nested function using setjmp/longjmp - may generate trampoline/helper functions */
void complex_nested_operation(jmp_buf *env) {
    /* Use GCC's nested function extension */
    __extension__ void nested_func(void) {
        volatile int local_counter = 0;
        
        /* Use setjmp which may require compiler-generated helper */
        if (setjmp(*env) == 0) {
            /* Access TLS variable to ensure it's used */
            tls_var = local_counter + 1;
            
            /* Call external function if available */
            if (external_helper) {
                tls_var += external_helper();
            }
        } else {
            /* longjmp target */
            tls_var = 100;
        }
        
        /* Force volatile access */
        local_counter = tls_var;
    }
    
    /* Execute the nested function */
    nested_func();
}

/* Function that uses OpenMP-like constructs (simulated) */
#pragma GCC visibility push(hidden)
static void hidden_helper(void) {
    /* This function has hidden visibility */
    volatile static int counter = 0;
    counter++;
    
    /* Complex initialization that might generate helpers */
    static int complex_init = (counter > 0 ? counter : 1);
    
    /* Use the complex init value */
    tls_var = complex_init;
}
#pragma GCC visibility pop

/* Process data with exception-like control flow */
static void process_data(void) {
    jmp_buf env;
    volatile int result = 0;
    
    /* Multiple attempts to trigger helper generation */
    for (int i = 0; i < 3; i++) {
        complex_nested_operation(&env);
        
        /* On first iteration, simulate longjmp */
        if (i == 0) {
            longjmp(env, 1);
        }
        
        /* Call hidden helper */
        hidden_helper();
        
        /* Use static variable with side effects */
        result += static_with_side_effects;
        
        /* Modify TLS variable */
        tls_var += i;
    }
    
    /* Print result to prevent optimization */
    printf("Processing result: %d\n", result);
    printf("TLS variable value: %d\n", tls_var);
}

/* Weak external function definition (simulates multi-file scenario) */
int external_helper(void) {
    return rand() % 100;
}

/* Main function with observable I/O */
int main(void) {
    /* Setup */
    srand(42);
    
    printf("Starting program to trigger GCC internal declarations...\n");
    
    /* Execute the complex operation */
    process_data();
    
    /* Additional complexity: function pointer that might trigger stub generation */
    void (*func_ptr)(void) = process_data;
    
    /* Call through pointer (prevents inlining) */
    volatile int call_counter = 0;
    if (call_counter == 0) {
        func_ptr();
    }
    
    /* Final output */
    printf("TLS init value: %d\n", tls_init);
    printf("Program completed successfully.\n");
    
    return 0;
}

/* Additional file-like separation using attribute sections */
__attribute__((section(".extra")))
static const char* extra_data = "Additional data section";

/* Force generation of static constructors/destructors */
static void __attribute__((constructor)) init_func(void) {
    tls_var = -1;
}

static void __attribute__((destructor)) cleanup_func(void) {
    printf("Cleanup: TLS was %d\n", tls_var);
}
