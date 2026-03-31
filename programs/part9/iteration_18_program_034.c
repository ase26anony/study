/* gcov_tool_test.c - Multi-run profile generator for gcov-tool overlap analysis */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Forward declarations for functions in different compilation units */
extern void process_data_hot(int *data, int size, int threshold);
extern void process_data_cold(int *data, int size);
extern int checksum(const int *data, int size);

/* __attribute__ directives to control instrumentation */
#define NOINLINE __attribute__((noinline))
#define HOT __attribute__((hot))
#define COLD __attribute__((cold))

/* Global configuration */
static int g_verbose = 0;
static int g_iterations = 1000;
static int g_seed = 42;
static int g_algorithm = 1;
static int g_threshold = 500;

/* Function 1: Hot path function with many branches */
HOT NOINLINE
void hot_function_a(int *data, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        if (data[i] > g_threshold) {
            sum += data[i] * 2;
        } else if (data[i] > g_threshold / 2) {
            sum += data[i];
        } else {
            sum += 1;
        }
        
        /* Nested condition */
        if (i % 3 == 0) {
            if (sum > 10000) {
                sum -= 500;
            }
        }
    }
    
    if (g_verbose) {
        printf("Hot function A completed: sum=%d\n", sum);
    }
}

/* Function 2: Another hot function with different pattern */
HOT NOINLINE
void hot_function_b(int *data, int size) {
    long long product = 1;
    for (int i = 0; i < size && i < 50; i++) {
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
                product /= 2;
                if (product < 1) product = 1;
                break;
            default:
                product += data[i];
        }
        
        /* Complex condition */
        if ((i & 1) && (product > 1000000)) {
            product >>= 2;
        }
    }
    
    if (g_verbose && (product > 1000)) {
        printf("Hot function B: large product detected\n");
    }
}

/* Function 3: Cold function called rarely */
COLD NOINLINE
void cold_function_a(int *data, int size) {
    if (size < 2) return;
    
    /* This function has many branches but is called infrequently */
    int min = data[0], max = data[0];
    for (int i = 1; i < size; i++) {
        if (data[i] < min) {
            min = data[i];
        } else if (data[i] > max) {
            max = data[i];
        }
        
        /* Rare condition */
        if (data[i] == g_seed) {
            if (g_verbose) {
                printf("Found seed value at index %d\n", i);
            }
        }
    }
    
    /* Multiple exit points */
    if (max - min > 1000) {
        return;
    }
}

/* Function 4: Medium frequency function */
NOINLINE
void medium_function(int *data, int size, int mode) {
    static int call_count = 0;
    call_count++;
    
    if (mode == 1) {
        for (int i = 0; i < size; i += 2) {
            data[i] = (data[i] * 13) % 997;
        }
    } else if (mode == 2) {
        for (int i = 1; i < size; i += 2) {
            data[i] = (data[i] * 17) % 991;
        }
    } else {
        /* Default mode with many conditions */
        for (int i = 0; i < size; i++) {
            if (data[i] % 2 == 0) {
                data[i] >>= 1;
            } else if (data[i] % 3 == 0) {
                data[i] *= 3;
            } else if (data[i] % 5 == 0) {
                data[i] /= 5;
            } else {
                data[i] += call_count;
            }
        }
    }
}

/* Function 5: Algorithm selector with complex control flow */
NOINLINE
void algorithm_dispatcher(int *data, int size) {
    switch (g_algorithm) {
        case 1:
            /* Bubble sort variant */
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
            /* Selection sort variant */
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
            /* Insertion sort variant */
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
            /* Linear processing */
            for (int i = 0; i < size; i++) {
                data[i] = (data[i] * 1103515245 + 12345) & 0x7fffffff;
            }
    }
}

/* Function 6: Recursive function for call graph depth */
NOINLINE
int recursive_function(int n, int depth) {
    if (depth > 10) return 1;
    if (n <= 1) return 1;
    
    int result = 0;
    if (n % 2 == 0) {
        result = recursive_function(n / 2, depth + 1) + 
                 recursive_function(n - 1, depth + 1);
    } else {
        result = recursive_function(n - 2, depth + 1) * 2;
    }
    
    return result % 1000;
}

/* Function 7: Final processing with many conditions */
NOINLINE
void finalize_data(int *data, int size) {
    int positive = 0, negative = 0, zero = 0;
    
    for (int i = 0; i < size; i++) {
        if (data[i] > 0) {
            positive++;
            data[i] = (data[i] * 31) % 1000;
        } else if (data[i] < 0) {
            negative++;
            data[i] = (-data[i] * 17) % 1000;
        } else {
            zero++;
            data[i] = i % 100;
        }
        
        /* Additional conditional processing */
        if (i % 7 == 0) {
            data[i] += g_seed % 100;
        }
    }
    
    if (g_verbose) {
        printf("Finalize: pos=%d, neg=%d, zero=%d\n", positive, negative, zero);
    }
}

/* Main execution controller */
int main(int argc, char *argv[]) {
    /* Parse command-line arguments for different execution modes */
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
            /* Alternative mode specification */
            if (strcmp(argv[i + 1], "fast") == 0) {
                g_iterations = 100;
                g_threshold = 100;
            } else if (strcmp(argv[i + 1], "slow") == 0) {
                g_iterations = 10000;
                g_threshold = 900;
            }
            i++;
        }
    }
    
    /* Initialize random generator with seed */
    srand(g_seed);
    
    /* Create data array */
    int data_size = 100 + (g_seed % 100);
    int *data = (int *)malloc(data_size * sizeof(int));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Fill with random data */
    for (int i = 0; i < data_size; i++) {
        data[i] = rand() % 1000;
    }
    
    /* Execute functions in different patterns based on configuration */
    int total_checksum = 0;
    
    /* Hot path - executed many times */
    for (int iter = 0; iter < g_iterations; iter++) {
        hot_function_a(data, data_size);
        
        if (iter % 10 == 0) {
            hot_function_b(data, data_size);
        }
        
        if (iter % 100 == 0) {
            medium_function(data, data_size, g_algorithm);
        }
    }
    
    /* Cold path - executed once */
    cold_function_a(data, data_size);
    
    /* Algorithm-specific processing */
    algorithm_dispatcher(data, data_size);
    
    /* Process external functions (from other compilation units) */
    if (g_threshold > 300) {
        process_data_hot(data, data_size, g_threshold);
    } else {
        process_data_cold(data, data_size);
    }
    
    /* Recursive calls */
    for (int i = 0; i < 5; i++) {
        int rec_result = recursive_function(10 + i, 0);
        if (g_verbose) {
            printf("Recursive(%d) = %d\n", 10 + i, rec_result);
        }
    }
    
    /* Final processing */
    finalize_data(data, data_size);
    
    /* Calculate checksum for verification */
    total_checksum = checksum(data, data_size);
    
    /* Output deterministic result */
    printf("RESULT: seed=%d, algo=%d, iterations=%d, checksum=%d\n",
           g_seed, g_algorithm, g_iterations, total_checksum);
    
    free(data);
    return 0;
}
