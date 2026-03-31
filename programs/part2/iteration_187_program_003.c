#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Function declarations
void hot_function_1(int iterations);
void hot_function_2(int iterations);
void cold_function_1(void);
void cold_function_2(void);
void mixed_function(int mode);
void recursive_function(int depth, int max_depth);
void loop_with_condition(int limit, int threshold);
void switch_based_function(int value);
void file_io_simulation(int count);
void memory_operation(int size);

// External functions from lib.c
extern void lib_hot_function(int iterations);
extern void lib_cold_function(void);
extern void lib_mixed_operation(int mode);
extern double lib_compute_value(int base, int multiplier);
extern int lib_process_array(int *arr, int size);

int main(int argc, char *argv[]) {
    int mode = 1;
    int iterations = 1000;
    
    // Parse command line argument for mode
    if (argc > 1) {
        mode = atoi(argv[1]);
        if (mode < 1) mode = 1;
        if (mode > 3) mode = 3;
    }
    
    if (argc > 2) {
        iterations = atoi(argv[2]);
        if (iterations < 100) iterations = 100;
        if (iterations > 10000) iterations = 10000;
    }
    
    srand(time(NULL));
    
    printf("Running in mode %d with %d iterations\n", mode, iterations);
    
    // Vary execution based on mode to create different profiles
    switch (mode) {
        case 1:
            // Mode 1: Heavy on hot functions
            hot_function_1(iterations * 2);
            hot_function_2(iterations);
            lib_hot_function(iterations * 3);
            break;
            
        case 2:
            // Mode 2: Balanced mix
            for (int i = 0; i < iterations / 10; i++) {
                hot_function_1(10);
                cold_function_1();
                mixed_function(i % 3);
            }
            lib_mixed_operation(mode);
            break;
            
        case 3:
            // Mode 3: More cold paths and recursion
            cold_function_1();
            cold_function_2();
            lib_cold_function();
            recursive_function(0, 5);
            for (int i = 0; i < iterations / 100; i++) {
                hot_function_1(5);
            }
            break;
    }
    
    // Common operations across all modes
    loop_with_condition(iterations / 2, iterations / 4);
    switch_based_function(mode * 10);
    
    // Call lib.c functions
    double result = lib_compute_value(mode, iterations);
    printf("Computed value: %.2f\n", result);
    
    // Memory operations
    int arr_size = 100;
    int *arr = malloc(arr_size * sizeof(int));
    if (arr) {
        for (int i = 0; i < arr_size; i++) {
            arr[i] = i * mode;
        }
        int processed = lib_process_array(arr, arr_size);
        printf("Processed %d array elements\n", processed);
        free(arr);
    }
    
    // File I/O simulation
    file_io_simulation(mode * 5);
    
    // Final memory operation
    memory_operation(mode * 50);
    
    return 0;
}

// Hot function - executed many times
void hot_function_1(int iterations) {
    int sum = 0;
    for (int i = 0; i < iterations; i++) {
        sum += i;
        if (i % 100 == 0) {
            sum -= 50;  // Cold path within hot function
        }
    }
    // Unused variable warning suppression
    (void)sum;
}

// Another hot function
void hot_function_2(int iterations) {
    long product = 1;
    for (int i = 1; i <= iterations % 100; i++) {
        product *= i;
        if (product > 1000000) {
            product = 1;  // Reset condition
        }
    }
    // Unused variable warning suppression
    (void)product;
}

// Cold function - executed rarely
void cold_function_1(void) {
    printf("Cold function 1 executed\n");
    // Simulate some computation
    volatile int x = 0;
    for (int i = 0; i < 10; i++) {
        x += i * 2;
    }
}

// Another cold function
void cold_function_2(void) {
    // This function has multiple branches that are rarely taken
    int r = rand() % 1000;
    if (r < 10) {
        printf("Very rare branch taken: %d\n", r);
    } else if (r < 100) {
        printf("Somewhat rare branch: %d\n", r);
    } else {
        // Most common path, but function itself is cold
    }
}

// Function with mixed hot/cold paths
void mixed_function(int mode) {
    if (mode == 0) {
        // Hot path
        for (int i = 0; i < 100; i++) {
            volatile int x = i * i;
            (void)x;
        }
    } else if (mode == 1) {
        // Warm path
        printf("Mixed function mode 1\n");
    } else {
        // Cold path
        printf("Mixed function other mode: %d\n", mode);
    }
}

// Recursive function
void recursive_function(int depth, int max_depth) {
    if (depth >= max_depth) {
        return;
    }
    
    // Vary recursion pattern
    if (depth % 2 == 0) {
        recursive_function(depth + 1, max_depth);
    } else {
        recursive_function(depth + 2, max_depth);
        if (depth < max_depth - 1) {
            recursive_function(depth + 1, max_depth);
        }
    }
}

// Function with loop and condition
void loop_with_condition(int limit, int threshold) {
    int hot_count = 0;
    int cold_count = 0;
    
    for (int i = 0; i < limit; i++) {
        if (i < threshold) {
            // Hot path
            hot_count++;
        } else {
            // Cold path
            cold_count++;
            if (i % 7 == 0) {
                // Even colder path
                printf("Special condition at i=%d\n", i);
            }
        }
    }
    
    printf("Loop results: hot=%d, cold=%d\n", hot_count, cold_count);
}

// Switch-based function
void switch_based_function(int value) {
    switch (value % 5) {
        case 0:
            // Most common case
            break;
        case 1:
            printf("Case 1: %d\n", value);
            break;
        case 2:
            // Rare case
            if (value > 100) {
                printf("Large value case 2\n");
            }
            break;
        case 3:
            // Another case
            break;
        case 4:
            // Default-like case
            printf("Case 4 executed\n");
            break;
    }
}

// Simulate file I/O operations
void file_io_simulation(int count) {
    for (int i = 0; i < count; i++) {
        // Simulate file operations
        if (i % 3 == 0) {
            printf("Simulating file read %d\n", i);
        } else if (i % 3 == 1) {
            printf("Simulating file write %d\n", i);
        } else {
            // Error simulation (cold path)
            printf("Simulating file error %d\n", i);
        }
    }
}

// Memory operations
void memory_operation(int size) {
    if (size <= 0) return;
    
    char *buffer = malloc(size);
    if (!buffer) {
        printf("Memory allocation failed\n");
        return;
    }
    
    // Use the buffer
    for (int i = 0; i < size && i < 1000; i++) {
        buffer[i] = i % 256;
    }
    
    // Sometimes free, sometimes leak (for testing)
    if (size % 2 == 0) {
        free(buffer);
    } else {
        printf("Buffer of size %d allocated but not freed (simulated leak)\n", size);
    }
}
