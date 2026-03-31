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
void helper_function(int x);
void recursive_function(int depth, int max);
void switch_based(int value);
void loop_variations(int type);

// External functions from lib.c
extern void lib_hot_function(int n);
extern void lib_cold_function(void);
extern void lib_medium_work(int iterations);
extern int lib_compute(int a, int b);
extern void lib_rare_path(int flag);

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
    
    printf("Running in mode %d with %d iterations\n", mode, iterations);
    
    // Seed random number generator
    srand(time(NULL) ^ mode);
    
    // Vary execution based on mode
    switch (mode) {
        case 1:
            // Mode 1: Heavy execution of hot paths
            hot_function(iterations * 10);
            medium_function(iterations / 2);
            lib_hot_function(iterations * 5);
            lib_medium_work(iterations);
            
            // Execute cold functions rarely
            if (iterations % 100 == 0) {
                cold_function();
                lib_cold_function();
            }
            break;
            
        case 2:
            // Mode 2: More balanced execution
            for (int i = 0; i < iterations; i++) {
                hot_function(10);
                if (i % 10 == 0) {
                    medium_function(5);
                }
                if (i % 50 == 0) {
                    cold_function();
                }
            }
            lib_medium_work(iterations / 2);
            break;
            
        case 3:
            // Mode 3: Different path selection
            path_selector(mode);
            for (int i = 0; i < iterations; i++) {
                switch_based(i % 5);
                loop_variations(i % 3);
            }
            lib_rare_path(1);
            break;
            
        default:
            // Default mode: Mixed execution
            hot_function(iterations);
            medium_function(iterations / 10);
            cold_function();
            rarely_called();
            lib_hot_function(iterations);
            lib_cold_function();
            break;
    }
    
    // Always execute some common code
    int data[100];
    for (int i = 0; i < 100; i++) {
        data[i] = rand() % 1000;
    }
    process_data(data, 100);
    
    // Call recursive function with varying depth
    recursive_function(0, mode + 2);
    
    // Call helper functions
    for (int i = 0; i < iterations % 100; i++) {
        helper_function(i);
    }
    
    printf("Mode %d execution completed\n", mode);
    return 0;
}

void hot_function(int iterations) {
    int sum = 0;
    // Hot loop - executed many times
    for (int i = 0; i < iterations; i++) {
        sum += i;
        if (i % 100 == 0) {
            sum -= i / 2;
        }
    }
    
    // Nested loop
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            sum += i * j;
        }
    }
    
    // Conditional execution
    if (sum > 1000000) {
        sum /= 2;
    } else if (sum > 100000) {
        sum -= 1000;
    } else {
        sum += 1000;
    }
}

void cold_function(void) {
    // Rarely executed function
    static int call_count = 0;
    call_count++;
    
    // Complex but rarely used logic
    int result = 1;
    for (int i = 1; i <= 10; i++) {
        result *= i;
    }
    
    // Multiple exit points
    if (call_count % 3 == 0) {
        return;
    }
    
    // Unusual control flow
    goto skip;
    
    // This code is never reached
    result += 100;
    
skip:
    // Do nothing
    ;
}

void medium_function(int count) {
    // Medium frequency function
    if (count <= 0) {
        return;
    }
    
    int total = 0;
    for (int i = 0; i < count; i++) {
        total += i * i;
        
        // Inner condition
        if (i % 7 == 0) {
            total -= i;
        } else if (i % 3 == 0) {
            total += i * 2;
        }
    }
    
    // Switch statement
    switch (count % 4) {
        case 0:
            total += 100;
            break;
        case 1:
            total -= 50;
            break;
        case 2:
            total *= 2;
            break;
        case 3:
            total /= 2;
            break;
    }
}

void rarely_called(void) {
    // Very rarely called function
    // Complex logic that's almost never executed
    int matrix[5][5];
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            matrix[i][j] = i * j;
            
            // Deeply nested condition
            if (i > j) {
                if (j > 0) {
                    if (i % 2 == 0) {
                        matrix[i][j] += 1;
                    }
                }
            }
        }
    }
}

void path_selector(int mode) {
    // Function with multiple execution paths
    switch (mode) {
        case 1:
            // Path 1 - frequently taken
            hot_function(100);
            break;
        case 2:
            // Path 2 - less frequent
            medium_function(50);
            break;
        case 3:
            // Path 3 - rare
            cold_function();
            break;
        default:
            // Default path
            rarely_called();
            break;
    }
}

void process_data(int *data, int size) {
    if (!data || size <= 0) {
        return;
    }
    
    int min = data[0];
    int max = data[0];
    long long sum = 0;
    
    for (int i = 0; i < size; i++) {
        sum += data[i];
        
        if (data[i] < min) {
            min = data[i];
        }
        
        if (data[i] > max) {
            max = data[i];
        }
        
        // Conditional processing
        if (data[i] % 2 == 0) {
            data[i] *= 2;
        } else {
            data[i] += 1;
        }
    }
    
    // Calculate average
    int avg = (size > 0) ? (sum / size) : 0;
}

void helper_function(int x) {
    // Small helper function
    static int counter = 0;
    counter++;
    
    // Simple transformation
    x = x * 2 + 1;
    
    // Always executed
    if (x < 0) {
        x = -x;
    }
}

void recursive_function(int depth, int max) {
    if (depth >= max) {
        return;
    }
    
    // Do some work
    int result = depth * depth;
    
    // Recursive call
    recursive_function(depth + 1, max);
    
    // More work after recursion
    result += depth;
}

void switch_based(int value) {
    // Function with switch statement
    switch (value) {
        case 0:
            // Most common case
            helper_function(1);
            break;
        case 1:
            // Common case
            helper_function(2);
            break;
        case 2:
            // Less common
            medium_function(3);
            break;
        case 3:
            // Rare case
            cold_function();
            break;
        case 4:
            // Very rare case
            rarely_called();
            break;
        default:
            // Default case (shouldn't happen)
            helper_function(value);
            break;
    }
}

void loop_variations(int type) {
    // Different loop structures
    switch (type) {
        case 0:
            // Simple for loop
            for (int i = 0; i < 10; i++) {
                helper_function(i);
            }
            break;
            
        case 1:
            // While loop
            int i = 0;
            while (i < 5) {
                medium_function(i);
                i++;
            }
            break;
            
        case 2:
            // Do-while loop
            int j = 0;
            do {
                helper_function(j * 2);
                j++;
            } while (j < 3);
            break;
            
        default:
            // Nested loops
            for (int x = 0; x < 3; x++) {
                for (int y = 0; y < 3; y++) {
                    helper_function(x + y);
                }
            }
            break;
    }
}
