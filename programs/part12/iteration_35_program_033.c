#include <stdio.h>
#include <stdlib.h>
#include "func1.h"
#include "func2.h"
#include "func3.h"

#define HOT_LOOP_COUNT 1000000
#define COLD_LOOP_COUNT 100

void process_input(int value, int iteration) {
    if (value > 1000) {
        // Hot path - executed many times
        for (int i = 0; i < HOT_LOOP_COUNT; i++) {
            func1_hot(i);
        }
    } else if (value > 500) {
        // Medium path
        func2_medium(value);
    } else if (value > 100) {
        // Cold path
        func3_cold(value);
    } else {
        // Very cold path
        printf("Very cold path: %d\n", value);
    }
    
    // Branch based on iteration number
    switch (iteration % 4) {
        case 0:
            func1_helper(value);
            break;
        case 1:
            func2_helper(value);
            break;
        case 2:
            func3_helper(value);
            break;
        case 3:
            // Do nothing
            break;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <value> <iteration>\n", argv[0]);
        return 1;
    }
    
    int value = atoi(argv[1]);
    int iteration = atoi(argv[2]);
    
    printf("Run %d with value %d\n", iteration, value);
    
    // Different execution patterns based on iteration
    if (iteration % 2 == 0) {
        // Even iterations use more hot paths
        for (int i = 0; i < 10; i++) {
            process_input(value + i * 100, iteration);
        }
    } else {
        // Odd iterations use more cold paths
        for (int i = 0; i < 5; i++) {
            process_input(value - i * 50, iteration);
        }
    }
    
    // Object-level variation: call functions from different compilation units
    if (value % 3 == 0) {
        cross_file_call_1();
    }
    if (value % 5 == 0) {
        cross_file_call_2();
    }
    
    return 0;
}
