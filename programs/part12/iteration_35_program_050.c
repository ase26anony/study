/* test_prog.c - Generates varied coverage data for gcov-tool overlap testing */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Function prototypes from other modules */
extern void hot_function(int iterations);
extern void cold_function(void);
extern void medium_function(int count);
extern void conditional_function(int value);

/* Global counter for hot blocks */
static int global_counter = 0;

/* Main function with multiple execution paths */
int main(int argc, char *argv[]) {
    int input_value = 0;
    
    /* Read input from command line or file */
    if (argc > 1) {
        input_value = atoi(argv[1]);
    } else {
        /* Try to read from file if no command line argument */
        FILE *fp = fopen("input.txt", "r");
        if (fp) {
            fscanf(fp, "%d", &input_value);
            fclose(fp);
        }
    }
    
    printf("Running with input value: %d\n", input_value);
    
    /* Branch 1: Very hot path - large loop */
    if (input_value > 1000) {
        printf("Taking HOT path\n");
        for (int i = 0; i < input_value; i++) {
            global_counter++;
            if (i % 100 == 0) {
                hot_function(10);  /* Call hot function */
            }
        }
    }
    /* Branch 2: Medium path */
    else if (input_value > 100) {
        printf("Taking MEDIUM path\n");
        for (int i = 0; i < input_value / 10; i++) {
            medium_function(i);
        }
        conditional_function(input_value);
    }
    /* Branch 3: Cold path */
    else if (input_value > 0) {
        printf("Taking COLD path\n");
        cold_function();
        conditional_function(input_value);
    }
    /* Branch 4: Very cold path */
    else {
        printf("Taking VERY COLD path\n");
        /* Minimal execution */
        if (input_value == 0) {
            printf("Zero input\n");
        } else {
            printf("Negative input\n");
        }
    }
    
    /* Nested conditional for more coverage complexity */
    switch (input_value % 4) {
        case 0:
            printf("Case 0\n");
            break;
        case 1:
            printf("Case 1\n");
            hot_function(5);
            break;
        case 2:
            printf("Case 2\n");
            cold_function();
            break;
        case 3:
            printf("Case 3\n");
            medium_function(3);
            break;
    }
    
    return 0;
}
