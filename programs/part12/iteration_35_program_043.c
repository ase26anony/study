#include <stdio.h>
#include <stdlib.h>
#include "functions.h"

#define HOT_LOOP_COUNT 1000000
#define WARM_LOOP_COUNT 1000
#define COLD_LOOP_COUNT 10

void process_input(int value, const char* mode) {
    printf("Processing value %d in mode %s\n", value, mode);
    
    if (value > 1000) {
        // Hot path - executed many times
        for (int i = 0; i < HOT_LOOP_COUNT; i++) {
            if (i % 2 == 0) {
                hot_function_a();
            } else {
                hot_function_b();
            }
        }
    } else if (value > 100) {
        // Warm path
        for (int i = 0; i < WARM_LOOP_COUNT; i++) {
            warm_function();
        }
    } else if (value > 0) {
        // Cold path
        for (int i = 0; i < COLD_LOOP_COUNT; i++) {
            cold_function();
        }
    } else {
        // Rare path
        rare_function();
    }
    
    // Object-level differentiation
    if (value % 3 == 0) {
        obj1_function();
    } else if (value % 3 == 1) {
        obj2_function();
    } else {
        obj3_function();
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <value> <output_file>\n", argv[0]);
        return 1;
    }
    
    int value = atoi(argv[1]);
    const char* output_file = argv[2];
    
    // Create different execution patterns based on value
    if (value > 5000) {
        // Extreme hot path
        for (int j = 0; j < 10; j++) {
            process_input(value + j, "extreme");
        }
    } else {
        process_input(value, "normal");
    }
    
    // Write result to file
    FILE *f = fopen(output_file, "w");
    if (f) {
        fprintf(f, "Processed value: %d\n", value);
        fclose(f);
    }
    
    return 0;
}
