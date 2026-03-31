/* test_targhooks.c - Complex program to trigger GCC's internal declaration generation */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

/* External function declaration to influence DECL_EXTERNAL handling */
extern int external_helper(void) __attribute__((weak));

/* Thread-local storage with dynamic initialization */
__thread int tls_var = 0;
static volatile int init_counter = 0;

/* Function that will be marked nothrow */
static int initialize_tls(void) __attribute__((nothrow));

/* Complex static initializer with side effects */
static int complex_init = (printf("Initializing complex static...\n"), 42);

/* Function using setjmp/longjmp for non-local jumps */
static jmp_buf jump_buffer;

/* External function definition (simulating multi-file compilation) */
int external_helper(void) {
    return rand() % 100;
}

/* TLS initialization function */
static int initialize_tls(void) {
    int result = external_helper();
    tls_var = result + complex_init;
    init_counter++;
    return tls_var;
}

/* Nested function using GCC extension */
static void process_with_nested(int value) {
    /* Use GCC's nested function extension */
    auto void nested_helper(int x) __attribute__((nothrow));
    
    void nested_helper(int x) {
        volatile int local = x;
        
        /* Use setjmp within nested function */
        if (setjmp(jump_buffer) == 0) {
            /* Modify TLS variable */
            tls_var += local;
            
            /* Force compiler to consider non-local exit */
            if (tls_var > 1000) {
                longjmp(jump_buffer, 1);
            }
        }
    }
    
    /* Call the nested function */
    nested_helper(value);
    
    /* Access volatile variable to prevent optimization */
    (void)init_counter;
}

/* Main function with OpenMP-like structure */
int main(void) {
    int result = 0;
    
    /* Seed random for external_helper */
    srand(42);
    
    printf("Program start\n");
    
    /* Initialize TLS with dynamic value */
    tls_var = initialize_tls();
    printf("Initial TLS value: %d\n", tls_var);
    
    /* Loop to execute the triggering construct multiple times */
    for (int i = 0; i < 5; i++) {
        /* Use nested function with setjmp */
        process_with_nested(i * 10);
        
        /* Complex expression with volatile */
        volatile int temp = tls_var;
        result += temp;
        
        printf("Iteration %d: tls_var = %d, result = %d\n", 
               i, tls_var, result);
        
        /* Occasionally trigger longjmp */
        if (tls_var > 500 && i < 3) {
            longjmp(jump_buffer, 1);
        }
    }
    
    /* Final output to ensure code isn't dead */
    printf("Final result: %d\n", result);
    printf("Init counter: %d\n", init_counter);
    printf("Complex init value: %d\n", complex_init);
    
    return 0;
}

/* Additional static initialization with function call */
static int late_init = (initialize_tls(), 100);

/* Simulate OpenMP target region using attribute */
__attribute__((target("default")))
static void target_function(void) {
    /* This might generate offloading stubs */
    volatile int x = tls_var + late_init;
    (void)x;
}

/* Call target function to ensure it's not dead code */
__attribute__((constructor))
static void init_target(void) {
    target_function();
}
