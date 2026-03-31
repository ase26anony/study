/* gcov_tool_test.c - Generates varied GCOV profiles for testing gcov-tool overlap analysis */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Forward declarations for functions in different compilation units */
extern void process_data(int* data, int size, int threshold);
extern void analyze_results(const int* data, int size, int mode);

/* Global configuration */
static int global_seed = 0;
static int global_iterations = 1000;
static int global_algorithm = 0;
static int global_threshold = 50;

/* Function prototypes with attributes */
__attribute__((noinline)) static void hot_function_a(int iterations);
__attribute__((noinline)) static void hot_function_b(int iterations);
__attribute__((noinline)) static void cold_function_a(void);
__attribute__((noinline)) static void cold_function_b(void);
__attribute__((noinline, cold)) static void rarely_called(void);
__attribute__((noinline, hot)) static void frequently_called(int count);
__attribute__((noinline)) static int complex_conditional(int x, int y, int mode);
__attribute__((noinline)) static void nested_loops(int depth, int width);
__attribute__((noinline)) static int data_checksum(const int* data, int size);

/* Different namespaces via static functions in same file */
static void helper_process(void) { /* Local helper */ }
static void util_process(void) { /* Another helper with same base name */ }

/* Main processing function with multiple execution paths */
__attribute__((noinline))
static void execute_algorithm(int algorithm, int* data, int size, int seed) {
    int i, j, temp;
    
    switch (algorithm) {
        case 0: /* Bubble sort variant */
            for (i = 0; i < size - 1; i++) {
                for (j = 0; j < size - i - 1; j++) {
                    if (data[j] > data[j + 1]) {
                        temp = data[j];
                        data[j] = data[j + 1];
                        data[j + 1] = temp;
                    }
                }
                /* Conditional break based on seed */
                if (i > size / 2 && (seed % 3 == 0)) {
                    break;
                }
            }
            break;
            
        case 1: /* Selection sort with early exit */
            for (i = 0; i < size - 1; i++) {
                int min_idx = i;
                for (j = i + 1; j < size; j++) {
                    if (data[j] < data[min_idx]) {
                        min_idx = j;
                    }
                }
                if (min_idx != i) {
                    temp = data[min_idx];
                    data[min_idx] = data[i];
                    data[i] = temp;
                }
                /* Different exit condition */
                if (i > 10 && (seed % 5 == 0)) {
                    break;
                }
            }
            break;
            
        case 2: /* Custom algorithm with varying loops */
            for (i = 0; i < size; i++) {
                if (data[i] % 2 == 0) {
                    for (j = 0; j < (seed % 10); j++) {
                        data[i] += j;
                    }
                } else {
                    data[i] -= (seed % 7);
                }
            }
            break;
            
        default:
            /* Linear processing */
            for (i = 0; i < size; i++) {
                data[i] = data[i] * 2 - (seed % 3);
            }
            break;
    }
}

/* Hot function - runs many times */
__attribute__((noinline, hot))
static void hot_function_a(int iterations) {
    int i, sum = 0;
    for (i = 0; i < iterations; i++) {
        sum += i * (i % 7);
        if (i % 100 == 0) {
            sum -= i / 2;
        }
    }
    /* Prevent dead code elimination */
    if (sum > 1000000) {
        printf("."); /* Side effect */
    }
}

/* Another hot function with different pattern */
__attribute__((noinline, hot))
static void hot_function_b(int iterations) {
    int i, j;
    for (i = 0; i < iterations / 10; i++) {
        for (j = 0; j < 10; j++) {
            if ((i + j) % 3 == 0) {
                helper_process();
            } else if ((i + j) % 5 == 0) {
                util_process();
            }
        }
    }
}

/* Cold function - runs rarely */
__attribute__((noinline, cold))
static void cold_function_a(void) {
    int x = 0;
    /* Complex but rarely executed logic */
    if (global_seed % 100 == 0) {
        for (x = 0; x < 5; x++) {
            printf("Rare event %d\n", x);
        }
    }
}

/* Another cold function */
__attribute__((noinline, cold))
static void cold_function_b(void) {
    static int call_count = 0;
    call_count++;
    if (call_count < 3) {
        printf("Cold function called %d times\n", call_count);
    }
}

/* Rarely called function */
__attribute__((noinline, cold))
static void rarely_called(void) {
    /* Only called under specific conditions */
    if (global_threshold > 95) {
        printf("Threshold exceeded!\n");
    }
}

/* Frequently called with varying counts */
__attribute__((noinline, hot))
static void frequently_called(int count) {
    int i;
    for (i = 0; i < count; i++) {
        /* Mix of conditions */
        if (i % 2 == 0) {
            global_threshold++;
        } else if (i % 3 == 0) {
            global_threshold--;
        }
    }
}

