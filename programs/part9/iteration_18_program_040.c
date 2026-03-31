/* gcov_tool_test.c - Multi-run profile generator for gcov-tool overlap analysis */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Forward declarations for functions in different compilation units */
extern void process_data_hot(int *data, int size, int threshold);
extern void process_data_cold(int *data, int size);
extern int checksum_array(const int *data, int size);

/* Global configuration */
static int g_verbose = 0;
static int g_iterations = 1000;
static int g_seed = 42;
static int g_algorithm = 1;
static int g_threshold = 500;

/* Function 1: Hot path function (runs many times) */
__attribute__((hot, noinline))
static void hot_function_a(int *counter, int limit) {
    for (int i = 0; i < limit; i++) {
        if (i % 3 == 0) {
            *counter += i * 2;
        } else if (i % 3 == 1) {
            *counter += i / 2;
        } else {
            *counter -= i;
        }
        
        /* Nested conditional */
        if (*counter > 10000) {
            *counter = *counter % 1000;
        }
    }
}

/* Function 2: Another hot function with different pattern */
__attribute__((hot, noinline))
static void hot_function_b(int *data, int size, int multiplier) {
    int local_sum = 0;
    for (int i = 0; i < size; i++) {
        switch (data[i] % 5) {
            case 0:
                local_sum += data[i] * multiplier;
                break;
            case 1:
                local_sum -= data[i];
                break;
            case 2:
                local_sum *= (multiplier % 3) + 1;
                break;
            case 3:
                local_sum /= 2;
                break;
            default:
                local_sum = local_sum ^ data[i];
        }
        
        /* Complex branching */
        if (local_sum < 0) {
            local_sum = -local_sum;
            if (multiplier > 2) {
                local_sum += 100;
            }
        } else if (local_sum > 1000) {
            local_sum >>= 2;
        }
    }
    data[0] = local_sum;
}

/* Function 3: Cold function (rarely called) */
__attribute__((cold, noinline))
static void cold_function_a(void) {
    static int call_count = 0;
    call_count++;
    
    if (call_count == 1) {
        printf("First call to cold function\n");
    } else if (call_count < 5) {
        printf("Cold function called %d times\n", call_count);
    } else {
        /* Rare path */
        printf("Cold function warming up!\n");
    }
}

/* Function 4: Medium frequency function */
__attribute__((noinline))
static void medium_function(int *arr, int n, int mode) {
    int temp = 0;
    
    #pragma GCC unroll 0
    for (int i = 0; i < n; i++) {
        if (mode == 1) {
            arr[i] = arr[i] * 2 + 1;
        } else if (mode == 2) {
            arr[i] = arr[i] / 2 - 1;
        } else {
            /* Different path for overlap analysis */
            arr[i] = (arr[i] << 1) | 0x01;
        }
        
        /* Nested loop with varying iterations */
        for (int j = 0; j < (i % 10); j++) {
            temp += j * arr[i];
        }
    }
    
    if (temp > 1000 && mode == 1) {
        cold_function_a();
    }
}

/* Function 5: Algorithm selector */
__attribute__((noinline))
static int select_algorithm(int seed, int size) {
    int choice = seed % 4;
    
    switch (choice) {
        case 0:
            return 1;  /* Quick path */
        case 1:
            return 2;  /* Medium path */
        case 2:
            return 3;  /* Slow path */
        default:
            /* Complex decision tree */
            if (size > 100) return 1;
            if (size > 50) return 2;
            return 3;
    }
}

/* Function 6: Data initializer with different patterns */
__attribute__((noinline))
static void initialize_data(int *data, int size, int pattern) {
    for (int i = 0; i < size; i++) {
        if (pattern == 1) {
            /* Linear pattern */
            data[i] = i * 2 + 1;
        } else if (pattern == 2) {
            /* Random pattern */
            data[i] = rand() % 1000;
        } else {
            /* Fibonacci-like pattern */
            if (i < 2) {
                data[i] = 1;
            } else {
                data[i] = data[i-1] + data[i-2];
                if (data[i] > 1000) data[i] %= 1000;
            }
        }
        
        /* Additional conditional */
        if (pattern == 3 && i % 7 == 0) {
            data[i] = -data[i];
        }
    }
}

/* Function 7: Main processing orchestrator */
__attribute__((noinline))
static void process_orchestrator(int *data, int size, int algo) {
    int hot_counter = 0;
    
    /* Call hot functions many times */
    for (int iter = 0; iter < g_iterations / 10; iter++) {
        hot_function_a(&hot_counter, 50);
        hot_function_b(data, size > 10 ? 10 : size, algo);
    }
    
    /* Call medium function based on algorithm */
    int mode = select_algorithm(g_seed, size);
    medium_function(data, size, mode);
    
    /* Occasionally call cold function */
    if (hot_counter % 1000 == 0) {
        cold_function_a();
    }
    
    /* Call external functions */
    if (algo == 1 || algo == 3) {
        process_data_hot(data, size, g_threshold);
    } else {
        process_data_cold(data, size);
    }
}

/* Dead code for different build variants */
#ifdef VARIANT_A
__attribute__((noinline))
static void variant_a_specific(int *data, int size) {
    for (int i = 0; i < size; i += 2) {
        data[i] *= 3;
    }
}
#elif defined(VARIANT_B)
__attribute__((noinline))
static void variant_b_specific(int *data, int size) {
    for (int i = 1; i < size; i += 2) {
        data[i] /= 2;
    }
}
#endif

/* Main function with configurable behavior */
int main(int argc, char *argv[]) {
    /* Parse command-line arguments for different run modes */
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
        }
    }
    
    /* Seed RNG differently for each run */
    srand(g_seed);
    
    /* Create data arrays of different sizes */
    int data_size = 100 + (g_seed % 100);
    int *data1 = (int*)malloc(data_size * sizeof(int));
    int *data2 = (int*)malloc(data_size * sizeof(int));
    
    if (!data1 || !data2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data with different patterns based on run parameters */
    initialize_data(data1, data_size, (g_algorithm % 3) + 1);
    initialize_data(data2, data_size, ((g_algorithm + 1) % 3) + 1);
    
    /* Main processing loop with varying execution counts */
    int total = 0;
    for (int i = 0; i < g_iterations; i++) {
        /* Alternate between different processing paths */
        if (i % 3 == 0) {
            process_orchestrator(data1, data_size, g_algorithm);
        } else if (i % 3 == 1) {
            process_orchestrator(data2, data_size, g_algorithm + 1);
        } else {
            /* Different path for overlap analysis */
            hot_function_a(&total, 20);
            medium_function(data1, data_size / 2, 2);
        }
        
        /* Occasionally reset or modify data */
        if (i % 500 == 0 && i > 0) {
            initialize_data(data1, data_size, (i / 500) % 3 + 1);
        }
    }
    
    /* Build variant specific code */
    #ifdef VARIANT_A
    variant_a_specific(data1, data_size);
    #elif defined(VARIANT_B)
    variant_b_specific(data2, data_size);
    #endif
    
    /* Calculate final checksum (prevents dead code elimination) */
    int checksum1 = checksum_array(data1, data_size);
    int checksum2 = checksum_array(data2, data_size);
    int final_result = checksum1 ^ checksum2;
    
    if (g_verbose) {
        printf("Run configuration: seed=%d, iterations=%d, algorithm=%d\n",
               g_seed, g_iterations, g_algorithm);
        printf("Checksums: %d ^ %d = %d\n", checksum1, checksum2, final_result);
    } else {
        printf("%d\n", final_result);
    }
    
    free(data1);
    free(data2);
    
    return 0;
}
