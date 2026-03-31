/* gcov_tool_test.c - Program to generate varied GCOV profiles for overlap analysis */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Forward declarations for functions in different compilation units */
extern void process_data_hot(int *data, int size, int threshold);
extern void process_data_cold(int *data, int size);
extern int external_algorithm(int *data, int size, int mode);

/* Global configuration */
static int global_seed = 0;
static int global_iterations = 1000;
static int global_algorithm = 0;
static int global_threshold = 50;

/* Function prototypes */
__attribute__((noinline)) static void parse_arguments(int argc, char **argv);
__attribute__((noinline)) static void initialize_data(int *data, int size);
__attribute__((noinline)) static int bubble_sort(int *data, int size);
__attribute__((noinline)) static int quick_sort(int *data, int left, int right);
__attribute__((noinline)) static int partition(int *data, int left, int right);
__attribute__((noinline)) static void matrix_multiply(int size);
__attribute__((noinline)) static void fibonacci_sequence(int n);
__attribute__((hot)) static void hot_loop_function(int iterations);
__attribute__((cold)) static void cold_function_rarely_called(void);
__attribute__((noinline)) static int calculate_checksum(int *data, int size);
__attribute__((noinline)) static void process_mode_a(int *data, int size);
__attribute__((noinline)) static void process_mode_b(int *data, int size);
__attribute__((noinline)) static void process_mode_c(int *data, int size);

/* Different namespaces through static functions in same file */
static void helper_validate(int *data, int size) {
    for (int i = 0; i < size - 1; i++) {
        if (data[i] > data[i + 1]) {
            fprintf(stderr, "Validation failed at index %d\n", i);
        }
    }
}

/* Another helper with same name but different context */
static void helper_validate(double *data, int size) {
    /* Different parameter type creates different mangled name */
    for (int i = 0; i < size - 1; i++) {
        if (data[i] > data[i + 1]) {
            fprintf(stderr, "Double validation failed at %d\n", i);
        }
    }
}

int main(int argc, char **argv) {
    parse_arguments(argc, argv);
    
    const int data_size = 1000;
    int *data = malloc(data_size * sizeof(int));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with different patterns based on seed */
    initialize_data(data, data_size);
    
    /* Call different processing functions based on algorithm */
    int checksum = 0;
    switch (global_algorithm) {
        case 0:
            process_mode_a(data, data_size);
            checksum = bubble_sort(data, data_size);
            break;
        case 1:
            process_mode_b(data, data_size);
            checksum = quick_sort(data, 0, data_size - 1);
            break;
        case 2:
            process_mode_c(data, data_size);
            checksum = external_algorithm(data, data_size, global_algorithm);
            break;
        default:
            fprintf(stderr, "Unknown algorithm: %d\n", global_algorithm);
            free(data);
            return 1;
    }
    
    /* Hot functions called many times */
    hot_loop_function(global_iterations);
    
    /* Cold function rarely called */
    if (global_seed % 100 == 0) {
        cold_function_rarely_called();
    }
    
    /* Matrix operations for varied execution counts */
    matrix_multiply(50 + (global_seed % 20));
    
    /* Fibonacci for recursive profile */
    fibonacci_sequence(20 + (global_seed % 10));
    
    /* Process with hot/cold thresholds */
    if (global_iterations > global_threshold) {
        process_data_hot(data, data_size, global_threshold);
    } else {
        process_data_cold(data, data_size);
    }
    
    /* Calculate final checksum */
    checksum += calculate_checksum(data, data_size);
    
    /* Validate results */
    helper_validate(data, data_size);
    
    printf("Result checksum: %d\n", checksum);
    printf("Seed: %d, Iterations: %d, Algorithm: %d\n", 
           global_seed, global_iterations, global_algorithm);
    
    free(data);
    return 0;
}

/* Function implementations */
__attribute__((noinline)) 
static void parse_arguments(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            global_seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) {
            global_iterations = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--algorithm") == 0 && i + 1 < argc) {
            global_algorithm = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--threshold") == 0 && i + 1 < argc) {
            global_threshold = atoi(argv[++i]);
        }
    }
    
    if (global_seed == 0) {
        global_seed = time(NULL) % 10000;
    }
    
    srand(global_seed);
}

__attribute__((noinline))
static void initialize_data(int *data, int size) {
    /* Different initialization patterns based on seed */
    int pattern = global_seed % 4;
    
    for (int i = 0; i < size; i++) {
        switch (pattern) {
            case 0:
                data[i] = rand() % 1000;
                break;
            case 1:
                data[i] = (i * global_seed) % 1000;
                break;
            case 2:
                data[i] = size - i;
                break;
            case 3:
                data[i] = (i % 2 == 0) ? rand() % 500 : 1000 + rand() % 500;
                break;
        }
    }
}

__attribute__((noinline))
static int bubble_sort(int *data, int size) {
    int swaps = 0;
    #pragma GCC unroll 0
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            if (data[j] > data[j + 1]) {
                int temp = data[j];
                data[j] = data[j + 1];
                data[j + 1] = temp;
                swaps++;
            }
        }
    }
    return swaps;
}

