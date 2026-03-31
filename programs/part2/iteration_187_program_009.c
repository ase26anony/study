#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Declarations for functions in lib.c
void hot_function(int iterations);
void cold_function(void);
void medium_function(int count);
void rarely_called(void);
void varying_function(int mode);

// Functions in main.c
void process_data(int mode, int iterations) {
    if (mode == 1) {
        // Hot path for mode 1
        for (int i = 0; i < iterations * 10; i++) {
            hot_function(i % 100);
        }
    } else if (mode == 2) {
        // Different hot path for mode 2
        for (int i = 0; i < iterations * 5; i++) {
            medium_function(i % 50);
        }
    } else {
        // Cold path
        rarely_called();
    }
}

void analyze_results(int count) {
    int sum = 0;
    for (int i = 0; i < count; i++) {
        sum += i;
        if (i % 100 == 0) {
            varying_function(1);
        } else if (i % 37 == 0) {
            varying_function(2);
        }
    }
    printf("Analysis sum: %d\n", sum);
}

void setup_environment(void) {
    // Called once
    printf("Setting up environment...\n");
}

void cleanup_resources(void) {
    // Called once
    printf("Cleaning up resources...\n");
}

void decision_maker(int threshold) {
    if (threshold > 50) {
        hot_function(threshold);
    } else if (threshold > 20) {
        medium_function(threshold);
    } else {
        cold_function();
    }
}

int main(int argc, char *argv[]) {
    int mode = 1;
    int iterations = 1000;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    printf("Running in mode %d with %d iterations\n", mode, iterations);
    
    setup_environment();
    
    // Vary execution based on mode
    switch (mode) {
        case 1:
            process_data(1, iterations);
            analyze_results(iterations * 2);
            decision_maker(75);  // Hot path
            break;
        case 2:
            process_data(2, iterations / 2);
            analyze_results(iterations);
            decision_maker(30);  // Medium path
            break;
        case 3:
            process_data(3, 1);
            analyze_results(10);
            decision_maker(10);  // Cold path
            break;
        default:
            process_data(1, 100);
            analyze_results(100);
            decision_maker(40);
    }
    
    cleanup_resources();
    
    return 0;
}
