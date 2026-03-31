/* test_targhooks.c - Program to trigger GCC's internal tree initialization flags */
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

/* External function declaration to influence DECL_EXTERNAL handling */
extern int external_helper(void) __attribute__((weak));

/* Thread-local storage with dynamic initialization */
__thread int tls_var = 0;
static int init_counter = 0;

/* Function with side effects for static initialization */
int init_with_side_effects(void) {
    printf("Initializing with side effects...\n");
    return ++init_counter * 100;
}

/* Another static variable with complex initialization */
static int complex_static = (printf("Complex static init\n"), init_with_side_effects());

/* Function marked nothrow to provide context for TREE_NOTHROW */
void process_data(void) __attribute__((nothrow));

/* Global jmp_buf for non-local jumps */
jmp_buf jump_buffer;
volatile int jump_flag = 0;

/* External-like function definition (simulates multi-file compilation) */
int external_helper(void) {
    return rand() % 100;
}

/* Function using nested function with setjmp/longjmp */
void nested_function_test(void) {
    volatile int local_state = 0;
    
    /* Nested function using GCC extension */
    auto void nested_helper(void) __attribute__((nothrow));
    
    void nested_helper(void) {
        if (setjmp(jump_buffer) == 0) {
            /* First call - set up for longjmp */
            local_state = 1;
            tls_var = external_helper();  /* Access TLS and external function */
        } else {
            /* After longjmp */
            local_state = 2;
            tls_var += complex_static;    /* Access complex static */
        }
    }
    
    /* Execute the nested function */
    nested_helper();
    
    /* Potentially trigger longjmp */
    if (local_state == 1 && !jump_flag) {
        jump_flag = 1;
        longjmp(jump_buffer, 1);
    }
}

/* Main function with observable I/O */
int main(void) {
    int result = 0;
    
    /* Setup */
    srand(42);
    printf("Program start\n");
    
    /* Access TLS to ensure initialization */
    tls_var = init_with_side_effects();
    printf("Initial TLS value: %d\n", tls_var);
    
    /* Loop to execute the triggering construct multiple times */
    for (int i = 0; i < 3; i++) {
        printf("\nIteration %d:\n", i + 1);
        
        /* Reset jump flag for first iteration */
        if (i == 0) jump_flag = 0;
        
        /* Execute function with nested function and non-local jumps */
        nested_function_test();
        
        /* Access and modify TLS variable */
        tls_var += rand() % 50;
        printf("TLS after iteration: %d\n", tls_var);
        
        /* Call external helper */
        result += external_helper();
    }
    
    /* Final observable output */
    printf("\nFinal results:\n");
    printf("TLS variable: %d\n", tls_var);
    printf("Complex static: %d\n", complex_static);
    printf("Init counter: %d\n", init_counter);
    printf("Accumulated result: %d\n", result);
    printf("Program completed successfully\n");
    
    return 0;
}

/* Additional function to increase chance of internal declaration generation */
void process_data(void) {
    /* This function is marked nothrow and uses volatile */
    volatile int temp = 0;
    
    /* Try to create context for exception handling */
    for (volatile int i = 0; i < 10; i++) {
        temp += tls_var + complex_static;
    }
    
    /* Use inline assembly to prevent optimization */
    __asm__ volatile ("" : : "r"(temp));
}
