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
void file_operation_sim(int mode);

// External functions from lib.c
extern void lib_hot_function(int count);
extern void lib_cold_function(void);
extern void lib_mixed_operation(int value);
extern int lib_compute(int x, int y);
extern void lib_data_transform(int* arr, int len);

int main(int argc, char* argv[]) {
    int mode = 1;
    int iterations = 1000;
    
    // Parse command line arguments
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    printf("Running in mode %d with %d iterations\n", mode, iterations);
    
    // Seed random number generator
    srand(time(NULL));
    
    // Execute different code paths based on mode
    switch (mode) {
        case 1:
            // Mode 1: Heavy execution of hot functions
            for (int i = 0; i < iterations; i++) {
                hot_function_1(i % 100);
                hot_function_2(i % 50);
                
                if (i % 10 == 0) {
                    lib_hot_function(i % 20);
                }
                
                if (i % 100 == 0) {
                    cold_function_1();
                    lib_cold_function();
                }
            }
            break;
            
        case 2:
            // Mode 2: More balanced execution
            for (int i = 0; i < iterations / 2; i++) {
                mixed_function(i % 3);
                lib_mixed_operation(i);
                
                if (i % 5 == 0) {
                    hot_function_1(10);
                }
                
                if (i % 7 == 0) {
                    hot_function_2(5);
                }
            }
            break;
            
        case 3:
            // Mode 3: Data processing intensive
            {
                int data[100];
                for (int i = 0; i < 100; i++) {
                    data[i] = rand() % 1000;
                }
                
                for (int i = 0; i < iterations / 10; i++) {
                    process_data(data, 100, i % 10 + 1);
                    lib_data_transform(data, 100);
                }
            }
            break;
            
        default:
            // Default mode: Mix of all operations
            for (int i = 0; i < iterations / 5; i++) {
                hot_function_1(rand() % 50);
                hot_function_2(rand() % 30);
                mixed_function(rand() % 4);
                lib_hot_function(rand() % 15);
                lib_mixed_operation(rand() % 100);
                
                if (i % 20 == 0) {
                    cold_function_1();
                    cold_function_2();
                    lib_cold_function();
                }
            }
            break;
    }
    
    // Always execute some common code
    file_operation_sim(mode);
    recursive_function(0, mode + 2);
    
    printf("Program completed successfully\n");
    return 0;
}

void hot_function_1(int iterations) {
    int sum = 0;
    for (int i = 0; i < iterations; i++) {
        sum += i;
        if (i % 2 == 0) {
            sum *= 2;
        } else {
            sum /= 2;
        }
    }
    // Simulate some work
    volatile int result = sum;
}

void hot_function_2(int iterations) {
    double product = 1.0;
    for (int i = 1; i <= iterations; i++) {
        product *= i;
        if (product > 1000000.0) {
            product /= 1000.0;
        }
    }
    // Simulate some work
    volatile double res = product;
}

void cold_function_1(void) {
    // Rarely executed function
    printf("Cold function 1 executed\n");
    
    // Some computation
    long factorial = 1;
    for (int i = 1; i <= 5; i++) {
        factorial *= i;
    }
}

void cold_function_2(void) {
    // Another rarely executed function
    printf("Cold function 2 executed\n");
    
    // String manipulation simulation
    char buffer[50];
    for (int i = 0; i < 10; i++) {
        buffer[i] = 'A' + (i % 26);
    }
    buffer[10] = '\0';
}

void mixed_function(int mode) {
    switch (mode) {
        case 0:
            // Path 0
            for (int i = 0; i < 10; i++) {
                calculate_sum(i, i*2, i*3);
            }
            break;
        case 1:
            // Path 1
            {
                int x = lib_compute(10, 20);
                volatile int y = x * 2;
            }
            break;
        case 2:
            // Path 2
            for (int i = 0; i < 5; i++) {
                if (i % 2 == 0) {
                    calculate_sum(i, i+1, i+2);
                }
            }
            break;
        default:
            // Default path
            printf("Mixed function default path\n");
            break;
    }
}

void process_data(int* data, int size, int multiplier) {
    if (data == NULL || size <= 0) {
        return;
    }
    
    // Process data with different algorithms based on multiplier
    if (multiplier % 2 == 0) {
        // Even multiplier: simple scaling
        for (int i = 0; i < size; i++) {
            data[i] *= multiplier;
        }
    } else {
        // Odd multiplier: more complex transformation
        for (int i = 0; i < size; i++) {
            data[i] = (data[i] * multiplier) + (i % 10);
        }
    }
    
    // Additional processing for large multipliers
    if (multiplier > 5) {
        int temp = 0;
        for (int i = 0; i < size / 2; i++) {
            temp += data[i];
        }
        volatile int check = temp;
    }
}

int calculate_sum(int a, int b, int c) {
    int sum = a + b + c;
    
    // Some conditional logic
    if (sum > 100) {
        return sum / 2;
    } else if (sum < 0) {
        return sum * -1;
    } else {
        return sum;
    }
}

void recursive_function(int depth, int max_depth) {
    if (depth >= max_depth) {
        return;
    }
    
    // Do some work
    volatile int x = depth * 10;
    
    // Recursive call
    recursive_function(depth + 1, max_depth);
    
    // More work after recursion
    if (depth % 2 == 0) {
        volatile int y = x + 5;
    }
}

void file_operation_sim(int mode) {
    // Simulate file operations
    FILE* fp = NULL;
    
    if (mode == 1) {
        // Mode 1: Simple write simulation
        for (int i = 0; i < 10; i++) {
            volatile char c = 'A' + (i % 26);
        }
    } else if (mode == 2) {
        // Mode 2: Read simulation
        for (int i = 0; i < 5; i++) {
            volatile int value = i * 100;
        }
    } else {
        // Other modes: Mixed operations
        for (int i = 0; i < 3; i++) {
            if (i % 2 == 0) {
                volatile char c = 'Z' - i;
            } else {
                volatile int num = 1000 / (i + 1);
            }
        }
    }
}
