#include <stdio.h>
#include <stdlib.h>
#include "func1.h"
#include "func2.h"
#include "func3.h"

#define HOT_LOOP_COUNT 1000000
#define COLD_LOOP_COUNT 100

void process_input(int value, int iteration) {
    // This function will have varying coverage across runs
    if (value > 1000) {
        // Hot path for some runs
        for (int i = 0; i < HOT_LOOP_COUNT; i++) {
            func1_hot();
        }
    } else if (value > 500) {
        // Medium path
        func2_medium(value);
    } else if (value > 100) {
        // Cold path
        func3_cold(value);
    } else {
        // Very cold path - rarely executed
        printf("Very cold path: %d\n", value);
    }
    
    // Switch statement for additional branch coverage
    switch (value % 4) {
        case 0:
            func1_hot();
            break;
        case 1:
            func2_medium(value);
            break;
        case 2:
            func3_cold(value);
            break;
        case 3:
            // Nested condition
            if (iteration % 2 == 0) {
                for (int i = 0; i < COLD_LOOP_COUNT; i++) {
                    func1_hot();
                }
            }
            break;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <input_value> <iteration>\n", argv[0]);
        return 1;
    }
    
    int value = atoi(argv[1]);
    int iteration = atoi(argv[2]);
    
    printf("Run %d with value %d\n", iteration, value);
    
    // Always execute some baseline code
    func1_hot();
    func2_medium(42);
    
    // Variable execution based on input
    process_input(value, iteration);
    
    // Object-level variation: different execution based on iteration
    if (iteration % 3 == 0) {
        func3_cold(value);
    }
    
    return 0;
}
