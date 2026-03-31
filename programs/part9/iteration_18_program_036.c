/* gcov_tool_test.c - Multi-run profile generator for gcov-tool testing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Forward declarations for functions in different compilation units */
extern void process_data_hot(int *data, int size, int threshold);
extern void process_data_cold(int *data, int size);
extern int search_algorithm(int *data, int size, int target, int mode);

/* Global configuration */
static int g_verbose = 0;
static int g_iterations = 1000;
static int g_seed = 42;
static int g_algorithm = 1;
static int g_threshold = 500;

/* Function 1: Hot function with many iterations */
__attribute__((hot)) 
void hot_loop_function(int *data, int size) {
    long long sum = 0;
    for (int i = 0; i < g_iterations; i++) {
        for (int j = 0; j < size; j++) {
            if (data[j] > g_threshold) {
                sum += data[j] * 2;
            } else if (data[j] > g_threshold / 2) {
                sum += data[j];
            } else {
                sum += 1;
            }
        }
    }
    if (g_verbose) printf("Hot function sum: %lld\n", sum);
}

/* Function 2: Cold function with few iterations */
__attribute__((cold))
void cold_function(int *data, int size) {
    int count = 0;
    for (int i = 0; i < size; i++) {
        if (data[i] < 0) count++;
    }
    if (count > size / 2 && g_verbose) {
        printf("Many negative values: %d\n", count);
    }
}

/* Function 3: Complex branching function */
__attribute__((noinline))
int complex_branching(int a, int b, int c) {
    int result = 0;
    
    if (a > b) {
        if (b > c) {
            result = a * b;
        } else if (a > c) {
            result = a * c;
        } else {
            result = b * c;
        }
    } else {
        switch (c % 4) {
            case 0:
                result = a + b;
                break;
            case 1:
                result = a - b;
                break;
            case 2:
                result = b - a;
                break;
            case 3:
                result = a * b + c;
                break;
            default:
                result = -1;
        }
    }
    
    return result;
}

/* Function 4: Recursive function with varying depth */
__attribute__((noinline))
int recursive_function(int n, int depth) {
    if (depth <= 0 || n <= 1) {
        return n;
    }
    
    int result = 0;
    if (n % 2 == 0) {
        result = recursive_function(n / 2, depth - 1) + 1;
    } else {
        result = recursive_function(n * 3 + 1, depth - 1) - 1;
    }
    
    return result + complex_branching(n, depth, result);
}

/* Function 5: Matrix operation with nested loops */
void matrix_operation(int size) {
    int matrix[10][10];
    int result = 0;
    
    for (int i = 0; i < size && i < 10; i++) {
        for (int j = 0; j < size && j < 10; j++) {
            matrix[i][j] = i * j + g_seed;
            
            if (g_algorithm == 1) {
                result += matrix[i][j];
            } else if (g_algorithm == 2) {
                result -= matrix[i][j];
            } else {
                result ^= matrix[i][j];
            }
        }
    }
    
    if (g_verbose && result != 0) {
        printf("Matrix result: %d\n", result);
    }
}

/* Function 6: String processing with different paths */
void string_processor(const char *input) {
    int length = strlen(input);
    char buffer[256];
    int j = 0;
    
    for (int i = 0; i < length && j < 255; i++) {
        if (input[i] >= 'A' && input[i] <= 'Z') {
            buffer[j++] = input[i] + 32;  // to lowercase
        } else if (input[i] >= 'a' && input[i] <= 'z') {
            buffer[j++] = input[i] - 32;  // to uppercase
        } else if (input[i] >= '0' && input[i] <= '9') {
            buffer[j++] = input[i];
            if (j < 255) buffer[j++] = '_';
        } else {
            // Skip others
        }
    }
    buffer[j] = '\0';
    
    if (g_verbose && j > 0) {
        printf("Processed string: %s\n", buffer);
    }
}

/* Function 7: Data validation with early returns */
int validate_data(int *data, int size) {
    if (data == NULL || size <= 0) {
        return 0;
    }
    
    int valid_count = 0;
    for (int i = 0; i < size; i++) {
        if (data[i] >= 0 && data[i] <= 1000) {
            valid_count++;
        } else if (data[i] < 0) {
            return -1;  // Early return for negative
        } else {
            // Too large, skip
            continue;
        }
        
        if (valid_count > size / 2) {
            break;  // Early break
        }
    }
    
    return valid_count;
}

/* Main execution with configurable paths */
int main(int argc, char *argv[]) {
    /* Parse command line arguments */
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
        } else if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc) {
            /* Different modes for different execution paths */
            int mode = atoi(argv[++i]);
            g_algorithm = (mode % 3) + 1;
            g_iterations = 500 + (mode * 100);
        }
    }
    
    /* Initialize with seed for reproducibility */
    srand(g_seed);
    
    /* Create test data */
    int data_size = 100;
    int *data = malloc(data_size * sizeof(int));
    if (!data) return 1;
    
    for (int i = 0; i < data_size; i++) {
        data[i] = rand() % 2000 - 500;  /* Range: -500 to 1499 */
    }
    
    /* Execute functions in different orders based on algorithm */
    long long total = 0;
    
    /* Always run hot function (many iterations) */
    hot_loop_function(data, data_size);
    
    /* Run cold function occasionally */
    if (g_algorithm != 2) {
        cold_function(data, data_size);
    }
    
    /* Complex branching based on seed */
    for (int i = 0; i < 10; i++) {
        total += complex_branching(data[i], data[i+1], data[i+2]);
    }
    
    /* Recursive function with varying depth */
    int rec_depth = (g_algorithm == 1) ? 5 : 3;
    total += recursive_function(g_seed % 100, rec_depth);
    
    /* Matrix operation */
    matrix_operation(g_algorithm + 5);
    
    /* String processing */
    char test_string[] = "Test123StringABCxyz789";
    string_processor(test_string);
    
    /* Data validation */
    int valid = validate_data(data, data_size);
    total += valid;
    
    /* External functions (from other compilation units) */
    process_data_hot(data, data_size, g_threshold);
    process_data_cold(data, data_size);
    
    /* Search algorithm with different modes */
    int target = g_seed % 1000;
    int found = search_algorithm(data, data_size, target, g_algorithm);
    total += found;
    
    /* Dead code section - only compiled in certain variants */
#ifdef VARIANT_A
    /* Extra computation for variant A */
    for (int i = 0; i < data_size; i++) {
        if (data[i] % 3 == 0) total += 1;
    }
#elif defined(VARIANT_B)
    /* Different computation for variant B */
    for (int i = 0; i < data_size; i++) {
        if (data[i] % 5 == 0) total -= 1;
    }
#endif
    
    /* Final checksum output */
    printf("RESULT: %lld\n", total);
    
    free(data);
    return 0;
}
