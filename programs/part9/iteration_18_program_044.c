/* gcov_tool_test.c - Multi-run profile generator for gcov-tool overlap analysis */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Forward declarations for functions in different compilation units */
extern void process_data_hot(int *data, int size, int threshold);
extern void process_data_cold(int *data, int size);
extern int checksum(const int *data, int size);

/* Global configuration */
static int g_verbose = 0;
static int g_iterations = 1000;
static int g_seed = 42;
static int g_algorithm = 1;
static int g_threshold = 500;

/* Function targeting -f (function-level) and -F (fullname) */
__attribute__((noinline))
static void helper_function_a(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        if (arr[i] > g_threshold) {
            arr[i] = arr[i] * 2;
        } else if (arr[i] < -g_threshold) {
            arr[i] = arr[i] / 2;
        } else {
            arr[i] = arr[i] + 1;
        }
    }
}

__attribute__((noinline))
static void helper_function_b(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        switch (arr[i] % 5) {
            case 0: sum += arr[i]; break;
            case 1: sum -= arr[i]; break;
            case 2: sum *= 2; break;
            case 3: sum /= 2; break;
            default: sum = sum ^ arr[i];
        }
    }
    if (g_verbose) printf("Intermediate sum: %d\n", sum);
}

/* Hot function for -h and -t options */
__attribute__((hot, noinline))
void hot_loop_processor(int *data, int size) {
    long long total = 0;
    for (int iter = 0; iter < g_iterations; iter++) {
        for (int i = 0; i < size; i++) {
            if (data[i] > 0) {
                total += data[i];
                if (data[i] > 1000) total += data[i] * 2;
            } else {
                total -= data[i];
            }
            /* Nested condition for branch coverage */
            if (i % 2 == 0) {
                if (i % 3 == 0) {
                    total += 1;
                } else {
                    total -= 1;
                }
            }
        }
    }
    if (g_verbose) printf("Hot loop total: %lld\n", total);
}

/* Cold function */
__attribute__((cold, noinline))
void cold_function(int *data, int size) {
    if (size < 2) return;
    
    int temp = data[0];
    data[0] = data[size-1];
    data[size-1] = temp;
    
    if (g_verbose) printf("Cold function swapped endpoints\n");
}

/* Algorithm variants for different runs */
__attribute__((noinline))
void algorithm_variant1(int *data, int size) {
    for (int i = 0; i < size-1; i++) {
        for (int j = 0; j < size-i-1; j++) {
            if (data[j] > data[j+1]) {
                int temp = data[j];
                data[j] = data[j+1];
                data[j+1] = temp;
            }
        }
    }
}

__attribute__((noinline))
void algorithm_variant2(int *data, int size) {
    /* Quick sort style partitioning */
    if (size <= 1) return;
    
    int pivot = data[size/2];
    int i = 0, j = size-1;
    
    while (i <= j) {
        while (data[i] < pivot) i++;
        while (data[j] > pivot) j--;
        if (i <= j) {
            int temp = data[i];
            data[i] = data[j];
            data[j] = temp;
            i++;
            j--;
        }
    }
    
    algorithm_variant2(data, j+1);
    algorithm_variant2(data + i, size - i);
}

/* Dead code for different build variants */
#ifdef VARIANT_A
__attribute__((noinline))
void variant_a_specific(int *data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 3 + 7;
    }
}
#endif

#ifdef VARIANT_B
__attribute__((noinline))
void variant_b_specific(int *data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] / 2 - 5;
    }
}
#endif

/* Main processing function */
__attribute__((noinline))
void process_with_config(int *data, int size) {
    /* Call helper functions */
    helper_function_a(data, size);
    helper_function_b(data, size);
    
    /* Choose algorithm based on configuration */
    if (g_algorithm == 1) {
        algorithm_variant1(data, size);
    } else if (g_algorithm == 2) {
        algorithm_variant2(data, size);
    } else {
        /* Hybrid approach */
        algorithm_variant1(data, size/2);
        algorithm_variant2(data + size/2, size - size/2);
    }
    
    /* Call hot/cold functions */
    hot_loop_processor(data, size);
    cold_function(data, size);
    
    /* Build variant specific code */
    #ifdef VARIANT_A
    variant_a_specific(data, size);
    #endif
    
    #ifdef VARIANT_B
    variant_b_specific(data, size);
    #endif
}

/* Parse command line arguments */
void parse_args(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--seed") == 0 && i+1 < argc) {
            g_seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--iterations") == 0 && i+1 < argc) {
            g_iterations = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--algorithm") == 0 && i+1 < argc) {
            g_algorithm = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--threshold") == 0 && i+1 < argc) {
            g_threshold = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--verbose") == 0) {
            g_verbose = 1;
        } else if (strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [--seed N] [--iterations M] [--algorithm 1|2|3] [--threshold T] [--verbose]\n", argv[0]);
            exit(0);
        }
    }
}

int main(int argc, char *argv[]) {
    parse_args(argc, argv);
    
    /* Initialize with seed for reproducibility */
    srand(g_seed);
    
    /* Create data array with varying sizes based on seed */
    int data_size = 100 + (g_seed % 901); /* 100 to 1000 elements */
    int *data = malloc(data_size * sizeof(int));
    
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Fill with random data */
    for (int i = 0; i < data_size; i++) {
        data[i] = rand() % 2000 - 1000; /* -1000 to 1000 */
    }
    
    /* Process data */
    process_with_config(data, data_size);
    
    /* Call external functions */
    process_data_hot(data, data_size, g_threshold);
    process_data_cold(data, data_size);
    
    /* Calculate and output checksum */
    int result = checksum(data, data_size);
    printf("Result checksum: %d (seed=%d, iterations=%d, algorithm=%d)\n", 
           result, g_seed, g_iterations, g_algorithm);
    
    free(data);
    return 0;
}
