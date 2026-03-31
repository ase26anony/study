/* test_prog.c - Generates varied coverage data for overlap analysis */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Function prototypes from other modules */
extern void func_a(int iterations);
extern void func_b(int iterations);
extern void func_c(int iterations);
extern void hot_function(void);
extern void cold_function(void);

/* Global variables for different execution paths */
static int global_counter = 0;

/* Main function with multiple conditional paths */
int main(int argc, char *argv[]) {
    int input_value = 0;
    int loop_count = 1;
    
    /* Parse input argument */
    if (argc > 1) {
        input_value = atoi(argv[1]);
    } else {
        /* Read from stdin if no argument */
        char buffer[32];
        if (fgets(buffer, sizeof(buffer), stdin)) {
            input_value = atoi(buffer);
        }
    }
    
    printf("Running with input: %d\n", input_value);
    
    /* Branch 1: Different function calls based on input range */
    if (input_value < 0) {
        printf("Negative path\n");
        func_a(100);
        cold_function();
    } else if (input_value < 100) {
        printf("Small positive path\n");
        func_b(500);
        if (input_value % 2 == 0) {
            hot_function();
        } else {
            cold_function();
        }
    } else if (input_value < 1000) {
        printf("Medium positive path\n");
        func_c(1000);
        hot_function();
    } else {
        printf("Large positive path\n");
        /* Execute all functions with high iteration counts */
        func_a(2000);
        func_b(2000);
        func_c(2000);
        hot_function();
    }
    
    /* Branch 2: Loop execution based on input */
    loop_count = abs(input_value) % 10000;
    
    /* Create hot blocks with high execution counts */
    for (int i = 0; i < loop_count; i++) {
        global_counter++;
        
        /* Nested condition to create more branches */
        if (i % 100 == 0) {
            printf("Progress: %d\n", i);
        }
        
        if (i % 500 == 0 && input_value > 0) {
            hot_function();
        }
    }
    
    /* Branch 3: Switch statement for additional coverage */
    switch (input_value % 5) {
        case 0:
            printf("Case 0 executed\n");
            break;
        case 1:
            printf("Case 1 executed\n");
            func_a(10);
            break;
        case 2:
            printf("Case 2 executed\n");
            func_b(10);
            break;
        case 3:
            printf("Case 3 executed\n");
            func_c(10);
            break;
        case 4:
            printf("Case 4 executed\n");
            hot_function();
            cold_function();
            break;
        default:
            printf("Default case\n");
            break;
    }
    
    printf("Final counter: %d\n", global_counter);
    return 0;
}
