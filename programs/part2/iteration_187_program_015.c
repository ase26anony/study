#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function declarations
void hot_function(int iterations);
void cold_function(void);
void medium_function(int count);
void branching_function(int value);
void loop_function(int limit);
void utility_function(void);
void data_processing(int size);
void conditional_execution(int mode);
void nested_loops(int depth);
void mixed_operations(int param);

// External functions from lib.c
extern void lib_hot_function(int iterations);
extern void lib_cold_function(void);
extern void lib_complex_function(int param);
extern void lib_data_transform(int *data, int size);
extern void lib_recursive_function(int depth);

// Global variables for varied execution paths
static int global_counter = 0;
static char buffer[256];

int main(int argc, char *argv[]) {
    int mode = 1;
    int iterations = 1000;
    
    // Parse command line argument to change execution mode
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    printf("Running in mode %d with %d iterations\n", mode, iterations);
    
    // Vary execution based on mode to create different profile data
    switch (mode) {
        case 1:
            // Mode 1: Heavy execution of hot functions
            for (int i = 0; i < iterations; i++) {
                hot_function(i);
                lib_hot_function(i % 100);
                
                if (i % 10 == 0) {
                    medium_function(i);
                    lib_complex_function(i);
                }
                
                if (i % 50 == 0) {
                    cold_function();
                    lib_cold_function();
                }
            }
            loop_function(iterations / 10);
            break;
            
        case 2:
            // Mode 2: More balanced execution with different patterns
            for (int i = 0; i < iterations / 2; i++) {
                if (i % 3 == 0) {
                    hot_function(i);
                } else if (i % 3 == 1) {
                    medium_function(i);
                    lib_complex_function(i);
                } else {
                    cold_function();
                    lib_cold_function();
                }
                
                branching_function(i);
                lib_data_transform(NULL, i % 20);
            }
            nested_loops(5);
            break;
            
        case 3:
            // Mode 3: Different execution pattern with recursion
            for (int i = 0; i < iterations / 4; i++) {
                mixed_operations(i);
                conditional_execution(i % 4);
                
                if (i % 7 == 0) {
                    lib_recursive_function(3);
                }
            }
            data_processing(100);
            break;
            
        default:
            // Default mode: Minimal execution
            hot_function(10);
            cold_function();
            lib_hot_function(5);
            lib_cold_function();
            break;
    }
    
    // Always execute utility function
    utility_function();
    
    printf("Execution complete. Global counter: %d\n", global_counter);
    return 0;
}

// Hot function - executed many times
void hot_function(int iterations) {
    int sum = 0;
    for (int i = 0; i < iterations % 100; i++) {
        sum += i;
        global_counter++;
    }
    
    // Some branching
    if (sum > 1000) {
        strcpy(buffer, "Large sum");
    } else if (sum > 100) {
        strcpy(buffer, "Medium sum");
    } else {
        strcpy(buffer, "Small sum");
    }
}

// Cold function - executed rarely
void cold_function(void) {
    static int call_count = 0;
    call_count++;
    
    // Complex but rarely executed logic
    for (int i = 0; i < 5; i++) {
        buffer[i] = 'A' + (call_count + i) % 26;
    }
    buffer[5] = '\0';
    
    // Conditional that's usually false
    if (call_count > 1000000) {
        printf("This should never happen!\n");
    }
}

// Medium frequency function
void medium_function(int count) {
    int result = 0;
    
    for (int i = 0; i < count % 20; i++) {
        result += i * i;
        
        if (i % 3 == 0) {
            result -= i;
        } else if (i % 3 == 1) {
            result += i * 2;
        }
    }
    
    global_counter += result % 100;
}

// Function with multiple branches
void branching_function(int value) {
    if (value < 0) {
        // Rare branch
        printf("Negative value: %d\n", value);
    } else if (value < 10) {
        // Common branch
        global_counter += value;
    } else if (value < 100) {
        // Another common branch
        global_counter += value * 2;
    } else {
        // Less common branch
        global_counter += value / 2;
    }
    
    // Nested condition
    if (value % 2 == 0) {
        if (value % 4 == 0) {
            buffer[0] = 'E'; // Evenly divisible by 4
        } else {
            buffer[0] = 'e'; // Even but not by 4
        }
    } else {
        buffer[0] = 'O'; // Odd
    }
}

// Loop-intensive function
void loop_function(int limit) {
    int total = 0;
    
    for (int i = 0; i < limit; i++) {
        for (int j = 0; j < 10; j++) {
            total += i * j;
            
            if (j % 2 == 0) {
                total -= j;
            }
        }
        
        if (i % 100 == 0) {
            // Infrequent operation
            printf("Progress: %d/%d\n", i, limit);
        }
    }
    
    global_counter += total % 1000;
}

// Utility function
void utility_function(void) {
    // Always executed
    static int utility_calls = 0;
    utility_calls++;
    
    // Simple operation
    for (int i = 0; i < 10; i++) {
        buffer[i] = '0' + (utility_calls + i) % 10;
    }
    buffer[10] = '\0';
}

// Data processing function
void data_processing(int size) {
    if (size <= 0) return;
    
    int *data = malloc(size * sizeof(int));
    if (!data) return;
    
    for (int i = 0; i < size; i++) {
        data[i] = i * global_counter;
        
        // Some conditional logic
        if (i % 7 == 0) {
            data[i] *= 2;
        } else if (i % 13 == 0) {
            data[i] /= 2;
        }
    }
    
    // Process data
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += data[i];
    }
    
    free(data);
    global_counter = (global_counter + sum) % 10000;
}

// Conditional execution based on mode
void conditional_execution(int mode) {
    switch (mode) {
        case 0:
            // Fast path
            hot_function(5);
            break;
        case 1:
            // Medium path
            medium_function(15);
            break;
        case 2:
            // Slow path
            loop_function(20);
            break;
        case 3:
            // Complex path
            data_processing(30);
            break;
        default:
            // Default path
            cold_function();
            break;
    }
}

// Function with nested loops
void nested_loops(int depth) {
    if (depth <= 0) return;
    
    int result = 0;
    for (int i = 0; i < depth; i++) {
        for (int j = 0; j < depth; j++) {
            for (int k = 0; k < depth; k++) {
                result += i * j * k;
                
                if (k % 2 == 0) {
                    result += 1;
                }
            }
        }
    }
    
    global_counter = (global_counter + result) % 1000;
}

// Function with mixed operations
void mixed_operations(int param) {
    // Arithmetic
    int x = param * 2 + 7;
    
    // Bit operations
    x = x ^ (param << 3);
    
    // Conditional
    if (x > 100) {
        x /= 2;
    } else if (x < 0) {
        x = -x;
    }
    
    // Loop
    for (int i = 0; i < param % 10; i++) {
        x += i;
        
        // Nested condition in loop
        if (i % 2 == 0) {
            x *= 2;
        } else {
            x -= 1;
        }
    }
    
    global_counter = (global_counter + x) % 500;
}
