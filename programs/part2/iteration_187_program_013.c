#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Function declarations
void hot_function_1(int iterations);
void hot_function_2(int iterations);
void cold_function_1(void);
void cold_function_2(void);
void mixed_function(int mode);
void process_data(int* data, int size, int multiplier);
int calculate_sum(int a, int b, int c);
void recursive_function(int depth, int max_depth);
void utility_function(int value);

// External functions from lib.c
extern void lib_hot_function(int count);
extern void lib_cold_function(void);
extern void lib_mixed_operation(int mode);
extern int lib_compute(int x, int y);
extern void lib_process_array(int* arr, int size);

int main(int argc, char* argv[]) {
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
    
    // Seed random number generator
    srand(time(NULL));
    
    // Vary execution based on mode
    switch (mode) {
        case 1:
            // Mode 1: Heavy execution of hot functions
            for (int i = 0; i < iterations; i++) {
                hot_function_1(100);
                hot_function_2(50);
                if (i % 10 == 0) {
                    cold_function_1();
                }
                if (i % 20 == 0) {
                    cold_function_2();
                }
            }
            lib_hot_function(iterations / 2);
            break;
            
        case 2:
            // Mode 2: More balanced execution
            for (int i = 0; i < iterations / 2; i++) {
                mixed_function(i % 3);
                hot_function_1(20);
                if (i % 5 == 0) {
                    hot_function_2(10);
                }
            }
            cold_function_1();
            cold_function_2();
            lib_mixed_operation(2);
            break;
            
        case 3:
            // Mode 3: Light execution with recursion
            for (int i = 0; i < iterations / 10; i++) {
                recursive_function(0, 5);
                utility_function(i);
            }
            lib_cold_function();
            break;
            
        default:
            // Default mode: Mixed execution
            for (int i = 0; i < iterations; i++) {
                hot_function_1(30);
                mixed_function(i % 4);
                if (i % 15 == 0) {
                    recursive_function(0, 3);
                }
            }
            break;
    }
    
    // Additional processing based on mode
    if (mode == 1 || mode == 2) {
        int data[100];
        for (int i = 0; i < 100; i++) {
            data[i] = i * mode;
        }
        process_data(data, 100, mode);
        lib_process_array(data, 100);
    }
    
    // Final computations
    int result = 0;
    for (int i = 0; i < 50; i++) {
        result += calculate_sum(i, i * 2, i * 3);
        if (mode > 1) {
            result += lib_compute(i, mode);
        }
    }
    
    printf("Final result: %d\n", result);
    return 0;
}

void hot_function_1(int iterations) {
    int sum = 0;
    for (int i = 0; i < iterations; i++) {
        sum += i;
        if (i % 7 == 0) {
            sum -= 2;
        }
    }
    // Unused variable to create some uncovered code
    int unused = sum * 2;
}

void hot_function_2(int iterations) {
    int product = 1;
    for (int i = 1; i <= iterations; i++) {
        product *= (i % 10) + 1;
        if (product > 1000000) {
            product /= 10;
        }
    }
}

void cold_function_1(void) {
    // Rarely executed function
    static int call_count = 0;
    call_count++;
    
    // Complex but rarely used logic
    int matrix[3][3] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    int determinant = 0;
    
    // This calculation is rarely executed
    determinant = matrix[0][0] * (matrix[1][1] * matrix[2][2] - matrix[1][2] * matrix[2][1])
                - matrix[0][1] * (matrix[1][0] * matrix[2][2] - matrix[1][2] * matrix[2][0])
                + matrix[0][2] * (matrix[1][0] * matrix[2][1] - matrix[1][1] * matrix[2][0]);
}

void cold_function_2(void) {
    // Another rarely executed function
    char* messages[] = {"Cold function called", "Rare execution path", 
                       "Infrequently used code"};
    int index = rand() % 3;
    
    // This printf is rarely executed
    printf("Cold function message: %s\n", messages[index]);
}

void mixed_function(int mode) {
    switch (mode) {
        case 0:
            // Frequently executed path
            for (int i = 0; i < 10; i++) {
                utility_function(i);
            }
            break;
        case 1:
            // Less frequent path
            hot_function_1(5);
            break;
        case 2:
            // Rare path
            cold_function_1();
            break;
        default:
            // Default path
            utility_function(mode);
            break;
    }
}

void process_data(int* data, int size, int multiplier) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * multiplier;
        if (data[i] > 1000) {
            data[i] = data[i] % 1000;
        }
    }
    
    // Additional processing for larger multipliers
    if (multiplier > 2) {
        int sum = 0;
        for (int i = 0; i < size / 2; i++) {
            sum += data[i];
        }
    }
}

int calculate_sum(int a, int b, int c) {
    int result = a + b + c;
    
    // Conditional execution
    if (result > 100) {
        result /= 2;
    } else if (result < 0) {
        result = 0;
    }
    
    return result;
}

void recursive_function(int depth, int max_depth) {
    if (depth >= max_depth) {
        return;
    }
    
    // Do some work
    utility_function(depth);
    
    // Recursive calls
    recursive_function(depth + 1, max_depth);
    
    // Additional recursive call sometimes
    if (depth % 2 == 0 && depth < max_depth - 1) {
        recursive_function(depth + 2, max_depth);
    }
}

void utility_function(int value) {
    // Simple utility function
    static int total_calls = 0;
    total_calls++;
    
    // Perform some calculation
    int result = value * value;
    if (value % 2 == 0) {
        result += value;
    } else {
        result -= value;
    }
}
