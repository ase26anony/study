/* gcov_tool_test.c - Multi-run profile generator for gcov-tool overlap analysis */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Function attributes to control instrumentation */
#define HOT __attribute__((hot))
#define COLD __attribute__((cold))
#define NOINLINE __attribute__((noinline))

/* Global configuration */
static int g_iterations = 1000;
static int g_seed = 42;
static int g_algorithm = 0;
static int g_verbose = 0;

/* ========== OBJECT 1 FUNCTIONS (file1.c equivalent) ========== */

/* Hot function - runs many times */
HOT NOINLINE
void process_array_hot(int *arr, int size, int threshold) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        if (arr[i] > threshold) {
            sum += arr[i] * 2;  /* Hot path */
        } else {
            sum += arr[i];      /* Cold path */
        }
        
        /* Nested condition for branch coverage */
        if (i % 3 == 0) {
            arr[i] = (arr[i] * 3) % 100;
        } else if (i % 3 == 1) {
            arr[i] = (arr[i] * 7) % 100;
        } else {
            arr[i] = (arr[i] * 11) % 100;
        }
    }
    
    if (g_verbose) printf("Hot process sum: %d\n", sum);
}

/* Cold function - runs rarely */
COLD NOINLINE
void validate_data(int *arr, int size) {
    int errors = 0;
    for (int i = 0; i < size; i++) {
        if (arr[i] < 0 || arr[i] > 1000) {
            errors++;
            if (g_algorithm == 2) {
                arr[i] = arr[i] % 100;  /* Only in algorithm 2 */
            }
        }
    }
    if (errors > 0 && g_verbose) {
        printf("Found %d data errors\n", errors);
    }
}

/* Function with complex branching */
NOINLINE
int analyze_pattern(int *arr, int size, int mode) {
    int patterns[5] = {0};
    int total = 0;
    
    for (int i = 0; i < size - 1; i++) {
        int diff = arr[i + 1] - arr[i];
        
        switch (mode) {
            case 0:
                if (diff > 10) patterns[0]++;
                else if (diff < -10) patterns[1]++;
                else patterns[2]++;
                break;
            case 1:
                if (diff > 20) patterns[0]++;
                else if (diff > 0) patterns[1]++;
                else if (diff == 0) patterns[2]++;
                else patterns[3]++;
                break;
            case 2:
                /* Different path for algorithm 2 */
                if (diff % 2 == 0) patterns[0]++;
                if (diff % 3 == 0) patterns[1]++;
                if (diff % 5 == 0) patterns[2]++;
                patterns[3] += (diff > 0) ? 1 : 0;
                patterns[4] += (diff < 0) ? 1 : 0;
                break;
            default:
                patterns[0] += abs(diff);
        }
        
        total += diff;
    }
    
    /* Return pattern based on algorithm */
    if (g_algorithm == 0) return patterns[0];
    if (g_algorithm == 1) return patterns[1] + patterns[2];
    return patterns[0] + patterns[3];
}

/* ========== OBJECT 2 FUNCTIONS (file2.c equivalent) ========== */

/* Another hot function with different characteristics */
HOT NOINLINE
void transform_matrix(int **matrix, int rows, int cols) {
    int transformations = 0;
    
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int val = matrix[i][j];
            
            /* Complex conditional chain */
            if (val < 30) {
                matrix[i][j] = val * val;
                transformations++;
            } else if (val < 60) {
                matrix[i][j] = val + 100;
                if (g_algorithm == 1) {
                    matrix[i][j] -= 50;  /* Algorithm-specific */
                }
            } else if (val < 90) {
                matrix[i][j] = val / 2;
                transformations += 2;
            } else {
                matrix[i][j] = 255 - val;
                if (g_algorithm == 2 && val > 120) {
                    matrix[i][j] = 0;  /* Only in algorithm 2 */
                }
            }
            
            /* Inner loop with varying iterations */
            for (int k = 0; k < (val % 5); k++) {
                matrix[i][j] += k;
            }
        }
    }
    
    if (g_verbose && transformations > 100) {
        printf("Matrix transformations: %d\n", transformations);
    }
}

/* Cold utility function */
COLD NOINLINE
void initialize_matrix(int **matrix, int rows, int cols, int seed) {
    srand(seed);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            matrix[i][j] = rand() % 256;
        }
    }
}

