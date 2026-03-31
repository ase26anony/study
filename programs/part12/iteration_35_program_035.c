#include <stdio.h>
#include <stdlib.h>
#include "lib1.h"
#include "lib2.h"

#define HOT_LOOP_COUNT 1000000
#define COLD_LOOP_COUNT 100

void process_input(int value) {
    if (value > 1000) {
        // Hot path - executed many times
        for (int i = 0; i < HOT_LOOP_COUNT; i++) {
            lib1_hot_function();
        }
    } else if (value > 500) {
        // Medium path
        lib1_medium_function(value);
    } else if (value > 100) {
        // Cold path
        lib2_cold_function(value);
    } else {
        // Very cold path
        lib2_rare_function();
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_value>\n", argv[0]);
        return 1;
    }
    
    int value = atoi(argv[1]);
    
    // Always call some functions
    lib1_init();
    lib2_init();
    
    // Process based on input
    process_input(value);
    
    // Conditional execution based on value
    if (value % 2 == 0) {
        lib1_even_path();
    } else {
        lib1_odd_path();
    }
    
    // Another conditional
    switch (value % 3) {
        case 0:
            lib2_case0();
            break;
        case 1:
            lib2_case1();
            break;
        case 2:
            lib2_case2();
            break;
    }
    
    return 0;
}
