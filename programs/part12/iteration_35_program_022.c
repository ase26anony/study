/* test_prog.c - Generates varied coverage data for gcov-tool overlap testing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Function prototypes from other modules */
void hot_function(int iterations);
void cold_function(void);
void medium_function(int value);
void conditional_function(int a, int b);

/* Global counter for hot blocks */
static unsigned long hot_counter = 0;

/* Main function with multiple execution paths */
int main(int argc, char *argv[]) {
    int input_value = 0;
    
    /* Parse input value from command line or file */
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
        for (int i = 0; i < 1000000; i++) {
            hot_counter++;
            if (i % 1000 == 0) {
                hot_function(10);
            }
        }
    }
    /* Branch 2: Medium path */
    else if (input_value > 100) {
        printf("Taking MEDIUM path\n");
        for (int i = 0; i < 10000; i++) {
            medium_function(i);
            if (i % 2 == 0) {
                conditional_function(i, input_value);
            }
        }
    }
    /* Branch 3: Cold path */
    else if (input_value > 10) {
        printf("Taking COLD path\n");
        cold_function();
        for (int i = 0; i < 100; i++) {
            if (i % 3 == 0) {
                medium_function(i);
            }
        }
    }
    /* Branch 4: Very cold path */
    else {
        printf("Taking VERY COLD path\n");
        cold_function();
        if (input_value < 0) {
            /* This branch is rarely taken */
            printf("Negative input detected!\n");
        }
    }
    
    /* Nested conditional for more coverage complexity */
    switch (input_value % 4) {
        case 0:
            printf("Case 0\n");
            hot_function(5);
            break;
        case 1:
            printf("Case 1\n");
            cold_function();
            break;
        case 2:
            printf("Case 2\n");
            medium_function(input_value);
            break;
        case 3:
            printf("Case 3\n");
            /* Do nothing - uncovered branch */
            break;
    }
    
    return 0;
}
