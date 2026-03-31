/* 
 * This program is designed to trigger GCC's internal tree initialization
 * for compiler-generated artificial functions with hidden visibility.
 * It combines multiple patterns that force GCC to create internal declarations:
 * 1. Thread-local storage with dynamic initialization
 * 2. Nested functions with non-local jumps using setjmp/longjmp
 * 3. Complex static initializers with side effects
 * 4. External linkage hints and volatile variables
 * 5. Exception handling context via nothrow attribute
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

/* External function declaration to influence DECL_EXTERNAL handling */
extern int external_helper(void);
/* Weak attribute to simulate multi-file compilation scenario */
extern int maybe_defined_elsewhere(void) __attribute__((weak));

/* Thread-local storage with non-constant initializer */
__thread int tls_var = 0;
static int tls_initializer(void) {
    volatile int seed = 42; /* volatile to prevent optimization */
    return seed + rand();
}

/* Function marked nothrow to provide context for TREE_NOTHROW flag */
static void process_data(void) __attribute__((nothrow));

/* Complex static variable with side effects */
static int static_counter = (printf("Initializing static_counter...\n"), 0);

/* Global jmp_buf for non-local jumps */
static jmp_buf jump_buffer;

/* External function definition (simulating multi-file) */
int external_helper(void) {
    volatile int x = 10; /* volatile to prevent dead code elimination */
    return x * 2;
}

/* Main processing function with nested function and TLS access */
static void process_data(void) {
    volatile int local_state = 0; /* volatile to force actual memory operations */
    
    /* Nested function using GCC's statement expression extension */
    auto void nested_function(void) __attribute__((nothrow));
    
    void nested_function(void) {
        /* Access TLS variable - may trigger compiler-generated TLS init function */
        tls_var += 1;
        
        /* Use setjmp for non-local control flow */
        if (setjmp(jump_buffer) == 0) {
            /* Normal path - modify volatile to ensure execution */
            local_state = external_helper();
            
            /* Complex expression that might trigger helper generation */
            static_counter += (tls_var > 0) ? 1 : 0;
        } else {
            /* longjmp target - more side effects */
            local_state = -1;
            tls_var *= 2;
        }
    }
    
    /* Execute the nested function */
    nested_function();
    
    /* Force another potential internal function via longjmp simulation */
    if (local_state > 5) {
        /* This might trigger compiler-generated landing pads or helpers */
        longjmp(jump_buffer, 1);
    }
}

/* Secondary function with different TLS model */
static void __attribute__((noinline)) use_tls_aggressively(void) {
    /* Force TLS access in a loop to encourage internal stub generation */
    for (int i = 0; i < 3; i++) {
        /* Dynamic TLS initialization on each iteration */
        __thread int local_tls = tls_initializer() + i;
        tls_var += local_tls;
        
        /* Call to weak external function */
        if (maybe_defined_elsewhere) {
            tls_var += maybe_defined_elsewhere();
        }
    }
}

int main(void) {
    /* Setup phase */
    printf("Program start\n");
    srand(42);
    
    /* Initialize TLS (may trigger compiler-generated init function) */
    tls_var = tls_initializer();
    
    /* Main execution loop with the triggering constructs */
    for (int iteration = 0; iteration < 2; iteration++) {
        printf("Iteration %d: tls_var = %d\n", iteration, tls_var);
        
        /* Call function with nested function and setjmp */
        process_data();
        
        /* Call function with aggressive TLS usage */
        use_tls_aggressively();
        
        /* Complex static initializer in loop */
        static int loop_static = (printf("Loop static init\n"), 100 + iteration);
        tls_var += loop_static;
    }
    
    /* Final observable output */
    printf("Final values: tls_var = %d, static_counter = %d\n", 
           tls_var, static_counter);
    
    /* Access external helper one more time */
    volatile int final_check = external_helper();
    printf("External helper result: %d\n", final_check);
    
    return 0;
}

/* Definition of weak function (simulating potential other translation unit) */
int maybe_defined_elsewhere(void) {
    return 123;
}
