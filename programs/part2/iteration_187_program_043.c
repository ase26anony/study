#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declarations for functions in lib.c
void hot_function(int iterations);
void cold_function(void);
void medium_function(int count);
void rarely_called(void);
void path_a(void);
void path_b(void);
void mixed_function(int mode);

// Functions in main.c
void process_data(int size, int mode) {
    int i;
    if (mode == 1) {
        // Hot path - executed many times
        for (i = 0; i < size * 100; i++) {
            if (i % 2 == 0) {
                path_a();
            } else {
                path_b();
            }
        }
    } else if (mode == 2) {
        // Medium path
        for (i = 0; i < size * 10; i++) {
            mixed_function(i % 3);
        }
    } else {
        // Cold path
        rarely_called();
    }
}

void analyze_results(int count) {
    int i, j;
    double sum = 0.0;
    
    for (i = 0; i < count; i++) {
        for (j = 0; j < 50; j++) {
            sum += (i * j) / 100.0;
        }
        
        if (i % 100 == 0) {
            cold_function();
        }
    }
    
    if (sum > 1000.0) {
        printf("Sum is large: %.2f\n", sum);
    }
}

void decision_tree(int depth, int branch) {
    if (depth <= 0) {
        return;
    }
    
    switch (branch % 4) {
        case 0:
            hot_function(10);
            break;
        case 1:
            medium_function(5);
            break;
        case 2:
            cold_function();
            break;
        case 3:
            rarely_called();
            break;
    }
    
    decision_tree(depth - 1, branch + 1);
}

void main_loop(int iterations, int mode) {
    int i;
    
    for (i = 0; i < iterations; i++) {
        if (mode == 1) {
            // Mode 1: Heavy computation
            process_data(100, 1);
            analyze_results(50);
        } else if (mode == 2) {
            // Mode 2: Medium computation
            process_data(20, 2);
            analyze_results(10);
        } else {
            // Mode 3: Light computation
            process_data(5, 3);
            analyze_results(2);
        }
        
        if (i % 1000 == 0 && i > 0) {
            printf("Progress: %d/%d\n", i, iterations);
        }
    }
}

int main(int argc, char *argv[]) {
    int mode = 1;
    int iterations = 10000;
    
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
            // Heavy workload - hits hot paths
            main_loop(iterations, 1);
            decision_tree(5, 0);
            break;
        case 2:
            // Medium workload
            main_loop(iterations / 10, 2);
            decision_tree(3, 1);
            break;
        case 3:
            // Light workload - mostly cold paths
            main_loop(iterations / 100, 3);
            decision_tree(1, 2);
            break;
        default:
            // Mixed workload
            main_loop(iterations / 2, 1);
            main_loop(iterations / 2, 3);
            break;
    }
    
    printf("Execution complete\n");
    return 0;
}
