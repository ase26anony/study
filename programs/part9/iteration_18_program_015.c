/* gcov_tool_test.c - Multi-run profile generator for gcov-tool overlap analysis */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

/* Forward declarations for functions in different compilation units */
extern void process_data_hot(int *data, int size, int threshold);
extern void process_data_cold(int *data, int size);

/* Global configuration */
static int g_verbose = 0;
static int g_iterations = 1000;
static int g_seed = 42;
static int g_algorithm = 1;
static int g_threshold = 500;

/* Hot function - runs many times */
__attribute__((hot, noinline))
void hot_function_a(int *data, int size) {
    long long sum = 0;
    for (int i = 0; i < size; i++) {
        if (data[i] > g_threshold) {
            sum += data[i] * 2;  /* Hot path */
        } else {
            sum += data[i];      /* Cold path */
        }
        
        /* Nested condition for branch coverage */
        if (i % 3 == 0) {
            if (data[i] % 2 == 0) {
                sum += 1;
            } else {
                sum -= 1;
            }
        }
    }
    
    if (g_verbose) {
        printf("Hot function A sum: %lld\n", sum);
    }
}

/* Another hot function with different pattern */
__attribute__((hot, noinline))
void hot_function_b(int *data, int size) {
    int count = 0;
    for (int i = 0; i < size; i++) {
        switch (data[i] % 5) {
            case 0: count += 1; break;
            case 1: count += 2; break;
            case 2: count += 3; break;
            case 3: count += 4; break;
            case 4: count += 5; break;
            default: count += 0; break;
        }
        
        /* Complex nested if-else */
        if (data[i] > 100) {
            if (data[i] < 200) {
                count++;
            } else if (data[i] < 300) {
                count += 2;
            } else {
                count += 3;
            }
        }
    }
    
    if (g_verbose) {
        printf("Hot function B count: %d\n", count);
    }
}

/* Cold function - runs rarely */
__attribute__((cold, noinline))
void cold_function_a(int *data, int size) {
    if (size < 10) {
        printf("Warning: small array in cold function\n");
        return;
    }
    
    /* This path is rarely taken */
    for (int i = 0; i < 3; i++) {
        data[i] = data[size - 1 - i];
    }
}

/* Medium frequency function */
__attribute__((noinline))
void medium_function(int *data, int size, int multiplier) {
    int result = 0;
    
    for (int i = 0; i < size; i++) {
        /* Different execution based on algorithm choice */
        if (g_algorithm == 1) {
            result += data[i] * multiplier;
        } else if (g_algorithm == 2) {
            result += data[i] / (multiplier + 1);
        } else {
            result += abs(data[i] - multiplier);
        }
        
        /* Loop with early exit */
        if (result > 1000000) {
            break;
        }
    }
    
    if (g_verbose && result > 0) {
        printf("Medium function result: %d\n", result);
    }
}

/* Function with many branches */
__attribute__((noinline))
void branchy_function(int *data, int size) {
    int branches_taken = 0;
    
    for (int i = 0; i < size; i++) {
        /* Multiple independent conditions */
        if (data[i] % 2 == 0) branches_taken++;
        if (data[i] % 3 == 0) branches_taken++;
        if (data[i] % 5 == 0) branches_taken++;
        if (data[i] % 7 == 0) branches_taken++;
        if (data[i] % 11 == 0) branches_taken++;
        
        /* Chained if-else */
        if (data[i] < 0) {
            branches_taken += 10;
        } else if (data[i] < 100) {
            branches_taken += 20;
        } else if (data[i] < 200) {
            branches_taken += 30;
        } else {
            branches_taken += 40;
        }
    }
    
    if (branches_taken > size * 10) {
        printf("Many branches taken: %d\n", branches_taken);
    }
}

/* Main processing function */
__attribute__((noinline))
void process_with_algorithm(int *data, int size, int algo) {
    switch (algo) {
        case 1:
            /* Algorithm 1: Focus on hot functions */
            for (int i = 0; i < g_iterations / 10; i++) {
                hot_function_a(data, size);
                hot_function_b(data, size);
            }
            medium_function(data, size, 2);
            break;
            
        case 2:
            /* Algorithm 2: More balanced */
            for (int i = 0; i < g_iterations / 100; i++) {
                hot_function_a(data, size);
            }
            for (int i = 0; i < g_iterations / 50; i++) {
                hot_function_b(data, size);
            }
            medium_function(data, size, 3);
            branchy_function(data, size);
            break;
            
        case 3:
            /* Algorithm 3: Cold path focused */
            hot_function_a(data, size);
            cold_function_a(data, size);
            for (int i = 0; i < g_iterations / 1000; i++) {
                medium_function(data, size, 5);
            }
            branchy_function(data, size);
            break;
            
        default:
            /* Default: Mix of everything */
            hot_function_a(data, size);
            hot_function_b(data, size);
            medium_function(data, size, 1);
            branchy_function(data, size);
            cold_function_a(data, size);
            break;
    }
}

/* Function to generate test data */
void generate_data(int *data, int size, int seed) {
    srand(seed);
    for (int i = 0; i < size; i++) {
        data[i] = rand() % 1000;
    }
}

/* Function to compute checksum (prevents dead code elimination) */
long long compute_checksum(int *data, int size) {
    long long checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum = checksum * 31 + data[i];
    }
    return checksum;
}

/* Main function with configurable execution paths */
int main(int argc, char *argv[]) {
    /* Parse command line arguments for different execution modes */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            g_seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) {
            g_iterations = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--algorithm") == 0 && i + 1 < argc) {
            g_algorithm = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--threshold") == 0 && i + 1 < argc) {
            g_threshold = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--verbose") == 0) {
            g_verbose = 1;
        } else if (strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [--seed N] [--iterations M] [--algorithm A] [--threshold T] [--verbose]\n", argv[0]);
            printf("  --seed N        : Random seed (default: 42)\n");
            printf("  --iterations M  : Number of iterations (default: 1000)\n");
            printf("  --algorithm A   : Algorithm 1-3 (default: 1)\n");
            printf("  --threshold T   : Hot/cold threshold (default: 500)\n");
            printf("  --verbose       : Enable verbose output\n");
            return 0;
        }
    }
    
    const int data_size = 1000;
    int *data = malloc(data_size * sizeof(int));
    
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Generate data with specified seed */
    generate_data(data, data_size, g_seed);
    
    /* Process data using selected algorithm */
    process_with_algorithm(data, data_size, g_algorithm);
    
    /* Call functions from other compilation units */
    process_data_hot(data, data_size, g_threshold);
    process_data_cold(data, data_size);
    
    /* Compute and output checksum (ensures all code affects output) */
    long long checksum = compute_checksum(data, data_size);
    printf("Result checksum: %lld\n", checksum);
    
    free(data);
    return 0;
}
