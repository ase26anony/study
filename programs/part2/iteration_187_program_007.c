#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Declarations for functions in lib.c
void hot_function(int iterations);
void warm_function(int iterations);
void cold_function(void);
void rarely_called(void);
void path_a(void);
void path_b(void);
void mixed_function(int mode);

// Functions defined in main.c
void process_data(int count, int mode) {
    int i;
    for (i = 0; i < count; i++) {
        if (mode == 1) {
            // Hot path
            hot_function(10);
        } else if (mode == 2) {
            // Warm path
            warm_function(5);
        } else {
            // Cold path
            cold_function();
        }
        
        // Conditional execution
        if (i % 3 == 0) {
            path_a();
        } else {
            path_b();
        }
    }
}

void analyze_results(int iterations) {
    int i;
    double sum = 0.0;
    
    for (i = 0; i < iterations; i++) {
        sum += i * 0.1;
        
        if (i % 7 == 0) {
            rarely_called();
        }
        
        // Nested loop for more coverage
        int j;
        for (j = 0; j < 3; j++) {
            sum += j * 0.01;
        }
    }
    
    printf("Analysis result: %f\n", sum);
}

void decision_maker(int threshold) {
    static int counter = 0;
    counter++;
    
    if (counter > threshold) {
        printf("Threshold exceeded: %d\n", counter);
        mixed_function(1);
    } else {
        printf("Below threshold: %d\n", counter);
        mixed_function(2);
    }
    
    // Switch statement for coverage
    switch (counter % 4) {
        case 0:
            printf("Case 0\n");
            break;
        case 1:
            printf("Case 1\n");
            break;
        case 2:
            printf("Case 2\n");
            break;
        case 3:
            printf("Case 3\n");
            break;
    }
}

int main(int argc, char *argv[]) {
    int mode = 1;
    int iterations = 100;
    
    // Parse command line argument to change behavior
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    printf("Running in mode %d with %d iterations\n", mode, iterations);
    
    // Vary execution based on mode
    switch (mode) {
        case 1:
            // High frequency execution
            process_data(iterations * 10, 1);
            analyze_results(iterations * 5);
            break;
        case 2:
            // Medium frequency execution
            process_data(iterations, 2);
            analyze_results(iterations / 2);
            break;
        case 3:
            // Low frequency execution
            process_data(iterations / 10, 3);
            analyze_results(iterations / 20);
            break;
        default:
            // Mixed execution
            process_data(iterations, 1);
            analyze_results(iterations);
            break;
    }
    
    // Call decision maker multiple times
    int i;
    for (i = 0; i < iterations / 10; i++) {
        decision_maker(5);
    }
    
    // Final mixed function call
    mixed_function(mode);
    
    return 0;
}