__attribute__((noinline))
static int quick_sort(int *data, int left, int right) {
    if (left < right) {
        int pivot = partition(data, left, right);
        int comps = (right - left);
        comps += quick_sort(data, left, pivot - 1);
        comps += quick_sort(data, pivot + 1, right);
        return comps;
    }
    return 0;
}

__attribute__((noinline))
static int partition(int *data, int left, int right) {
    int pivot = data[right];
    int i = left - 1;
    
    for (int j = left; j < right; j++) {
        if (data[j] < pivot) {
            i++;
            int temp = data[i];
            data[i] = data[j];
            data[j] = temp;
        }
    }
    
    int temp = data[i + 1];
    data[i + 1] = data[right];
    data[right] = temp;
    
    return i + 1;
}

__attribute__((noinline))
static void matrix_multiply(int size) {
    double *A = malloc(size * size * sizeof(double));
    double *B = malloc(size * size * sizeof(double));
    double *C = malloc(size * size * sizeof(double));
    
    if (!A || !B || !C) {
        free(A); free(B); free(C);
        return;
    }
    
    /* Initialize matrices */
    for (int i = 0; i < size * size; i++) {
        A[i] = (double)rand() / RAND_MAX;
        B[i] = (double)rand() / RAND_MAX;
    }
    
    /* Multiply with different loop orders based on seed */
    int loop_order = global_seed % 3;
    
    switch (loop_order) {
        case 0:  /* ijk */
            for (int i = 0; i < size; i++) {
                for (int j = 0; j < size; j++) {
                    C[i*size + j] = 0;
                    for (int k = 0; k < size; k++) {
                        C[i*size + j] += A[i*size + k] * B[k*size + j];
                    }
                }
            }
            break;
            
        case 1:  /* ikj */
            for (int i = 0; i < size; i++) {
                for (int k = 0; k < size; k++) {
                    double a = A[i*size + k];
                    for (int j = 0; j < size; j++) {
                        C[i*size + j] += a * B[k*size + j];
                    }
                }
            }
            break;
            
        case 2:  /* jik */
            for (int j = 0; j < size; j++) {
                for (int i = 0; i < size; i++) {
                    C[i*size + j] = 0;
                    for (int k = 0; k < size; k++) {
                        C[i*size + j] += A[i*size + k] * B[k*size + j];
                    }
                }
            }
            break;
    }
    
    /* Validate with double version helper */
    helper_validate(C, size * size);
    
    free(A); free(B); free(C);
}

__attribute__((noinline))
static void fibonacci_sequence(int n) {
    if (n <= 1) return;
    
    int *fib = malloc(n * sizeof(int));
    if (!fib) return;
    
    fib[0] = 0;
    fib[1] = 1;
    
    for (int i = 2; i < n; i++) {
        fib[i] = fib[i-1] + fib[i-2];
    }
    
    free(fib);
}

__attribute__((hot))
static void hot_loop_function(int iterations) {
    volatile int counter = 0;
    #pragma GCC unroll 0
    for (int i = 0; i < iterations; i++) {
        /* Hot loop with many iterations */
        counter += (i % 2 == 0) ? i : -i;
        
        /* Nested conditionals for branch coverage */
        if (i % 3 == 0) {
            counter *= 2;
        } else if (i % 7 == 0) {
            counter /= 2;
        } else {
            counter += 1;
        }
        
        /* Switch statement for varied paths */
        switch (i % 5) {
            case 0: counter += 10; break;
            case 1: counter -= 5; break;
            case 2: counter *= 3; break;
            case 3: counter /= 2; break;
            case 4: counter = counter ^ 0xFF; break;
        }
    }
    
    /* Prevent dead code elimination */
    if (counter == 0) {
        printf("Hot loop completed\n");
    }
}

__attribute__((cold))
static void cold_function_rarely_called(void) {
    /* This function should have very low execution count */
    printf("Cold function called with seed: %d\n", global_seed);
    
    /* Complex but rarely executed path */
    int x = 0;
    for (int i = 0; i < 10; i++) {
        if (global_seed % (i + 2) == 0) {
            x += i;
        }
    }
}

__attribute__((noinline))
static int calculate_checksum(int *data, int size) {
    int checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum ^= data[i];
        checksum = (checksum << 1) | (checksum >> 31);
    }
    return checksum;
}

__attribute__((noinline))
static void process_mode_a(int *data, int size) {
    /* Mode A processing */
    for (int i = 0; i < size; i++) {
        if (data[i] % 2 == 0) {
            data[i] *= 2;
        } else {
            data[i] /= 2;
        }
    }
}

__attribute__((noinline))
static void process_mode_b(int *data, int size) {
    /* Mode B processing */
    for (int i = 0; i < size; i++) {
        data[i] = (data[i] * 3 + 7) % 1000;
    }
}

__attribute__((noinline))
static void process_mode_c(int *data, int size) {
    /* Mode C processing */
    for (int i = 0; i < size; i += 2) {
        int temp = data[i];
        data[i] = data[i + 1];
        data[i + 1] = temp;
    }
}
