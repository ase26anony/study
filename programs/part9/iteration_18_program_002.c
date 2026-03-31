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

/* __attribute__((noinline)) ensures functions aren't inlined */
__attribute__((noinline))
static void parse_arguments(int argc, char *argv[]) {
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
            exit(0);
        }
    }
}

/* Hot function - runs many times */
__attribute__((hot))
static void hot_function_a(int *data, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        /* Complex branching for varied profiles */
        if (data[i] > g_threshold) {
            sum += data[i] * 2;
        } else if (data[i] > g_threshold / 2) {
            sum += data[i];
        } else {
            sum += 1;
        }
        
        /* Nested condition */
        if (i % 3 == 0) {
            if (sum > 1000) {
                sum -= 500;
            }
        }
    }
    if (g_verbose) printf("Hot function A sum: %d\n", sum);
}

/* Another hot function with different pattern */
__attribute__((hot))
static void hot_function_b(int *data, int size) {
    long product = 1;
    for (int i = 0; i < size; i++) {
        switch (data[i] % 5) {
            case 0:
                product *= 2;
                break;
            case 1:
                product *= 3;
                break;
            case 2:
                product *= data[i];
                break;
            case 3:
                product /= (data[i] % 10 + 1);
                break;
            default:
                product += data[i];
        }
        
        /* Prevent overflow */
        if (product > 1000000) product = product % 1000;
    }
    if (g_verbose) printf("Hot function B product: %ld\n", product);
}

/* Cold function - runs rarely */
__attribute__((cold))
static void cold_function_a(int *data, int size) {
    if (size < 10) return;
    
    int min = data[0], max = data[0];
    for (int i = 1; i < size; i++) {
        if (data[i] < min) min = data[i];
        if (data[i] > max) max = data[i];
    }
    if (g_verbose) printf("Cold function range: %d-%d\n", min, max);
}

/* Cold function with early returns */
__attribute__((cold))
static void cold_function_b(int *data, int size) {
    if (size == 0) return;
    
    int count = 0;
    for (int i = 0; i < size; i++) {
        if (data[i] % 2 == 0) {
            count++;
            if (count > 5) break;  /* Early exit */
        }
    }
    
    if (count > 3) {
        if (g_verbose) printf("Many even numbers: %d\n", count);
    } else {
        if (g_verbose) printf("Few even numbers: %d\n", count);
    }
}

/* Function that behaves differently based on algorithm choice */
__attribute__((noinline))
static void algorithm_specific(int *data, int size) {
    switch (g_algorithm) {
        case 1:
            /* Algorithm 1: Bubble sort variant */
            for (int i = 0; i < size - 1; i++) {
                for (int j = 0; j < size - i - 1; j++) {
                    if (data[j] > data[j + 1]) {
                        int temp = data[j];
                        data[j] = data[j + 1];
                        data[j + 1] = temp;
                    }
                }
            }
            break;
            
        case 2:
            /* Algorithm 2: Selection sort variant */
            for (int i = 0; i < size - 1; i++) {
                int min_idx = i;
                for (int j = i + 1; j < size; j++) {
                    if (data[j] < data[min_idx]) {
                        min_idx = j;
                    }
                }
                if (min_idx != i) {
                    int temp = data[i];
                    data[i] = data[min_idx];
                    data[min_idx] = temp;
                }
            }
            break;
            
        case 3:
            /* Algorithm 3: Insertion sort variant */
            for (int i = 1; i < size; i++) {
                int key = data[i];
                int j = i - 1;
                while (j >= 0 && data[j] > key) {
                    data[j + 1] = data[j];
                    j--;
                }
                data[j + 1] = key;
            }
            break;
            
        default:
            /* Default: No sorting */
            break;
    }
}

/* Main execution flow */
int main(int argc, char *argv[]) {
    parse_arguments(argc, argv);
    
    /* Initialize random data based on seed */
    srand(g_seed);
    const int data_size = 100;
    int data[data_size];
    
    for (int i = 0; i < data_size; i++) {
        data[i] = rand() % 1000;
    }
    
    /* Create a copy for processing */
    int work_data[data_size];
    
    /* Main processing loop - creates hot paths */
    int total = 0;
    for (int iter = 0; iter < g_iterations; iter++) {
        /* Copy fresh data each iteration */
        memcpy(work_data, data, sizeof(data));
        
        /* Vary processing based on iteration count */
        if (iter % 3 == 0) {
            hot_function_a(work_data, data_size);
        } else if (iter % 3 == 1) {
            hot_function_b(work_data, data_size);
        } else {
            /* Call external functions from other compilation units */
            if (iter < g_iterations / 2) {
                process_data_hot(work_data, data_size, g_threshold);
            } else {
                process_data_cold(work_data, data_size);
            }
        }
        
        /* Occasionally call cold functions */
        if (iter % 100 == 0) {
            cold_function_a(work_data, data_size);
        }
        if (iter % 250 == 0) {
            cold_function_b(work_data, data_size);
        }
        
        /* Apply algorithm-specific processing */
        algorithm_specific(work_data, data_size);
        
        /* Accumulate checksum */
        total += checksum(work_data, data_size);
        
        /* Modify threshold occasionally */
        if (iter % 500 == 0) {
            g_threshold = (g_threshold + 50) % 800;
        }
    }
    
    /* Final processing */
    int final_data[data_size];
    for (int i = 0; i < data_size; i++) {
        final_data[i] = data[i] + (total % 100);
    }
    
    /* One more round of processing */
    hot_function_a(final_data, data_size);
    cold_function_a(final_data, data_size);
    
    /* Output deterministic result for verification */
    printf("Result: %d (seed=%d, iterations=%d, algorithm=%d)\n", 
           total, g_seed, g_iterations, g_algorithm);
    
    return 0;
}
