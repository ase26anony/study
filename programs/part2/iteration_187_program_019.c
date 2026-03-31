#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function declarations from lib.c
void hot_function(int iterations);
void cold_function(void);
void medium_function(int count);
void branching_function(int value);
void utility_function(int x);

// Functions defined in main.c
void process_data(int mode, int iterations) {
    if (mode == 1) {
        // Hot path - executed many times
        for (int i = 0; i < iterations * 10; i++) {
            utility_function(i % 100);
        }
    } else if (mode == 2) {
        // Medium path
        for (int i = 0; i < iterations * 5; i++) {
            utility_function(i % 50);
        }
    } else {
        // Cold path - executed rarely
        utility_function(1);
    }
}

void analyze_results(int data) {
    int sum = 0;
    for (int i = 0; i < data; i++) {
        if (i % 3 == 0) {
            sum += i * 2;
        } else if (i % 3 == 1) {
            sum += i;
        } else {
            sum += i / 2;
        }
    }
    
    if (sum > 1000) {
        printf("High sum: %d\n", sum);
    } else if (sum > 100) {
        printf("Medium sum: %d\n", sum);
    } else {
        printf("Low sum: %d\n", sum);
    }
}

void complex_calculation(int base) {
    int result = base;
    for (int i = 0; i < 100; i++) {
        result = (result * 3 + 7) % 1000;
        if (result > 500) {
            result /= 2;
        } else if (result > 200) {
            result += 100;
        }
    }
    
    // Nested loop
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < i; j++) {
            result += j;
        }
    }
}

void data_transformer(int *array, int size) {
    if (size <= 0) return;
    
    for (int i = 0; i < size; i++) {
        if (array[i] % 2 == 0) {
            array[i] *= 2;
        } else {
            array[i] = array[i] * 3 + 1;
        }
    }
    
    // Bubble sort (inefficient on purpose for coverage)
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            if (array[j] > array[j + 1]) {
                int temp = array[j];
                array[j] = array[j + 1];
                array[j + 1] = temp;
            }
        }
    }
}

void recursive_function(int depth, int max_depth) {
    if (depth >= max_depth) return;
    
    printf("Depth: %d\n", depth);
    if (depth % 2 == 0) {
        recursive_function(depth + 1, max_depth);
    } else {
        recursive_function(depth + 2, max_depth);
    }
}

int main(int argc, char *argv[]) {
    int mode = 1;
    int iterations = 100;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    printf("Running in mode %d with %d iterations\n", mode, iterations);
    
    // Call functions with different frequencies based on mode
    switch (mode) {
        case 1:
            // Hot profile - execute hot paths many times
            for (int i = 0; i < iterations; i++) {
                hot_function(100);
                process_data(1, 10);
                medium_function(50);
            }
            branching_function(1000);
            break;
            
        case 2:
            // Medium profile - balanced execution
            for (int i = 0; i < iterations / 2; i++) {
                hot_function(50);
                process_data(2, 5);
                medium_function(25);
            }
            branching_function(500);
            cold_function();
            break;
            
        case 3:
            // Cold profile - minimal execution
            hot_function(10);
            process_data(3, 1);
            medium_function(5);
            branching_function(100);
            cold_function();
            break;
            
        default:
            // Mixed profile
            for (int i = 0; i < iterations / 4; i++) {
                if (i % 3 == 0) {
                    hot_function(30);
                } else if (i % 3 == 1) {
                    medium_function(20);
                } else {
                    cold_function();
                }
                process_data(i % 3 + 1, 3);
            }
            branching_function(300);
            break;
    }
    
    // Additional function calls for coverage
    analyze_results(iterations * 10);
    complex_calculation(mode * 100);
    
    int data[20];
    for (int i = 0; i < 20; i++) {
        data[i] = (i * mode) % 100;
    }
    data_transformer(data, 20);
    
    recursive_function(0, 5);
    
    return 0;
}
