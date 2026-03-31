/* test_prog.c - Generates varied coverage data for overlap analysis */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Function prototypes from other modules */
extern void hot_function(int iterations);
extern void cold_function(void);
extern void medium_function(int value);
extern void varied_function(int mode);

/* Global counter for hot blocks */
static long global_counter = 0;

/* Main function with multiple execution paths */
int main(int argc, char *argv[]) {
    int input_value = 0;
    
    /* Read input from command line or file */
    if (argc > 1) {
        input_value = atoi(argv[1]);
    } else {
        /* Try to read from file */
        FILE *fp = fopen("input.txt", "r");
        if (fp) {
            fscanf(fp, "%d", &input_value);
            fclose(fp);
        }
    }
    
    printf("Running with input value: %d\n", input_value);
    
    /* Branch 1: Hot path - executed many times */
    if (input_value > 1000) {
        /* Very hot block - large loop */
        for (int i = 0; i < input_value; i++) {
            global_counter++;
            if (i % 100 == 0) {
                hot_function(10);  /* Call hot function */
            }
        }
    }
    
    /* Branch 2: Medium path */
    else if (input_value > 100) {
        for (int i = 0; i < input_value / 10; i++) {
            global_counter += 2;
            medium_function(i);
        }
    }
    
    /* Branch 3: Cold path */
    else if (input_value > 10) {
        cold_function();
        varied_function(1);
    }
    
    /* Branch 4: Very cold path */
    else if (input_value > 0) {
        varied_function(2);
    }
    
    /* Branch 5: Default path */
    else {
        printf("Default execution path\n");
        /* Mix of calls */
        hot_function(1);
        cold_function();
        medium_function(5);
        varied_function(3);
    }
    
    /* Switch statement for additional coverage */
    switch (input_value % 5) {
        case 0:
            printf("Case 0 executed\n");
            break;
        case 1:
            printf("Case 1 executed\n");
            hot_function(3);
            break;
        case 2:
            printf("Case 2 executed\n");
            cold_function();
            break;
        case 3:
            printf("Case 3 executed\n");
            medium_function(input_value);
            break;
        case 4:
            printf("Case 4 executed\n");
            varied_function(input_value % 3);
            break;
        default:
            printf("Unexpected case\n");
    }
    
    /* Final hot block - always executed */
    for (int i = 0; i < 100; i++) {
        global_counter += i;
    }
    
    printf("Final counter: %ld\n", global_counter);
    return 0;
}