/* Function with same name as in file1 but different signature */
NOINLINE
void process_array_hot(double *arr, int size) {
    double sum = 0.0;
    for (int i = 0; i < size; i++) {
        arr[i] = arr[i] * 1.5;
        sum += arr[i];
        
        if (i % 4 == 0) {
            arr[i] += 10.0;
        } else if (i % 4 == 1) {
            arr[i] -= 5.0;
        }
    }
    
    if (sum > 1000.0 && g_verbose) {
        printf("Double array sum: %.2f\n", sum);
    }
}

/* ========== MAIN PROGRAM ========== */

/* Dead code variants */
#ifdef VARIANT_A
NOINLINE
void variant_a_specific(int *arr, int size) {
    for (int i = 0; i < size; i += 2) {
        arr[i] = arr[i] * 3;
    }
}
#else
NOINLINE
void variant_b_specific(int *arr, int size) {
    for (int i = 1; i < size; i += 2) {
        arr[i] = arr[i] * 7;
    }
}
#endif

/* Main computation driver */
NOINLINE
unsigned long long run_algorithm(int algorithm, int seed, int iterations) {
    unsigned long long checksum = 0;
    
    /* Initialize data with seed */
    srand(seed);
    int data_size = 500 + (seed % 500);
    int *data = (int*)malloc(data_size * sizeof(int));
    double *ddata = (double*)malloc(data_size * sizeof(double));
    
    for (int i = 0; i < data_size; i++) {
        data[i] = rand() % 1000;
        ddata[i] = (double)(rand() % 1000) / 10.0;
        checksum += data[i];
    }
    
    /* Create matrix */
    int rows = 10 + (seed % 10);
    int cols = 10 + (seed % 10);
    int **matrix = (int**)malloc(rows * sizeof(int*));
    for (int i = 0; i < rows; i++) {
        matrix[i] = (int*)malloc(cols * sizeof(int));
    }
    initialize_matrix(matrix, rows, cols, seed + 1);
    
    /* Run selected algorithm */
    for (int iter = 0; iter < iterations; iter++) {
        switch (algorithm) {
            case 0:  /* Algorithm 0 - Balanced */
                process_array_hot(data, data_size, 300);
                validate_data(data, data_size);
                checksum += analyze_pattern(data, data_size, 0);
                transform_matrix(matrix, rows, cols);
                break;
                
            case 1:  /* Algorithm 1 - Matrix heavy */
                process_array_hot(data, data_size, 500);
                checksum += analyze_pattern(data, data_size, 1);
                for (int i = 0; i < 3; i++) {
                    transform_matrix(matrix, rows, cols);
                }
                process_array_hot(ddata, data_size);
                break;
                
            case 2:  /* Algorithm 2 - Data heavy */
                for (int i = 0; i < 5; i++) {
                    process_array_hot(data, data_size, 200);
                }
                validate_data(data, data_size);
                checksum += analyze_pattern(data, data_size, 2);
                transform_matrix(matrix, rows, cols);
                
                #ifdef VARIANT_A
                variant_a_specific(data, data_size);
                #else
                variant_b_specific(data, data_size);
                #endif
                break;
                
            default:  /* Random algorithm */
                process_array_hot(data, data_size, rand() % 800);
                if (rand() % 10 == 0) {
                    validate_data(data, data_size);
                }
                checksum += analyze_pattern(data, data_size, rand() % 3);
                transform_matrix(matrix, rows, cols);
        }
        
        /* Update checksum with matrix */
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                checksum += matrix[i][j];
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < rows; i++) free(matrix[i]);
    free(matrix);
    free(data);
    free(ddata);
    
    return checksum;
}

/* Command line parsing */
void parse_args(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            g_seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) {
            g_iterations = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--algorithm") == 0 && i + 1 < argc) {
            g_algorithm = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--verbose") == 0) {
            g_verbose = 1;
        } else if (strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [--seed N] [--iterations M] [--algorithm A] [--verbose]\n", argv[0]);
            printf("  --seed N        : Random seed (default: 42)\n");
            printf("  --iterations M  : Main loop iterations (default: 1000)\n");
            printf("  --algorithm A   : 0=Balanced, 1=Matrix heavy, 2=Data heavy (default: 0)\n");
            printf("  --verbose       : Enable verbose output\n");
            exit(0);
        }
    }
}

int main(int argc, char *argv[]) {
    parse_args(argc, argv);
    
    if (g_verbose) {
        printf("Starting algorithm %d with seed %d, iterations %d\n",
               g_algorithm, g_seed, g_iterations);
    }
    
    unsigned long long result = run_algorithm(g_algorithm, g_seed, g_iterations);
    
    /* Output deterministic result to prevent dead code elimination */
    printf("Result: %llu\n", result);
    
    return 0;
}
