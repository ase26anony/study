#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lib.h"

#define HOT_LOOP_COUNT 1000
#define COLD_LOOP_COUNT 10

/* Function declarations */
void hot_function_1(int iterations);
void hot_function_2(int iterations);
void cold_function_1(void);
void cold_function_2(void);
void mixed_function(int mode);
void path_dependent_function(int mode);
void recursive_function(int depth, int max_depth);
void loop_variant_function(int base, int multiplier);

/* Global variables for different execution paths */
static int global_counter = 0;
static const char* mode_names[] = {"FAST", "SLOW", "MIXED"};

int main(int argc, char *argv[]) {
    int mode = 1;  // Default mode
    int iterations = 100;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
        if (mode < 1) mode = 1;
        if (mode > 3) mode = 3;
    }
    
    if (argc > 2) {
        iterations = atoi(argv[2]);
        if (iterations < 1) iterations = 100;
    }
    
    printf("Running in mode %d (%s) with %d iterations\n", 
           mode, mode_names[mode-1], iterations);
    
    // Vary execution based on mode
    switch(mode) {
        case 1:  // Hot path dominant
            for (int i = 0; i < HOT_LOOP_COUNT; i++) {
                hot_function_1(iterations);
                hot_function_2(iterations / 2);
                lib_hot_function_1(i);
            }
            cold_function_1();  // Called once
            break;
            
        case 2:  // Cold path dominant
            for (int i = 0; i < COLD_LOOP_COUNT; i++) {
                cold_function_1();
                cold_function_2();
                lib_cold_function_1();
            }
            hot_function_1(10);  // Minimal hot calls
            break;
            
        case 3:  // Mixed execution
            for (int i = 0; i < iterations; i++) {
                mixed_function(i % 3);
                path_dependent_function(i % 4);
                lib_mixed_function(i);
            }
            break;
    }
    
    // Always execute these but with different frequencies
    recursive_function(0, mode * 2);
    loop_variant_function(mode, iterations);
    
    // Call library functions
    lib_common_function(mode);
    lib_utility_function(iterations);
    
    printf("Global counter: %d\n", global_counter);
    return 0;
}

void hot_function_1(int iterations) {
    int sum = 0;
    for (int i = 0; i < iterations; i++) {
        sum += i * i;
        global_counter++;
    }
    // Conditional that's usually true
    if (sum > 100) {
        global_counter += 2;
    } else {
        global_counter--;  // Rarely executed
    }
}

void hot_function_2(int iterations) {
    for (int i = 0; i < iterations; i++) {
        if (i % 2 == 0) {
            global_counter += 3;
        } else {
            global_counter += 1;
        }
    }
}

void cold_function_1(void) {
    static int call_count = 0;
    call_count++;
    
    // Complex but rarely executed logic
    if (call_count % 7 == 0) {
        global_counter *= 2;
    } else if (call_count % 13 == 0) {
        global_counter /= 2;
    } else {
        global_counter += 5;
    }
}

void cold_function_2(void) {
    // Nested conditionals rarely all true
    if (global_counter > 1000) {
        if (global_counter % 17 == 0) {
            global_counter -= 50;
        }
    }
}

void mixed_function(int mode) {
    switch(mode) {
        case 0:
            global_counter += 10;  // Frequently executed
            break;
        case 1:
            global_counter += 5;   // Sometimes executed
            break;
        default:
            global_counter += 1;   // Rarely executed
            break;
    }
}

void path_dependent_function(int mode) {
    // Different execution paths
    if (mode == 0) {
        for (int i = 0; i < 5; i++) {
            global_counter++;
        }
    } else if (mode == 1) {
        global_counter += 10;
    } else if (mode == 2) {
        // Nested loop
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 2; j++) {
                global_counter++;
            }
        }
    } else {
        // Default path
        global_counter += 3;
    }
}

void recursive_function(int depth, int max_depth) {
    if (depth >= max_depth) {
        return;
    }
    
    global_counter += depth;
    
    // Sometimes recurse twice
    if (depth % 2 == 0) {
        recursive_function(depth + 1, max_depth);
        recursive_function(depth + 1, max_depth);
    } else {
        recursive_function(depth + 1, max_depth);
    }
}

void loop_variant_function(int base, int multiplier) {
    int limit = base * multiplier;
    
    for (int i = 0; i < limit; i++) {
        if (i % base == 0) {
            global_counter += 2;
        } else {
            global_counter += 1;
        }
        
        // Inner conditional
        if (i > limit / 2) {
            global_counter--;
        }
    }
}
