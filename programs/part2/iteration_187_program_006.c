#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Function declarations
void hot_function(int iterations);
void cold_function(void);
void medium_function(int count);
void rarely_called(void);
void path_selector(int mode);
void process_data(int *data, int size);
void recursive_function(int depth, int max_depth);
void loop_variations(int type);
void conditional_test(int value);
void file_operations(void);

// External functions from lib.c
extern void lib_hot_function(int count);
extern void lib_cold_function(void);
extern void lib_complex_function(int mode);
extern int lib_calculate(int a, int b);
extern void lib_data_process(int iterations);

int main(int argc, char *argv[]) {
    int mode = 1;
    int iterations = 1000;
    
    // Parse command line arguments to vary execution
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    srand(time(NULL));
    
    printf("Running in mode %d with %d iterations\n", mode, iterations);
    
    // Vary execution based on mode to create different profiles
    switch (mode) {
        case 1:
            // Mode 1: Heavy on hot functions
            for (int i = 0; i < iterations; i++) {
                hot_function(i);
                if (i % 10 == 0) {
                    medium_function(i);
                }
                if (i % 100 == 0) {
                    cold_function();
                }
                
                // Call library functions
                lib_hot_function(i % 50);
                if (i % 20 == 0) {
                    lib_cold_function();
                }
            }
            break;
            
        case 2:
            // Mode 2: More balanced execution
            for (int i = 0; i < iterations / 2; i++) {
                hot_function(i);
                medium_function(i);
                if (i % 5 == 0) {
                    cold_function();
                }
                
                // Different library usage pattern
                lib_complex_function(i % 3);
                int result = lib_calculate(i, i * 2);
                if (result % 7 == 0) {
                    rarely_called();
                }
            }
            break;
            
        case 3:
            // Mode 3: Different execution paths
            for (int i = 0; i < iterations; i++) {
                path_selector(i % 4);
                loop_variations(i % 3);
                conditional_test(i);
                
                // Heavy library usage
                lib_data_process(i % 100);
                if (i % 50 == 0) {
                    lib_cold_function();
                }
            }
            break;
            
        default:
            // Default mode: Mixed execution
            for (int i = 0; i < iterations; i++) {
                if (i % 3 == 0) hot_function(i);
                if (i % 5 == 0) medium_function(i);
                if (i % 7 == 0) cold_function();
                if (i % 11 == 0) rarely_called();
                
                lib_hot_function(i % 30);
                if (i % 13 == 0) {
                    lib_complex_function(2);
                }
            }
            break;
    }
    
    // Always execute some common code
    process_data(NULL, 0);
    recursive_function(0, 3);
    
    printf("Execution completed for mode %d\n", mode);
    return 0;
}

// Hot function - executed many times
void hot_function(int iterations) {
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += i * iterations;
    }
    
    // Some conditional logic
    if (sum > 1000000) {
        sum /= 2;
    } else if (sum > 100000) {
        sum /= 3;
    } else {
        sum += iterations;
    }
}

// Cold function - rarely executed
void cold_function(void) {
    static int call_count = 0;
    call_count++;
    
    // Complex but rarely executed logic
    int array[10];
    for (int i = 0; i < 10; i++) {
        array[i] = i * call_count;
    }
    
    // Nested conditionals
    if (call_count % 2 == 0) {
        for (int i = 0; i < 5; i++) {
            array[i] *= 2;
        }
    } else {
        for (int i = 5; i < 10; i++) {
            array[i] /= 2;
        }
    }
}

// Medium frequency function
void medium_function(int count) {
    int result = 0;
    for (int i = 0; i < count % 50; i++) {
        result += i;
        if (i % 3 == 0) {
            result *= 2;
        } else if (i % 3 == 1) {
            result /= 2;
        }
    }
}

// Very rarely called function
void rarely_called(void) {
    // Complex nested loops that are rarely executed
    int matrix[5][5];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            matrix[i][j] = i * j;
            if (i > j) {
                matrix[i][j] += 100;
            }
        }
    }
}

// Function with multiple execution paths
void path_selector(int mode) {
    switch (mode) {
        case 0:
            // Path 0 - frequently taken
            for (int i = 0; i < 10; i++) {
                hot_function(i);
            }
            break;
        case 1:
            // Path 1 - moderately taken
            medium_function(20);
            break;
        case 2:
            // Path 2 - rarely taken
            cold_function();
            break;
        case 3:
            // Path 3 - very rarely taken
            rarely_called();
            break;
    }
}

void process_data(int *data, int size) {
    if (data == NULL || size == 0) {
        // Default behavior
        int local_data[10];
        for (int i = 0; i < 10; i++) {
            local_data[i] = i * 2;
        }
    } else {
        // Process actual data
        for (int i = 0; i < size; i++) {
            data[i] *= 2;
        }
    }
}

void recursive_function(int depth, int max_depth) {
    if (depth >= max_depth) {
        return;
    }
    
    // Some computation
    int value = depth * 10;
    for (int i = 0; i < depth; i++) {
        value += i;
    }
    
    // Recursive call
    recursive_function(depth + 1, max_depth);
    
    // More computation after recursion
    value *= 2;
}

void loop_variations(int type) {
    switch (type) {
        case 0:
            // Simple loop
            for (int i = 0; i < 100; i++) {
                // Do nothing, just count
            }
            break;
        case 1:
            // Nested loop
            for (int i = 0; i < 10; i++) {
                for (int j = 0; j < 10; j++) {
                    int temp = i * j;
                }
            }
            break;
        case 2:
            // Loop with condition
            for (int i = 0; i < 50; i++) {
                if (i % 2 == 0) {
                    hot_function(i);
                } else {
                    medium_function(i);
                }
            }
            break;
    }
}

void conditional_test(int value) {
    // Multiple conditionals to create complex branching
    if (value < 10) {
        // Frequently true
        hot_function(value);
    } else if (value < 50) {
        // Moderately true
        medium_function(value);
    } else if (value < 100) {
        // Rarely true
        cold_function();
    } else {
        // Very rarely true
        rarely_called();
    }
    
    // Nested conditionals
    if (value % 2 == 0) {
        if (value % 4 == 0) {
            // 25% of even numbers
            hot_function(value * 2);
        }
    } else {
        if (value % 3 == 0) {
            // 33% of odd numbers
            medium_function(value / 2);
        }
    }
}

void file_operations(void) {
    // This function is never called in our tests
    // but exists to show uncovered code
    printf("This should not appear in output\n");
}
