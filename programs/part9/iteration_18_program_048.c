/* gcov_overlap_test.c - Test program for gcov-tool overlap analysis */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Forward declarations for functions in different compilation units */
extern void process_data(int *arr, int n, int mode);
extern void analyze_results(int *arr, int n, int threshold);
extern void cold_path_operation(int *arr, int n);

/* Global configuration */
static int verbose = 0;
static int hot_threshold = 1000;

/* ========== HOT FUNCTIONS (high execution count) ========== */

/* __attribute__((hot)) ensures compiler treats this as hot code */
__attribute__((hot, noinline))
static void hot_loop_processor(int *arr, int n, int multiplier) {
    long long sum = 0;
    /* This loop runs many times - will be "hot" */
    for (int i = 0; i < n * multiplier; i++) {
        int idx = i % n;
        arr[idx] = (arr[idx] * 1103515245 + 12345) & 0x7fffffff;
        sum += arr[idx];
        
        /* Branch with different probabilities across runs */
        if (arr[idx] % 3 == 0) {
            arr[idx] >>= 1;
        } else if (arr[idx] % 7 == 0) {
            arr[idx] <<= 1;
        }
        
        /* Nested conditionals for complex coverage */
        if (i % 100 == 0) {
            if (arr[idx] > 1000000) {
                arr[idx] = 1000000;
            }
        }
    }
    
    if (verbose) {
        printf("Hot loop processed %d iterations, final sum: %lld\n", 
               n * multiplier, sum);
    }
}

__attribute__((hot, noinline))
static void matrix_operations(int size, int seed) {
    /* Create and process a matrix - another hot function */
    int *matrix = malloc(size * size * sizeof(int));
    if (!matrix) return;
    
    srand(seed);
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            matrix[i*size + j] = rand() % 1000;
        }
    }
    
    /* Perform matrix operations */
    long long trace = 0;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (i == j) {
                trace += matrix[i*size + j];
            }
            /* Conditional with varying probability */
            if (matrix[i*size + j] > 500) {
                matrix[i*size + j] -= 250;
            } else {
                matrix[i*size + j] += 250;
            }
        }
    }
    
    free(matrix);
}

/* ========== COLD FUNCTIONS (low execution count) ========== */

__attribute__((cold, noinline))
static void initialization_routine(int *arr, int n, int seed) {
    /* Cold function - runs once per execution */
    srand(seed);
    for (int i = 0; i < n; i++) {
        arr[i] = rand() % 10000;
    }
    
    /* Rarely taken branch */
    if (seed == 0) {
        for (int i = 0; i < n; i++) {
            arr[i] = i;
        }
    }
}

__attribute__((cold, noinline))
static void validation_check(int *arr, int n) {
    /* Another cold function */
    int errors = 0;
    for (int i = 0; i < n; i++) {
        if (arr[i] < 0) errors++;
    }
    
    if (errors > 0 && verbose) {
        printf("Validation found %d negative values\n", errors);
    }
}

/* ========== MEDIUM FREQUENCY FUNCTIONS ========== */

__attribute__((noinline))
static void data_transform(int *arr, int n, int mode) {
    /* Medium frequency function with mode-dependent behavior */
    switch (mode) {
        case 1:
            for (int i = 0; i < n; i++) {
                arr[i] = arr[i] * 2 + 1;
            }
            break;
        case 2:
            for (int i = 0; i < n; i++) {
                arr[i] = (arr[i] << 1) | (arr[i] >> 31);
            }
            break;
        case 3:
            for (int i = 0; i < n; i++) {
                arr[i] = arr[i] ^ 0x55555555;
            }
            break;
        default:
            for (int i = 0; i < n; i++) {
                arr[i] = ~arr[i];
            }
    }
    
    /* Loop with varying iteration count */
    int limit = (mode == 1) ? n/2 : n;
    for (int i = 0; i < limit; i++) {
        if (arr[i] % 2 == 0) {
            arr[i] /= 2;
        }
    }
}

__attribute__((noinline))
static void threshold_filter(int *arr, int n, int threshold) {
    /* Function that behaves differently based on threshold */
    int count = 0;
    for (int i = 0; i < n; i++) {
        if (arr[i] > threshold) {
            arr[i] = threshold;
            count++;
        } else if (arr[i] < -threshold) {
            arr[i] = -threshold;
            count++;
        }
    }
    
    if (count > n/2) {
        /* This path varies across runs */
        for (int i = 0; i < n; i += 2) {
            arr[i] += threshold / 10;
        }
    }
}

/* ========== MAIN PROGRAM LOGIC ========== */

static void print_usage(const char *progname) {
    printf("Usage: %s [options]\n", progname);
    printf("Options:\n");
    printf("  --seed=N          Random seed (default: time-based)\n");
    printf("  --size=N          Data size (default: 1000)\n");
    printf("  --mode=N          Operation mode 1-4 (default: 1)\n");
    printf("  --iterations=N    Hot loop multiplier (default: 100)\n");
    printf("  --verbose         Enable verbose output\n");
    printf("  --algorithm=A|B|C Algorithm variant (default: A)\n");
}

int main(int argc, char *argv[]) {
    /* Parse command-line arguments */
    int seed = (int)time(NULL);
    int data_size = 1000;
    int mode = 1;
    int iterations = 100;
    char algorithm = 'A';
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--seed") == 0 && i+1 < argc) {
            seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--size") == 0 && i+1 < argc) {
            data_size = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--mode") == 0 && i+1 < argc) {
            mode = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--iterations") == 0 && i+1 < argc) {
            iterations = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--verbose") == 0) {
            verbose = 1;
        } else if (strcmp(argv[i], "--algorithm") == 0 && i+1 < argc) {
            algorithm = argv[++i][0];
        } else if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        }
    }
    
    if (data_size <= 0) data_size = 1000;
    if (iterations <= 0) iterations = 100;
    if (mode < 1 || mode > 4) mode = 1;
    
    /* Allocate and initialize data */
    int *data = malloc(data_size * sizeof(int));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Call functions in different orders based on mode/algorithm */
    initialization_routine(data, data_size, seed);
    
    /* Algorithm variants create different execution profiles */
    switch (algorithm) {
        case 'A':
            hot_loop_processor(data, data_size, iterations);
            data_transform(data, data_size, mode);
            process_data(data, data_size, mode);
            matrix_operations(50, seed);
            break;
            
        case 'B':
            data_transform(data, data_size, mode);
            hot_loop_processor(data, data_size, iterations/2);
            threshold_filter(data, data_size, 5000);
            analyze_results(data, data_size, 1000);
            matrix_operations(30, seed + 1);
            break;
            
        case 'C':
            matrix_operations(40, seed);
            hot_loop_processor(data, data_size, iterations * 2);
            cold_path_operation(data, data_size);
            validation_check(data, data_size);
            process_data(data, data_size, (mode % 2) + 1);
            break;
            
        default:
            hot_loop_processor(data, data_size, iterations);
            data_transform(data, data_size, mode);
    }
    
    /* Final processing */
    threshold_filter(data, data_size, hot_threshold);
    analyze_results(data, data_size, hot_threshold);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < data_size; i++) {
        checksum = (checksum * 31 + data[i]) & 0xFFFFFFFF;
    }
    
    printf("Result checksum: %lld (seed=%d, size=%d, mode=%d, algo=%c)\n",
           checksum, seed, data_size, mode, algorithm);
    
    free(data);
    return 0;
}
