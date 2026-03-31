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
void loop_variations(int type, int limit);
void mixed_operations(int value);

// External functions from lib.c
extern void lib_hot_function(int iterations);
extern void lib_cold_function(void);
extern void lib_complex_function(int mode);
extern int lib_calculate(int a, int b, int op);
extern void lib_data_transform(int *arr, int size);

// Global variables for profiling
static int global_counter = 0;
static int data_array[100];

int main(int argc, char *argv[]) {
    int mode = 1;
    int iterations = 1000;
    
    // Parse command line argument for mode
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    srand(time(NULL));
    
    printf("Running in mode %d with %d iterations\n", mode, iterations);
    
    // Different execution paths based on mode
    switch (mode) {
        case 1:
            // Mode 1: Heavy execution of hot functions
            for (int i = 0; i < iterations; i++) {
                hot_function(i % 100);
                lib_hot_function(i % 50);
                
                if (i % 10 == 0) {
                    medium_function(i);
                }
                
                if (i % 100 == 0) {
                    cold_function();
                    lib_cold_function();
                }
            }
            break;
            
        case 2:
            // Mode 2: More balanced execution
            for (int i = 0; i < iterations / 2; i++) {
                hot_function(i % 20);
                lib_hot_function(i % 30);
                
                if (i % 5 == 0) {
                    medium_function(i);
                    lib_complex_function(i % 3);
                }
                
                if (i % 50 == 0) {
                    cold_function();
                }
            }
            
            // Additional operations in mode 2
            process_data(data_array, 100);
            lib_data_transform(data_array, 100);
            break;
            
        case 3:
            // Mode 3: Different execution pattern
            for (int i = 0; i < iterations / 4; i++) {
                if (i % 3 == 0) {
                    hot_function(i);
                } else if (i % 3 == 1) {
                    lib_hot_function(i);
                } else {
                    medium_function(i);
                }
                
                loop_variations(i % 4, 50);
                mixed_operations(i);
            }
            
            recursive_function(0, 5);
            break;
            
        default:
            // Default mode: Minimal execution
            hot_function(10);
            lib_hot_function(5);
            rarely_called();
            break;
    }
    
    // Always execute path_selector
    path_selector(mode);
    
    // Some calculations using lib functions
    for (int i = 0; i < 10; i++) {
        int result = lib_calculate(i, i * 2, i % 4);
        global_counter += result;
    }
    
    printf("Execution complete. Global counter: %d\n", global_counter);
    
    return 0;
}

// Hot function - executed many times
void hot_function(int iterations) {
    int local_sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        local_sum += i;
        
        // Nested loop for more complexity
        for (int j = 0; j < 5; j++) {
            local_sum += j * i;
        }
    }
    
    global_counter += local_sum % 1000;
}

// Cold function - rarely executed
void cold_function(void) {
    static int call_count = 0;
    call_count++;
    
    // Complex but rarely executed path
    int temp = 0;
    for (int i = 0; i < 100; i++) {
        temp += i * call_count;
        
        if (i % 7 == 0) {
            temp -= i;
        } else if (i % 13 == 0) {
            temp *= 2;
        }
    }
    
    printf("Cold function called %d times\n", call_count);
}

// Medium frequency function
void medium_function(int count) {
    int result = count;
    
    for (int i = 0; i < count % 20; i++) {
        result = (result * 3 + 7) % 100;
        
        if (result > 50) {
            result /= 2;
        } else {
            result += 25;
        }
    }
    
    global_counter += result;
}

// Rarely called function
void rarely_called(void) {
    // This function has multiple branches but is rarely called
    int x = rand() % 100;
    
    if (x < 10) {
        printf("Case 1: %d\n", x);
    } else if (x < 30) {
        printf("Case 2: %d\n", x);
    } else if (x < 60) {
        printf("Case 3: %d\n", x);
    } else if (x < 90) {
        printf("Case 4: %d\n", x);
    } else {
        printf("Case 5: %d\n", x);
    }
}

// Function with multiple paths
void path_selector(int mode) {
    switch (mode % 4) {
        case 0:
            printf("Path A selected\n");
            for (int i = 0; i < 10; i++) {
                global_counter += i;
            }
            break;
            
        case 1:
            printf("Path B selected\n");
            for (int i = 0; i < 20; i += 2) {
                global_counter -= i;
            }
            break;
            
        case 2:
            printf("Path C selected\n");
            for (int i = 0; i < 15; i += 3) {
                global_counter *= (i + 1);
                if (global_counter > 1000) global_counter %= 1000;
            }
            break;
            
        case 3:
            printf("Path D selected\n");
            for (int i = 0; i < 5; i++) {
                global_counter += rand() % 100;
            }
            break;
    }
}

// Function that processes data
void process_data(int *data, int size) {
    if (size <= 0 || data == NULL) return;
    
    for (int i = 0; i < size; i++) {
        data[i] = i * 2;
        
        if (i % 3 == 0) {
            data[i] += 5;
        } else if (i % 3 == 1) {
            data[i] -= 3;
        } else {
            data[i] *= 2;
        }
        
        // Nested condition
        if (data[i] > 100) {
            data[i] %= 100;
        }
    }
}

// Recursive function
void recursive_function(int depth, int max_depth) {
    if (depth >= max_depth) {
        return;
    }
    
    global_counter += depth;
    
    // Sometimes recurse twice
    if (depth % 2 == 0) {
        recursive_function(depth + 1, max_depth);
        recursive_function(depth + 2, max_depth);
    } else {
        recursive_function(depth + 1, max_depth);
    }
}

// Function with different loop types
void loop_variations(int type, int limit) {
    switch (type) {
        case 0:
            // Simple for loop
            for (int i = 0; i < limit; i++) {
                global_counter += i;
            }
            break;
            
        case 1:
            // While loop
            int i = 0;
            while (i < limit) {
                global_counter -= i;
                i++;
            }
            break;
            
        case 2:
            // Do-while loop
            int j = 0;
            do {
                global_counter *= (j + 1);
                if (global_counter > 10000) global_counter %= 1000;
                j++;
            } while (j < limit);
            break;
            
        case 3:
            // Nested loops
            for (int x = 0; x < limit / 10; x++) {
                for (int y = 0; y < 10; y++) {
                    global_counter += x * y;
                }
            }
            break;
    }
}

// Function with mixed operations
void mixed_operations(int value) {
    int temp = value;
    
    // Multiple if-else chains
    if (temp < 0) {
        temp = -temp;
    } else if (temp == 0) {
        temp = 1;
    }
    
    // Switch statement
    switch (temp % 4) {
        case 0:
            temp += 10;
            break;
        case 1:
            temp *= 2;
            break;
        case 2:
            temp /= 2;
            break;
        case 3:
            temp -= 5;
            break;
    }
    
    // Loop with break
    for (int i = 0; i < 20; i++) {
        if (i > temp) {
            break;
        }
        global_counter += i;
    }
    
    // Conditional operator
    int result = (temp > 50) ? temp * 2 : temp / 2;
    global_counter += result;
}
