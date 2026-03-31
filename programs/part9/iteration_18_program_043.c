/* gcov_tool_test.c - Generates varied GCOV profiles for overlap analysis */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Forward declarations for functions in different compilation units */
extern void process_data(int *arr, int n, int mode);
extern void analyze_results(int *arr, int n, int threshold);

/* Hot function attribute - will be called many times */
__attribute__((hot)) void hot_loop_function(int *arr, int n, int multiplier) {
    long long sum = 0;
    for (int i = 0; i < n * multiplier; i++) {
        /* Complex branching with varying execution counts */
        if (i % 3 == 0) {
            sum += arr[i % n] * 2;
        } else if (i % 3 == 1) {
            sum += arr[i % n] / 2;
        } else {
            sum += arr[i % n] + 1;
        }
        
        /* Nested conditionals for branch coverage */
        if (sum > 1000000) {
            sum %= 1000000;
        }
    }
    printf("Hot function result: %lld\n", sum % 1000);
}

/* Cold function - rarely called */
__attribute__((cold)) void cold_function(int *arr, int n) {
    if (n <= 0) return;
    
    int min = arr[0], max = arr[0];
    for (int i = 1; i < n; i++) {
        if (arr[i] < min) min = arr[i];
        if (arr[i] > max) max = arr[i];
    }
    printf("Cold function range: %d\n", max - min);
}

/* Function with switch statement for varied coverage */
__attribute__((noinline)) void switch_based_processing(int *arr, int n, int mode) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        switch (mode % 4) {
            case 0:
                result += arr[i];
                break;
            case 1:
                result -= arr[i];
                break;
            case 2:
                result *= (arr[i] + 1);
                break;
            case 3:
                if (arr[i] != 0) result /= (arr[i] | 1);
                break;
        }
    }
    printf("Switch result: %d\n", result);
}

/* Recursive function for call graph depth */
__attribute__((noinline)) int recursive_function(int n, int depth) {
    if (depth <= 0 || n <= 1) return 1;
    
    if (n % 2 == 0) {
        return recursive_function(n / 2, depth - 1) + depth;
    } else {
        return recursive_function(3 * n + 1, depth - 1) - depth;
    }
}

/* Matrix operation for loop coverage */
void matrix_operations(int size, int intensity) {
    int matrix[100][100];
    int result = 0;
    
    /* Initialize matrix */
    for (int i = 0; i < size && i < 100; i++) {
        for (int j = 0; j < size && j < 100; j++) {
            matrix[i][j] = (i * j + intensity) % 100;
        }
    }
    
    /* Perform operations based on intensity */
    for (int iter = 0; iter < intensity; iter++) {
        for (int i = 0; i < size && i < 100; i++) {
            for (int j = 0; j < size && j < 100; j++) {
                if (iter % 3 == 0) {
                    matrix[i][j] += (i + j);
                } else if (iter % 3 == 1) {
                    matrix[i][j] *= (iter + 1);
                } else {
                    matrix[i][j] = matrix[i][j] > 50 ? matrix[i][j] : 50;
                }
                
                /* Branch with varying probability */
                if (matrix[i][j] % 7 == 0) {
                    result++;
                }
            }
        }
    }
    printf("Matrix operations complete, result: %d\n", result);
}

/* Main function with configurable execution paths */
int main(int argc, char *argv[]) {
    int seed = 42;
    int iterations = 1000;
    int algorithm = 1;
    int hot_threshold = 10000;
    int array_size = 100;
    
    /* Parse command line arguments for different execution modes */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) {
            iterations = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--algorithm") == 0 && i + 1 < argc) {
            algorithm = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--hot-threshold") == 0 && i + 1 < argc) {
            hot_threshold = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--array-size") == 0 && i + 1 < argc) {
            array_size = atoi(argv[++i]);
        }
    }
    
    printf("Running with seed=%d, iterations=%d, algorithm=%d\n", 
           seed, iterations, algorithm);
    
    /* Initialize random generator with seed for reproducible but varied runs */
    srand(seed);
    
    /* Create and initialize data array */
    int *data = malloc(array_size * sizeof(int));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    for (int i = 0; i < array_size; i++) {
        data[i] = rand() % 1000;
    }
    
    /* Execute different code paths based on algorithm choice */
    switch (algorithm) {
        case 1:
            /* Path 1: Heavy hot function usage */
            hot_loop_function(data, array_size, hot_threshold / 100);
            matrix_operations(50, iterations / 10);
            break;
            
        case 2:
            /* Path 2: Balanced execution */
            for (int i = 0; i < iterations / 100; i++) {
                hot_loop_function(data, array_size, 10);
                cold_function(data, array_size);
            }
            break;
            
        case 3:
            /* Path 3: Recursive and switch-based */
            printf("Recursive result: %d\n", 
                   recursive_function(array_size, 10));
            switch_based_processing(data, array_size, seed);
            break;
            
        case 4:
            /* Path 4: Mixed execution with varying intensity */
            for (int i = 0; i < iterations; i++) {
                if (i % 1000 == 0) {
                    cold_function(data, array_size);
                }
                if (i % 100 == 0) {
                    switch_based_processing(data, array_size, i);
                }
                if (i % 10 == 0) {
                    hot_loop_function(data, array_size, 1);
                }
            }
            break;
            
        default:
            /* Default path: All functions */
            hot_loop_function(data, array_size, hot_threshold / 50);
            cold_function(data, array_size);
            switch_based_processing(data, array_size, algorithm);
            matrix_operations(30, iterations / 100);
            printf("Recursive result: %d\n", 
                   recursive_function(array_size, 5));
    }
    
    /* Call functions from other compilation units */
    process_data(data, array_size, algorithm);
    analyze_results(data, array_size, hot_threshold);
    
    /* Calculate checksum for verification */
    long long checksum = 0;
    for (int i = 0; i < array_size; i++) {
        checksum = (checksum * 31 + data[i]) % 1000000007;
    }
    printf("Final checksum: %lld\n", checksum);
    
    free(data);
    return 0;
}