/* Complex conditional logic */
__attribute__((noinline))
static int complex_conditional(int x, int y, int mode) {
    int result = 0;
    
    if (mode == 0) {
        if (x > y) {
            result = x - y;
        } else if (x < y) {
            result = y - x;
        } else {
            result = x * y;
        }
    } else if (mode == 1) {
        switch (x % 4) {
            case 0:
                result = y + x;
                break;
            case 1:
                result = y - x;
                break;
            case 2:
                result = y * x;
                break;
            case 3:
                result = y / (x ? x : 1);
                break;
        }
    } else {
        result = (x > 0 && y > 0) ? 1 : 0;
        result += (x < 0 || y < 0) ? 2 : 0;
        result += (x == 0 ^ y == 0) ? 4 : 0;
    }
    
    return result;
}

/* Nested loops with varying depth */
__attribute__((noinline))
static void nested_loops(int depth, int width) {
    int i, j, k;
    int counter = 0;
    
    for (i = 0; i < depth; i++) {
        for (j = 0; j < width; j++) {
            for (k = 0; k < (i + j) % 5; k++) {
                counter += complex_conditional(i, j, k % 3);
            }
        }
        /* Early exit based on global seed */
        if (i > depth / 2 && (global_seed % (i + 1)) == 0) {
            break;
        }
    }
    
    /* Prevent optimization */
    if (counter > 1000) {
        printf("*");
    }
}

/* Calculate checksum for verification */
__attribute__((noinline))
static int data_checksum(const int* data, int size) {
    int sum = 0;
    int i;
    
    for (i = 0; i < size; i++) {
        sum = (sum * 31 + data[i]) % 1000000007;
        
        /* Different summation based on position */
        if (i % 3 == 0) {
            sum += data[i] % 17;
        } else if (i % 7 == 0) {
            sum -= data[i] % 13;
        }
    }
    
    return sum;
}

/* Initialize data array with seed-dependent values */
static void initialize_data(int* data, int size, int seed) {
    int i;
    srand(seed);
    
    for (i = 0; i < size; i++) {
        data[i] = rand() % 1000;
        
        /* Create some patterns */
        if (i % 2 == 0) {
            data[i] += seed % 100;
        }
        if (i % 3 == 0) {
            data[i] -= seed % 50;
        }
    }
}

/* Main function with configurable execution paths */
int main(int argc, char** argv) {
    int i;
    int data_size = 100;
    int* data = malloc(data_size * sizeof(int));
    int checksum = 0;
    
    /* Parse command line arguments */
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            global_seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) {
            global_iterations = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--algorithm") == 0 && i + 1 < argc) {
            global_algorithm = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--threshold") == 0 && i + 1 < argc) {
            global_threshold = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--size") == 0 && i + 1 < argc) {
            data_size = atoi(argv[++i]);
            free(data);
            data = malloc(data_size * sizeof(int));
        }
    }
    
    /* Initialize with seed */
    if (global_seed == 0) {
        global_seed = time(NULL) % 10000;
    }
    
    /* Initialize data */
    initialize_data(data, data_size, global_seed);
    
    /* Execute based on configuration */
    printf("Running with seed=%d, iterations=%d, algorithm=%d, threshold=%d\n",
           global_seed, global_iterations, global_algorithm, global_threshold);
    
    /* Call hot functions with varying counts */
    hot_function_a(global_iterations);
    hot_function_b(global_iterations * 2);
    
    /* Call cold functions conditionally */
    if (global_seed % 10 == 0) {
        cold_function_a();
    }
    if (global_seed % 20 == 0) {
        cold_function_b();
    }
    
    /* Call rarely called function */
    if (global_threshold > 90) {
        rarely_called();
    }
    
    /* Call frequently called with varying count */
    frequently_called(global_iterations / 10);
    
    /* Execute main algorithm */
    execute_algorithm(global_algorithm, data, data_size, global_seed);
    
    /* Process data through external function */
    process_data(data, data_size, global_threshold);
    
    /* Complex conditional calls */
    for (i = 0; i < data_size / 10; i++) {
        int result = complex_conditional(data[i], global_seed, i % 3);
        data[i] += result;
    }
    
    /* Nested loops with varying parameters */
    nested_loops(global_iterations / 100, 10);
    
    /* Analyze results */
    analyze_results(data, data_size, global_algorithm);
    
    /* Calculate and output checksum */
    checksum = data_checksum(data, data_size);
    printf("\nChecksum: %d\n", checksum);
    
    /* Cleanup */
    free(data);
    
    return 0;
}
