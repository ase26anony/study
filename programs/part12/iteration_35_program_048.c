#include <stdio.h>
#include <stdlib.h>
#include "functions.h"

#define HOT_LOOP_COUNT 1000000
#define COLD_LOOP_COUNT 100

void process_input(int value) {
    if (value > 1000) {
        // Hot path - executed many times
        for (int i = 0; i < HOT_LOOP_COUNT; i++) {
            hot_function_1();
        }
    } else if (value > 500) {
        // Medium path
        for (int i = 0; i < COLD_LOOP_COUNT; i++) {
            medium_function();
        }
    } else if (value > 100) {
        // Cold path
        cold_function();
    } else {
        // Very cold path
        rarely_called_function();
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_value>\n", argv[0]);
        return 1;
    }
    
    int value = atoi(argv[1]);
    
    // Always call this to ensure coverage
    common_function();
    
    // Branch based on input value
    if (value % 2 == 0) {
        process_input(value);
    } else {
        // Alternative path
        for (int i = 0; i < value % 1000; i++) {
            hot_function_2();
        }
    }
    
    // Nested conditional for more coverage complexity
    if (value > 2000) {
        deeply_nested_function(value);
    }
    
    return 0;
}
