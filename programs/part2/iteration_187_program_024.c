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
    if (mode == 0) {
        // Hot path - executed many times
        for (int i = 0; i < iterations; i++) {
            hot_function(i % 10);
        }
    } else if (mode == 1) {
        // Medium path
        for (int i = 0; i < iterations / 2; i++) {
            medium_function(i);
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
        } else if (i % 50 == 0) {
            varying_function(2);
        }
    }
    printf("Analysis sum: %d\n", sum);
}

void initialize_system(void) {
    printf("System initialized\n");
    cold_function();
}

void cleanup_system(void) {
    printf("System cleanup\n");
}

void run_test_suite(int test_id) {
    switch (test_id) {
        case 1:
            process_data(0, 1000);
            analyze_results(500);
            break;
        case 2:
            process_data(1, 500);
            analyze_results(250);
            break;
        case 3:
            process_data(2, 100);
            analyze_results(100);
            break;
        default:
            process_data(0, 100);
            analyze_results(50);
    }
}

int main(int argc, char *argv[]) {
    int mode = 0;
    int iterations = 100;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    printf("Running in mode %d with %d iterations\n", mode, iterations);
    
    initialize_system();
    
    // Vary execution based on mode
    if (mode == 0) {
        // Profile 1: Heavy execution
        for (int i = 0; i < 5; i++) {
            process_data(0, iterations);
            analyze_results(iterations / 2);
        }
        run_test_suite(1);
    } else if (mode == 1) {
        // Profile 2: Medium execution
        process_data(1, iterations);
        analyze_results(iterations / 4);
        run_test_suite(2);
        run_test_suite(3);
    } else {
        // Profile 3: Light execution
        process_data(2, iterations / 10);
        analyze_results(iterations / 10);
        run_test_suite(3);
    }
    
    cleanup_system();
    
    return 0;
}
