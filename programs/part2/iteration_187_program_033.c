#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Function declarations from lib.c
void hot_function(int iterations);
void cold_function(void);
void medium_function(int count);
void rarely_called(void);
void path_a(void);
void path_b(void);
void mixed_function(int mode);

// Functions defined in main.c
void process_data(int iterations, int mode) {
    int i, j;
    int sum = 0;
    
    // Hot path - executed many times
    for (i = 0; i < iterations; i++) {
        sum += i;
        
        // Conditional branching
        if (i % 2 == 0) {
            for (j = 0; j < 10; j++) {
                sum += j * 2;
            }
        } else {
            sum -= 5;
        }
        
        // Nested condition
        if (i > iterations / 2) {
            sum *= 2;
        }
    }
    
    // Mode-specific behavior
    switch (mode) {
        case 1:
            // Mode 1: Call hot functions
            hot_function(iterations / 10);
            break;
        case 2:
            // Mode 2: Call cold functions
            cold_function();
            break;
        case 3:
            // Mode 3: Mixed behavior
            medium_function(iterations / 5);
            rarely_called();
            break;
        default:
            // Default: Call everything
            hot_function(iterations / 20);
            cold_function();
            medium_function(iterations / 10);
            break;
    }
    
    printf("Processed data: sum = %d\n", sum);
}

void recursive_function(int depth, int max_depth) {
    if (depth >= max_depth) {
        return;
    }
    
    // Some computation
    int local = depth * depth;
    
    // Recursive call with different paths
    if (depth % 3 == 0) {
        recursive_function(depth + 1, max_depth);
        recursive_function(depth + 2, max_depth);
    } else if (depth % 3 == 1) {
        recursive_function(depth + 1, max_depth);
    } else {
        // Do nothing for this path
    }
}

void data_transform(int *array, int size, int transform_type) {
    int i;
    
    if (transform_type == 1) {
        // Type 1: Square all elements
        for (i = 0; i < size; i++) {
            array[i] = array[i] * array[i];
            if (array[i] > 1000) {
                array[i] = 1000;  // Cap the value
            }
        }
    } else if (transform_type == 2) {
        // Type 2: Increment by index
        for (i = 0; i < size; i++) {
            array[i] += i;
            // Complex condition
            if (i > 0 && array[i] > array[i-1] * 2) {
                array[i] = array[i-1];
            }
        }
    } else {
        // Default: Reverse
        int temp;
        for (i = 0; i < size / 2; i++) {
            temp = array[i];
            array[i] = array[size - i - 1];
            array[size - i - 1] = temp;
        }
    }
}

void initialize_array(int *array, int size, int seed) {
    srand(seed);
    for (int i = 0; i < size; i++) {
        array[i] = rand() % 100;
    }
}

void analyze_array(int *array, int size) {
    int sum = 0;
    int max = array[0];
    int min = array[0];
    
    for (int i = 0; i < size; i++) {
        sum += array[i];
        if (array[i] > max) max = array[i];
        if (array[i] < min) min = array[i];
        
        // Nested loop with condition
        for (int j = 0; j < 3; j++) {
            if (array[i] % (j + 2) == 0) {
                sum += j;
            }
        }
    }
    
    printf("Array analysis: sum=%d, max=%d, min=%d\n", sum, max, min);
}

int main(int argc, char *argv[]) {
    int mode = 1;
    int iterations = 1000;
    int array_size = 100;
    
    // Parse command line arguments
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    if (argc > 3) {
        array_size = atoi(argv[3]);
    }
    
    printf("Running in mode %d with %d iterations\n", mode, iterations);
    
    // Create and process array
    int *data = malloc(array_size * sizeof(int));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Different initialization based on mode
    initialize_array(data, array_size, mode * 100);
    
    // Process data with different characteristics per mode
    process_data(iterations, mode);
    
    // Transform data differently based on mode
    data_transform(data, array_size, mode);
    
    // Analyze the array
    analyze_array(data, array_size);
    
    // Call functions from lib.c
    mixed_function(mode);
    
    // Call path functions with different frequencies
    if (mode == 1) {
        for (int i = 0; i < 10; i++) {
            path_a();
        }
        path_b();
    } else if (mode == 2) {
        path_a();
        for (int i = 0; i < 5; i++) {
            path_b();
        }
    } else {
        path_a();
        path_b();
    }
    
    // Recursive function with depth based on mode
    recursive_function(0, mode + 2);
    
    free(data);
    
    return 0;
}
