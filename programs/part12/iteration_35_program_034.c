#include <stdio.h>
#include <stdlib.h>
#include "functions.h"

#define HOT_LOOP_COUNT 1000000
#define COLD_LOOP_COUNT 100

void process_input(int value, int iteration) {
    if (value > 1000) {
        // Hot path - executed many times
        for (int i = 0; i < HOT_LOOP_COUNT; i++) {
            complex_calculation(value + i);
        }
        printf("Iteration %d: Hot path executed\n", iteration);
    } else if (value > 500) {
        // Medium path
        medium_function(value);
        printf("Iteration %d: Medium path executed\n", iteration);
    } else if (value > 100) {
        // Cold path
        cold_function(value);
        printf("Iteration %d: Cold path executed\n", iteration);
    } else {
        // Very cold path
        rarely_called(value);
        printf("Iteration %d: Very cold path executed\n", iteration);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <value>\n", argv[0]);
        return 1;
    }
    
    int value = atoi(argv[1]);
    int iteration = (argc > 2) ? atoi(argv[2]) : 1;
    
    // Call functions from other compilation units
    init_functions();
    
    // Main processing
    process_input(value, iteration);
    
    // Additional conditional calls
    if (value % 3 == 0) {
        utility_function_a(value);
    }
    if (value % 5 == 0) {
        utility_function_b(value);
    }
    
    cleanup_functions();
    return 0;
}
