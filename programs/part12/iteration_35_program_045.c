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
            func1_hot(i + iteration);
        }
    } else if (value > 500) {
        // Medium path
        func2_medium(value, iteration);
    } else if (value > 100) {
        // Cold path
        func3_cold(value);
    } else {
        // Very cold path
        printf("Default case: %d\n", value);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <input_value> <iteration_id>\n", argv[0]);
        return 1;
    }
    
    int value = atoi(argv[1]);
    int iteration = atoi(argv[2]);
    
    // Always execute some common code
    printf("Run %d with value %d\n", iteration, value);
    
    // Branch based on input
    if (value % 2 == 0) {
        process_input(value, iteration);
    } else {
        // Alternative path
        for (int i = 0; i < COLD_LOOP_COUNT; i++) {
            if (i % 3 == 0) {
                func1_hot(i);
            } else if (i % 3 == 1) {
                func2_medium(i, iteration);
            } else {
                func3_cold(i);
            }
        }
    }
    
    // Execute object-specific code
    if (iteration == 1) {
        object_specific_code_1();
    } else if (iteration == 2) {
        object_specific_code_2();
    } else {
        object_specific_code_3();
    }
    
    return 0;
}
