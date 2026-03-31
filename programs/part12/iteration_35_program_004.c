#include <stdio.h>
#include <stdlib.h>
#include "functions.h"

#define HOT_LOOP_COUNT 1000000
#define COLD_LOOP_COUNT 100

void process_input(int value, int iteration) {
    if (value > 1000) {
        // Hot path - executed many times
        for (int i = 0; i < HOT_LOOP_COUNT; i++) {
            hot_function(i, iteration);
        }
    } else if (value > 500) {
        // Medium path
        medium_function(value, iteration);
    } else if (value > 100) {
        // Cold path - rarely executed
        cold_function(value);
    } else {
        // Very cold path
        very_cold_function();
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <input_value> <run_id>\n", argv[0]);
        return 1;
    }
    
    int value = atoi(argv[1]);
    int run_id = atoi(argv[2]);
    
    printf("Run %d with value %d\n", run_id, value);
    
    // Always call some functions
    common_function(run_id);
    
    // Process based on input
    process_input(value, run_id);
    
    // Call object-specific functions
    if (value % 2 == 0) {
        obj1_function(value);
    } else {
        obj2_function(value);
    }
    
    // Conditional based on value
    if (value > 2000) {
        for (int i = 0; i < HOT_LOOP_COUNT / 10; i++) {
            hot_function(i, run_id);
        }
    }
    
    return 0;
}
