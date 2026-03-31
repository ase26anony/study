#include <stdio.h>
#include <stdlib.h>
#include "func1.h"
#include "func2.h"

/* Global variable to control execution paths */
int global_counter = 0;

/* Function with multiple branches */
void process_input(int value) {
    if (value < 0) {
        printf("Negative value: %d\n", value);
        global_counter += value;
    } else if (value == 0) {
        printf("Zero value\n");
        global_counter = 0;
    } else if (value < 100) {
        printf("Small positive: %d\n", value);
        global_counter += value * 2;
    } else if (value < 1000) {
        printf("Medium positive: %d\n", value);
        global_counter += value * 3;
    } else {
        printf("Large value: %d\n", value);
        global_counter += value * 4;
    }
}

/* Function with hot/cold blocks */
void hot_cold_test(int iterations, int threshold) {
    int i;
    /* HOT BLOCK - executed many times */
    for (i = 0; i < iterations; i++) {
        if (i % 1000 == 0) {
            /* COLD BLOCK - executed rarely */
            printf("Milestone: %d\n", i);
        }
        
        if (i < threshold) {
            /* Conditionally hot block */
            global_counter += i;
        } else {
            /* Another cold block */
            global_counter -= 1;
        }
    }
}

int main(int argc, char *argv[]) {
    int input_value;
    
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <value> [iterations]\n", argv[0]);
        return 1;
    }
    
    input_value = atoi(argv[1]);
    
    /* Process input with multiple branches */
    process_input(input_value);
    
    /* Call functions from other compilation units */
    int result1 = func1_operation(input_value);
    int result2 = func2_operation(input_value);
    
    printf("Results: func1=%d, func2=%d\n", result1, result2);
    
    /* Hot/cold testing */
    int iterations = (argc > 2) ? atoi(argv[2]) : 10000;
    hot_cold_test(iterations, input_value);
    
    /* Switch statement for additional coverage */
    switch (input_value % 5) {
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
        case 4:
            printf("Case 4\n");
            break;
    }
    
    printf("Final counter: %d\n", global_counter);
    return 0;
}
