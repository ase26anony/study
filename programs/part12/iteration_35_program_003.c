/* test_prog.c - Program with varied execution paths for coverage analysis */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Function declarations from other modules */
extern void func_a(int count);
extern void func_b(int count);
extern void func_c(int count);
extern void hot_loop(int iterations);
extern void cold_path(void);

/* Global variables to track execution */
static int global_counter = 0;

/* Main function with multiple branches */
int main(int argc, char *argv[]) {
    int input_value = 0;
    
    /* Parse input argument */
    if (argc > 1) {
        input_value = atoi(argv[1]);
    } else {
        /* Read from file if no argument */
        FILE *fp = fopen("input.txt", "r");
        if (fp) {
            fscanf(fp, "%d", &input_value);
            fclose(fp);
        }
    }
    
    printf("Running with input: %d\n", input_value);
    
    /* Complex branching structure */
    if (input_value <= 0) {
        /* Path 1: Negative or zero input */
        printf("Path 1: Default execution\n");
        func_a(1);
        func_b(1);
        cold_path();
    } else if (input_value <= 100) {
        /* Path 2: Small positive input */
        printf("Path 2: Moderate execution\n");
        func_a(input_value);
        func_b(input_value / 2);
        
        /* Nested condition */
        if (input_value > 50) {
            func_c(10);
        }
    } else if (input_value <= 1000) {
        /* Path 3: Medium input - hot path */
        printf("Path 3: Hot execution path\n");
        hot_loop(input_value);
        func_a(100);
        func_b(50);
    } else {
        /* Path 4: Large input - very hot path */
        printf("Path 4: Very hot execution\n");
        hot_loop(input_value * 2);
        func_a(200);
        func_b(100);
        func_c(50);
        
        /* Additional loop for extreme counts */
        for (int i = 0; i < input_value / 100; i++) {
            global_counter++;
        }
    }
    
    /* Switch statement for additional coverage */
    switch (input_value % 4) {
        case 0:
            printf("Case 0\n");
            break;
        case 1:
            printf("Case 1\n");
            break;
        case 2:
            printf("Case 2\n");
            break;
        case 3:
            printf("Case 3\n");
            break;
        default:
            printf("Default case\n");
    }
    
    /* Final conditional based on global state */
    if (global_counter > 1000) {
        printf("Extremely hot execution detected!\n");
    }
    
    return 0;
}
