/* gcov_tool_test.c - Generates varied GCOV profiles for overlap analysis */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Function attributes to control instrumentation */
#define NOINLINE __attribute__((noinline))
#define HOT __attribute__((hot))
#define COLD __attribute__((cold))

/* Global configuration */
static int g_mode = 0;
static int g_iterations = 1000;
static int g_seed = 42;
static int g_algorithm = 0;
static int g_use_full_paths = 0;

/* ========== OBJECT 1 FUNCTIONS (file1.c equivalent) ========== */

/* Hot function - runs many times */
HOT NOINLINE
void process_data_hot(int* data, int size, int threshold) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        if (data[i] > threshold) {
            sum += data[i] * 2;  /* Hot path */
        } else {
            sum += data[i];      /* Cold path */
        }
        
        /* Nested condition for branch coverage */
        if (g_mode == 1) {
            data[i] = (data[i] % 7) + 1;
        } else if (g_mode == 2) {
            data[i] = (data[i] % 13) + 1;
        } else {
            data[i] = (data[i] % 5) + 1;
        }
    }
    
    /* Prevent dead code elimination */
    if (sum < 0) printf("Impossible\n");
}

/* Cold function - runs rarely */
COLD NOINLINE
void validate_data(int* data, int size) {
    int errors = 0;
    for (int i = 0; i < size; i++) {
        if (data[i] < 0 || data[i] > 1000) {
            errors++;
            if (g_use_full_paths) {
                data[i] = data[i] % 1000;
            }
        }
    }
    
    if (errors > size / 2) {
        printf("Warning: %d invalid values\n", errors);
    }
}

/* Function with switch for varied coverage */
NOINLINE
int select_operation(int value, int algo) {
    int result = 0;
    
    switch (algo) {
        case 0:
            result = value * 2;
            break;
        case 1:
            result = value + value / 2;
            break;
        case 2:
            result = value * 3 - value / 2;
            break;
        case 3:
            result = (value << 1) | 0x1;
            break;
        default:
            result = value;
            break;
    }
    
    return result;
}

/* ========== OBJECT 2 FUNCTIONS (file2.c equivalent) ========== */

/* Another hot function with different characteristics */
HOT NOINLINE
void transform_matrix(int** matrix, int rows, int cols) {
    int total = 0;
    
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            /* Complex branching based on mode */
            if (matrix[i][j] % 2 == 0) {
                if (g_mode == 1) {
                    matrix[i][j] = matrix[i][j] / 2;
                } else if (g_mode == 2 && j > 0) {
                    matrix[i][j] = matrix[i][j] + matrix[i][j-1];
                } else {
                    matrix[i][j] = matrix[i][j] * 3;
                }
                total += matrix[i][j];
            } else {
                matrix[i][j] = matrix[i][j] - 1;
                total -= matrix[i][j];
            }
            
            /* Threshold-based hot/cold differentiation */
            if (total > 1000000) {
                matrix[i][j] = matrix[i][j] % 1000;
            }
        }
    }
}

/* Cold function with recursion */
COLD NOINLINE
int recursive_check(int depth, int value) {
    if (depth <= 0) return value;
    
    if (value % 3 == 0) {
        return recursive_check(depth - 1, value / 3);
    } else if (value % 2 == 0) {
        return recursive_check(depth - 2, value / 2);
    } else {
        return recursive_check(depth - 1, value - 1);
    }
}

/* Function with loop that varies by iteration count */
NOINLINE
void variable_loop_operation(int* array, int size) {
    int actual_iterations = g_iterations;
    if (g_mode == 2) {
        actual_iterations = g_iterations / 2;
    } else if (g_mode == 3) {
        actual_iterations = g_iterations * 2;
    }
    
    /* Loop with potentially many iterations */
    for (int iter = 0; iter < actual_iterations; iter++) {
        int idx = iter % size;
        if (iter < actual_iterations / 10) {
            /* Hot section - first 10% of iterations */
            array[idx] = select_operation(array[idx], g_algorithm);
        } else {
            /* Cold section - remaining iterations */
            array[idx] = array[idx] + (iter % 17);
        }
    }
}

/* ========== OBJECT 3 FUNCTIONS (file3.c equivalent) ========== */

