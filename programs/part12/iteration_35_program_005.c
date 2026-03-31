#include <stdio.h>
#include <stdlib.h>
#include "functions.h"

#define HOT_LOOP_COUNT 1000000
#define COLD_LOOP_COUNT 100

void process_input(int value, int iteration) {
    if (value > 1000) {
        // Hot path - executed many times
        for (int i = 0; i < HOT_LOOP_COUNT; i++) {
            hot_function1(i + iteration);
        }
    } else if (value > 500) {
        // Medium path
        medium_function(value);
    } else if (value > 100) {
        // Cold path - rarely executed
        cold_function(value);
    } else {
        // Very cold path
        very_cold_function(value);
    }
    
    // Branch based on value
    switch (value % 4) {
        case 0:
            branch_function_a(value);
            break;
        case 1:
            branch_function_b(value);
            break;
        case 2:
            branch_function_c(value);
            break;
        case 3:
            branch_function_d(value);
            break;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input_value>\n", argv[0]);
        return 1;
    }
    
    int value = atoi(argv[1]);
    int iteration = (argc > 2) ? atoi(argv[2]) : 1;
    
    printf("Processing value: %d, iteration: %d\n", value, iteration);
    
    // Call functions from different compilation units
    init_module();
    
    // Main processing
    process_input(value, iteration);
    
    // Additional conditional execution
    if (value % 7 == 0) {
        special_case_function(value);
    }
    
    cleanup_module();
    
    return 0;
}
