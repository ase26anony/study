#include <stdio.h>
#include <stdlib.h>
#include "func1.h"
#include "func2.h"
#include "func3.h"

#define HOT_LOOP_COUNT 1000000
#define WARM_LOOP_COUNT 10000
#define COLD_LOOP_COUNT 10

void process_input(int value, int iteration) {
    // Create varying execution patterns based on input
    if (value > 1000) {
        // Hot path - executed many times
        for (int i = 0; i < HOT_LOOP_COUNT; i++) {
            func1_hot();
        }
        printf("Iteration %d: Hot path taken\n", iteration);
    } else if (value > 500) {
        // Warm path
        for (int i = 0; i < WARM_LOOP_COUNT; i++) {
            func2_warm();
        }
        printf("Iteration %d: Warm path taken\n", iteration);
    } else if (value > 100) {
        // Medium path
        func3_medium(value);
        printf("Iteration %d: Medium path taken\n", iteration);
    } else if (value > 0) {
        // Cold path - rarely executed
        for (int i = 0; i < COLD_LOOP_COUNT; i++) {
            func1_cold();
        }
        printf("Iteration %d: Cold path taken\n", iteration);
    } else {
        // Very cold path
        func2_very_cold();
        printf("Iteration %d: Very cold path taken\n", iteration);
    }
    
    // Additional branching for more coverage
    switch (value % 5) {
        case 0:
            func3_case0();
            break;
        case 1:
            func3_case1();
            break;
        case 2:
            func3_case2();
            break;
        case 3:
            func3_case3();
            break;
        case 4:
            func3_case4();
            break;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_value> <run_id>\n", argv[0]);
        return 1;
    }
    
    int value = atoi(argv[1]);
    int run_id = atoi(argv[2]);
    
    printf("Run %d with value %d\n", run_id, value);
    
    // Call functions from different compilation units
    func1_init();
    func2_init();
    func3_init();
    
    // Process input multiple times to create varying profiles
    for (int i = 0; i < 10; i++) {
        process_input(value + i, i);
    }
    
    // Additional execution based on run_id
    if (run_id % 2 == 0) {
        func1_even_run();
    } else {
        func2_odd_run();
    }
    
    return 0;
}
