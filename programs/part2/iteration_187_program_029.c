#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Function declarations
void hot_function(int iterations);
void cold_function(void);
void medium_function(int n);
void rarely_called(void);
void always_called(void);
void conditional_branch(int x);
void loop_variations(int mode);

// External functions from lib.c
extern void lib_hot_function(int count);
extern void lib_cold_function(void);
extern void lib_medium_work(int iterations);
extern void lib_rare_function(void);
extern void lib_always_executed(void);

int main(int argc, char *argv[]) {
    int mode = 1;
    int iterations = 1000;
    
    // Parse command line arguments to vary execution paths
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    printf("Running in mode %d with %d iterations\n", mode, iterations);
    
    // Always execute these
    always_called();
    lib_always_executed();
    
    // Vary execution based on mode
    switch (mode) {
        case 1:
            // Mode 1: Heavy execution of hot functions
            hot_function(iterations * 10);
            lib_hot_function(iterations * 5);
            medium_function(iterations);
            lib_medium_work(iterations / 2);
            break;
            
        case 2:
            // Mode 2: More balanced execution
            hot_function(iterations);
            lib_hot_function(iterations);
            for (int i = 0; i < iterations / 10; i++) {
                cold_function();
                lib_cold_function();
            }
            break;
            
        case 3:
            // Mode 3: Focus on rarely called functions
            hot_function(iterations / 100);
            rarely_called();
            lib_rare_function();
            for (int i = 0; i < 5; i++) {
                conditional_branch(i);
            }
            break;
            
        default:
            // Default: Mixed execution
            hot_function(iterations);
            lib_hot_function(iterations);
            medium_function(iterations / 2);
            cold_function();
            lib_cold_function();
            rarely_called();
            lib_rare_function();
    }
    
    // Execute loop variations based on mode
    loop_variations(mode);
    
    // More conditional execution
    if (mode % 2 == 0) {
        for (int i = 0; i < iterations / 100; i++) {
            conditional_branch(i % 10);
        }
    } else {
        conditional_branch(mode);
    }
    
    printf("Execution complete for mode %d\n", mode);
    return 0;
}

void hot_function(int iterations) {
    int sum = 0;
    // Hot loop - executed many times
    for (int i = 0; i < iterations; i++) {
        sum += i;
        if (i % 100 == 0) {
            sum -= 50;  // Branch taken occasionally
        }
    }
    // Prevent optimization
    if (sum < 0) printf("Impossible\n");
}

void cold_function(void) {
    // Rarely executed function
    static int call_count = 0;
    call_count++;
    if (call_count % 1000 == 0) {
        printf("Cold function called %d times\n", call_count);
    }
}

void medium_function(int n) {
    // Medium frequency function
    for (int i = 0; i < n; i++) {
        if (i % 3 == 0) {
            // Branch taken 1/3 of the time
            int x = i * 2;
            if (x > 100) {
                x /= 2;
            }
        }
    }
}

void rarely_called(void) {
    // Very rarely called
    printf("Rare function executed\n");
}

void always_called(void) {
    // Always executed once
    printf("Program starting\n");
}

void conditional_branch(int x) {
    // Function with multiple branches
    if (x < 0) {
        printf("Negative: %d\n", x);
    } else if (x == 0) {
        printf("Zero\n");
    } else if (x < 10) {
        printf("Small positive: %d\n", x);
    } else {
        printf("Large positive: %d\n", x);
    }
}

void loop_variations(int mode) {
    // Different loop patterns based on mode
    switch (mode) {
        case 1:
            for (int i = 0; i < 1000; i++) {
                if (i % 100 == 0) continue;
                if (i > 500) break;
            }
            break;
            
        case 2:
            for (int i = 1000; i > 0; i--) {
                if (i % 7 == 0) {
                    // Nested loop
                    for (int j = 0; j < 10; j++) {
                        if (j == 5) continue;
                    }
                }
            }
            break;
            
        default:
            int i = 0;
            while (i < 500) {
                i += (mode % 3) + 1;
                if (i % 50 == 0) {
                    i += 10;
                }
            }
    }
}
