#include <stdio.h>
#include <stdlib.h>
#include "functions.h"

#define HOT_LOOP_COUNT 1000000
#define COLD_LOOP_COUNT 100

void hot_function(int iterations) {
    int i, sum = 0;
    // This becomes a hot block when called with high iterations
    for (i = 0; i < iterations; i++) {
        sum += i % 100;
    }
    printf("Hot function result: %d\n", sum);
}

void cold_function(int iterations) {
    int i, sum = 0;
    // This stays cold
    for (i = 0; i < iterations; i++) {
        sum += i % 10;
    }
    printf("Cold function result: %d\n", sum);
}

int main(int argc, char *argv[]) {
    int input_value = 0;
    
    if (argc > 1) {
        input_value = atoi(argv[1]);
    } else {
        // Read from file if no command line argument
        FILE *f = fopen("input.txt", "r");
        if (f) {
            fscanf(f, "%d", &input_value);
            fclose(f);
        }
    }
    
    printf("Input value: %d\n", input_value);
    
    // Complex branching to generate varied coverage
    if (input_value < 0) {
        printf("Negative path\n");
        helper_function1();
        hot_function(COLD_LOOP_COUNT);
    } else if (input_value == 0) {
        printf("Zero path\n");
        helper_function2();
        cold_function(COLD_LOOP_COUNT);
    } else if (input_value < 100) {
        printf("Small positive path\n");
        helper_function1();
        helper_function2();
        hot_function(input_value * 1000);
    } else if (input_value < 1000) {
        printf("Medium positive path\n");
        helper_function1();
        hot_function(HOT_LOOP_COUNT);
        cold_function(COLD_LOOP_COUNT);
    } else {
        printf("Large positive path\n");
        helper_function2();
        // Very hot path
        hot_function(HOT_LOOP_COUNT * 2);
        cold_function(input_value % 100);
    }
    
    // Additional branching based on input
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
    }
    
    return 0;
}
