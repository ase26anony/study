#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function declarations from lib.c
extern void process_data(int iterations, int mode);
extern void analyze_results(int count);
extern void validate_input(int value);
extern void generate_report(int lines);
extern void cleanup_resources(void);

// Local functions
void hot_function_1(int iterations) {
    for (int i = 0; i < iterations; i++) {
        if (i % 100 == 0) {
            printf("Hot function 1: iteration %d\n", i);
        }
    }
}

void hot_function_2(int iterations) {
    int sum = 0;
    for (int i = 0; i < iterations; i++) {
        sum += i;
        if (i % 500 == 0) {
            printf("Hot function 2: sum = %d\n", sum);
        }
    }
}

void cold_function_1(void) {
    printf("Cold function 1 executed\n");
    // Rarely executed path
    if (rand() % 1000 == 0) {
        printf("Very rare path in cold function 1\n");
    }
}

void cold_function_2(void) {
    printf("Cold function 2 executed\n");
    // Another rarely executed path
    for (int i = 0; i < 3; i++) {
        if (i == 2 && rand() % 500 == 0) {
            printf("Special case in cold function 2\n");
        }
    }
}

void medium_function(int iterations) {
    for (int i = 0; i < iterations / 10; i++) {
        if (i % 50 == 0) {
            printf("Medium function: %d\n", i);
        }
    }
}

void process_mode_1(void) {
    printf("=== Processing Mode 1 (High Activity) ===\n");
    
    // Execute hot functions many times
    hot_function_1(10000);
    hot_function_2(5000);
    
    // Execute medium function
    medium_function(1000);
    
    // Rarely execute cold functions
    if (rand() % 10 == 0) {
        cold_function_1();
    }
    
    // Call library functions
    process_data(1000, 1);
    analyze_results(100);
    generate_report(50);
}

void process_mode_2(void) {
    printf("=== Processing Mode 2 (Medium Activity) ===\n");
    
    // Execute hot functions fewer times
    hot_function_1(2000);
    hot_function_2(1000);
    
    // Execute medium function more
    medium_function(500);
    
    // Execute cold functions more often
    if (rand() % 3 == 0) {
        cold_function_1();
        cold_function_2();
    }
    
    // Call library functions with different parameters
    process_data(500, 2);
    analyze_results(50);
    validate_input(42);
    generate_report(25);
}

void process_mode_3(void) {
    printf("=== Processing Mode 3 (Low Activity) ===\n");
    
    // Minimal execution
    hot_function_1(100);
    hot_function_2(50);
    
    // Mostly cold functions
    cold_function_1();
    cold_function_2();
    
    // Library calls with minimal data
    process_data(100, 3);
    validate_input(10);
    cleanup_resources();
}

int main(int argc, char *argv[]) {
    int mode = 1;
    
    // Parse command line argument for mode
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    printf("Starting program in mode %d\n", mode);
    
    // Seed random number generator differently for each mode
    srand(mode * 1000);
    
    // Process based on mode
    switch (mode) {
        case 1:
            process_mode_1();
            break;
        case 2:
            process_mode_2();
            break;
        case 3:
            process_mode_3();
            break;
        default:
            printf("Unknown mode %d, using mode 1\n", mode);
            process_mode_1();
            break;
    }
    
    // Final cleanup
    cleanup_resources();
    printf("Program completed successfully in mode %d\n", mode);
    
    return 0;
}