/* Static function with same name as in other files */
static NOINLINE
int helper_function(int x) {
    return x * x + x;
}

/* Another static helper with same name */
static NOINLINE
int helper_function(int x, int y) {
    return x * y - (x + y);
}

/* Namespace-like structure for -F testing */
struct math_ops {
    NOINLINE
    static int add(int a, int b) {
        int result = a + b;
        for (int i = 0; i < (g_mode == 1 ? 10 : 5); i++) {
            result += (i % 2);
        }
        return result;
    }
    
    NOINLINE
    static int multiply(int a, int b) {
        int result = 0;
        int limit = (g_algorithm == 0) ? b : a;
        for (int i = 0; i < limit; i++) {
            result += a;
            if (result > 1000000) break;
        }
        return result;
    }
};

/* ========== MAIN PROGRAM ========== */

NOINLINE
void run_algorithm_a(int* data, int size) {
    printf("Running Algorithm A\n");
    process_data_hot(data, size, 500);
    variable_loop_operation(data, size);
    
    int** matrix = (int**)malloc(10 * sizeof(int*));
    for (int i = 0; i < 10; i++) {
        matrix[i] = (int*)malloc(10 * sizeof(int));
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = data[(i * 10 + j) % size];
        }
    }
    
    transform_matrix(matrix, 10, 10);
    
    for (int i = 0; i < 10; i++) free(matrix[i]);
    free(matrix);
}

NOINLINE
void run_algorithm_b(int* data, int size) {
    printf("Running Algorithm B\n");
    validate_data(data, size);
    
    for (int i = 0; i < size; i++) {
        data[i] = recursive_check(3, data[i]);
        data[i] = math_ops::add(data[i], i);
        data[i] = math_ops::multiply(data[i], 2);
    }
    
    process_data_hot(data, size, 200);
}

NOINLINE
void run_algorithm_c(int* data, int size) {
    printf("Running Algorithm C\n");
    
    /* Mixed execution pattern */
    for (int i = 0; i < 5; i++) {
        if (i % 2 == 0) {
            process_data_hot(data + (i * size/5), size/5, 300);
        } else {
            validate_data(data + (i * size/5), size/5);
        }
    }
    
    variable_loop_operation(data, size);
    
    /* Use both helper functions */
    for (int i = 0; i < size; i++) {
        if (i % 3 == 0) {
            data[i] = helper_function(data[i]);
        } else {
            data[i] = helper_function(data[i], i);
        }
    }
}

int main(int argc, char** argv) {
    /* Parse command-line arguments for variability */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc) {
            g_mode = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) {
            g_iterations = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            g_seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--algorithm") == 0 && i + 1 < argc) {
            g_algorithm = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--fullpaths") == 0) {
            g_use_full_paths = 1;
        }
    }
    
    /* Seed RNG for reproducible but varied profiles */
    srand(g_seed);
    
    /* Create test data */
    const int data_size = 1000;
    int* data = (int*)malloc(data_size * sizeof(int));
    
    for (int i = 0; i < data_size; i++) {
        data[i] = rand() % 10000;
    }
    
    /* Select execution path based on mode */
    switch (g_mode) {
        case 0:
            run_algorithm_a(data, data_size);
            break;
        case 1:
            run_algorithm_b(data, data_size);
            break;
        case 2:
            run_algorithm_c(data, data_size);
            break;
        case 3:
            /* Mixed execution */
            run_algorithm_a(data, data_size / 2);
            run_algorithm_b(data + data_size / 2, data_size / 2);
            break;
        default:
            /* All algorithms */
            run_algorithm_a(data, data_size);
            run_algorithm_b(data, data_size);
            run_algorithm_c(data, data_size);
            break;
    }
    
    /* Calculate checksum to prevent optimization */
    unsigned long long checksum = 0;
    for (int i = 0; i < data_size; i++) {
        checksum = (checksum * 31 + data[i]) % 1000000007;
    }
    
    printf("Checksum: %llu\n", checksum);
    printf("Mode: %d, Seed: %d, Iterations: %d, Algorithm: %d\n", 
           g_mode, g_seed, g_iterations, g_algorithm);
    
    free(data);
    return 0;
}
