/* gcov_tool_test.c - Program to generate varied GCOV profiles for testing gcov-tool overlap analysis */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

/* Function attributes to control instrumentation */
#define NOINLINE __attribute__((noinline))
#define HOT __attribute__((hot))
#define COLD __attribute__((cold))

/* Global configuration */
static int g_mode = 0;
static int g_iterations = 1000;
static int g_seed = 42;
static int g_algorithm = 0;

/* ========== OBJECT 1 FUNCTIONS (file1.c) ========== */

/* Hot function - runs many times */
HOT NOINLINE
void process_array_hot(int *arr, int size, int threshold) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        if (arr[i] > threshold) {
            sum += arr[i] * 2;
        } else if (arr[i] < -threshold) {
            sum += arr[i] / 2;
        } else {
            sum += arr[i];
        }
    }
    
    /* Nested conditionals for branch coverage */
    if (sum > 1000000) {
        printf("Very large sum: %d\n", sum);
    } else if (sum > 1000) {
        if (size > 50) {
            /* Do nothing */
        }
    }
}

/* Cold function - runs rarely */
COLD NOINLINE
void validate_data(int *arr, int size) {
    int errors = 0;
    for (int i = 0; i < size; i++) {
        if (arr[i] == 0x7FFFFFFF || arr[i] == 0x80000000) {
            errors++;
        }
    }
    if (errors > 0) {
        printf("Validation found %d potential overflows\n", errors);
    }
}

NOINLINE
int search_algorithm_a(int *arr, int size, int target) {
    int count = 0;
    for (int i = 0; i < size; i++) {
        if (arr[i] == target) {
            count++;
            if (g_mode == 1) break;  /* Different behavior per mode */
        }
    }
    return count;
}

/* ========== OBJECT 2 FUNCTIONS (file2.c) ========== */

/* Another function with same name but in different compilation unit */
NOINLINE
static void helper_function(int x) {
    switch (x % 4) {
        case 0:
            x *= 2;
            break;
        case 1:
            x += 100;
            break;
        case 2:
            x -= 50;
            if (g_algorithm == 2) x *= 3;
            break;
        case 3:
            x /= 2;
            break;
        default:
            x = 0;
    }
}

HOT NOINLINE
void process_array_cold(int *arr, int size) {
    int limit = (g_mode == 2) ? size / 2 : size;
    
    #pragma GCC unroll 0  /* Prevent aggressive unrolling */
    for (int i = 0; i < limit; i++) {
        if (i % 3 == 0) {
            arr[i] = arr[i] * arr[i];
        } else if (i % 3 == 1) {
            arr[i] = arr[i] + arr[size - i - 1];
        } else {
            arr[i] = arr[i] - (i * 2);
        }
        
        /* Call same-named function */
        helper_function(arr[i]);
    }
}

NOINLINE
int complex_calculation(int a, int b, int c) {
    int result = 0;
    
    if (a > b && a > c) {
        result = a * b - c;
    } else if (b > a && b > c) {
        result = b * c - a;
        if (result < 0) result = -result;
    } else {
        result = c * a - b;
        for (int i = 0; i < 10; i++) {
            result += i * (g_mode + 1);
        }
    }
    
    /* Dead code that might be eliminated */
    #ifdef VARIANT_A
    if (result > 1000) {
        result = 1000;
    }
    #endif
    
    #ifdef VARIANT_B
    if (result < -1000) {
        result = -1000;
    }
    #endif
    
    return result;
}

/* ========== OBJECT 3 FUNCTIONS (file3.c) ========== */

/* Another helper_function in different file */
NOINLINE
static void helper_function(double x) {
    if (x > 0.0) {
        x = sqrt(x);
    } else {
        x = -sqrt(-x);
    }
}

COLD NOINLINE
void statistical_analysis(int *arr, int size) {
    if (size < 2) return;
    
    double mean = 0.0;
    double variance = 0.0;
    
    /* Calculate mean */
    for (int i = 0; i < size; i++) {
        mean += arr[i];
    }
    mean /= size;
    
    /* Calculate variance */
    int sample_size = (g_mode == 3) ? size / 2 : size;
    for (int i = 0; i < sample_size; i++) {
        double diff = arr[i] - mean;
        variance += diff * diff;
        
        /* Call overloaded helper */
        helper_function(diff);
    }
    variance /= sample_size;
    
    if (variance > 1000000.0) {
        printf("High variance: %f\n", variance);
    }
}

HOT NOINLINE
void sorting_algorithm(int *arr, int size) {
    /* Simple bubble sort with mode-dependent optimization */
    int limit = (g_algorithm == 1) ? size - 1 : size;
    
    for (int i = 0; i < limit; i++) {
        int swapped = 0;
        for (int j = 0; j < size - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
                swapped = 1;
            }
            
            /* Different behavior per algorithm */
            if (g_algorithm == 2 && j % 100 == 0) {
                arr[j] = arr[j] % 1000;
            }
        }
        if (!swapped) break;
    }
}

/* ========== MAIN PROGRAM ========== */

NOINLINE
int process_with_mode(int *data, int size) {
    int result = 0;
    
    /* Vary function call order based on mode */
    switch (g_mode) {
        case 0:
            process_array_hot(data, size, 100);
            result += search_algorithm_a(data, size, 42);
            sorting_algorithm(data, size);
            break;
        case 1:
            sorting_algorithm(data, size);
            process_array_cold(data, size);
            result += complex_calculation(data[0], data[1], data[2]);
            break;
        case 2:
            process_array_hot(data, size, 500);
            statistical_analysis(data, size);
            validate_data(data, size);
            result += data[size / 2];
            break;
        case 3:
            for (int i = 0; i < 10; i++) {
                process_array_hot(data, size, i * 50);
            }
            result += search_algorithm_a(data, size, -1);
            break;
        default:
            process_array_hot(data, size, 100);
            process_array_cold(data, size);
            sorting_algorithm(data, size);
            result = 1;
    }
    
    return result;
}

int main(int argc, char *argv[]) {
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
        }
    }
    
    /* Initialize random generator with seed for reproducibility */
    srand(g_seed);
    
    /* Create data array */
    int data_size = 1000 + (g_mode * 100);
    int *data = malloc(data_size * sizeof(int));
    
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Fill with random data */
    for (int i = 0; i < data_size; i++) {
        data[i] = rand() % 2000 - 1000;  /* Range: -1000 to 999 */
    }
    
    /* Main processing loop - creates hot/cold regions */
    int total_result = 0;
    
    /* HOT LOOP: Runs many times */
    for (int iter = 0; iter < g_iterations; iter++) {
        /* Modify data slightly each iteration */
        if (iter % 100 == 0) {
            data[rand() % data_size] = rand() % 2000 - 1000;
        }
        
        /* Process data */
        total_result += process_with_mode(data, data_size);
        
        /* COLD PATH: Only execute occasionally */
        if (iter % 500 == 0) {
            validate_data(data, data_size);
            statistical_analysis(data, data_size);
        }
    }
    
    /* Final processing */
    sorting_algorithm(data, data_size);
    process_array_hot(data, data_size, 750);
    
    /* Calculate checksum for verification */
    int checksum = 0;
    for (int i = 0; i < data_size; i++) {
        checksum = (checksum * 31 + data[i]) % 1000000007;
    }
    checksum = (checksum + total_result) % 1000000007;
    
    printf("Result: %d (Checksum: %d)\n", total_result, checksum);
    
    free(data);
    return 0;
}
