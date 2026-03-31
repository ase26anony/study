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
static int g_seed = 0;
static int g_iterations = 1000;
static int g_algorithm = 1;
static int g_threshold = 50;

/* Function 1: Hot path function (runs many times) */
__attribute__((hot, noinline))
void hot_function_a(int *counter) {
    for (int i = 0; i < 100; i++) {
        if (rand() % 100 < g_threshold) {
            (*counter) += 2;
        } else {
            (*counter) -= 1;
        }
    }
}

/* Function 2: Another hot function with nested conditionals */
__attribute__((hot, noinline))
void hot_function_b(int *data, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        if (data[i] > 0) {
            if (data[i] % 2 == 0) {
                sum += data[i] * 2;
            } else {
                sum += data[i];
            }
        } else if (data[i] < -10) {
            sum -= data[i];
        } else {
            /* Cold path within hot function */
            sum += 1;
        }
    }
    data[0] = sum;
}

/* Function 3: Cold function (runs rarely) */
__attribute__((cold, noinline))
void cold_function_a(void) {
    static int call_count = 0;
    call_count++;
    
    if (call_count == 1) {
        printf("First call to cold function\n");
    } else if (call_count < 5) {
        printf("Rare call %d to cold function\n", call_count);
    } else {
        /* Even rarer path */
        printf("Very rare call %d\n", call_count);
    }
}

/* Function 4: Mode-dependent function */
__attribute__((noinline))
void mode_dependent_function(int mode) {
    switch (mode) {
        case 1:
            for (int i = 0; i < 10; i++) {
                if (i % 3 == 0) cold_function_a();
            }
            break;
        case 2:
            for (int i = 0; i < 20; i++) {
                if (i % 7 == 0) cold_function_a();
            }
            break;
        case 3:
            /* Different path for mode 3 */
            hot_function_a(&mode);
            break;
        default:
            /* Dead code in most runs */
            #ifdef VARIANT_B
            printf("Variant B specific code\n");
            #endif
            break;
    }
}

/* Function 5: Threshold-sensitive function */
__attribute__((noinline))
void threshold_function(int *data, int size) {
    int hot_counter = 0;
    int cold_counter = 0;
    
    #pragma GCC unroll 0
    for (int i = 0; i < size; i++) {
        if (data[i] > g_threshold) {
            hot_counter++;
            hot_function_a(&hot_counter);
        } else if (data[i] < -g_threshold) {
            cold_counter++;
            cold_function_a();
        } else {
            /* Middle ground - varies with seed */
            if (rand() % 100 < 30) {
                hot_counter++;
            } else {
                cold_counter++;
            }
        }
    }
    
    data[size-1] = hot_counter - cold_counter;
}

/* Function 6: Algorithm implementation A */
__attribute__((noinline))
void algorithm_a(int *data, int size) {
    int swaps;
    do {
        swaps = 0;
        for (int i = 0; i < size - 1; i++) {
            if (data[i] > data[i + 1]) {
                int temp = data[i];
                data[i] = data[i + 1];
                data[i + 1] = temp;
                swaps++;
            }
        }
    } while (swaps > 0);
}

/* Function 7: Algorithm implementation B */
__attribute__((noinline))
void algorithm_b(int *data, int size) {
    /* Quick sort style partitioning */
    if (size <= 1) return;
    
    int pivot = data[size / 2];
    int i = 0, j = size - 1;
    
    while (i <= j) {
        while (data[i] < pivot) i++;
        while (data[j] > pivot) j--;
        
        if (i <= j) {
            int temp = data[i];
            data[i] = data[j];
            data[j] = temp;
            i++;
            j--;
        }
    }
    
    if (j > 0) algorithm_b(data, j + 1);
    if (i < size) algorithm_b(data + i, size - i);
}

/* Main driver with configurable execution paths */
int main(int argc, char *argv[]) {
    /* Parse command line arguments for different run modes */
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
    
    /* Seed RNG for reproducible but varied profiles */
    if (g_seed == 0) {
        g_seed = time(NULL);
    }
    srand(g_seed);
    
    if (g_verbose) {
        printf("Run configuration: seed=%d, iterations=%d, algorithm=%d, threshold=%d\n",
               g_seed, g_iterations, g_algorithm, g_threshold);
    }
    
    /* Create data arrays with varying distributions */
    int data_size = 100 + (rand() % 100);
    int *data1 = malloc(data_size * sizeof(int));
    int *data2 = malloc(data_size * sizeof(int));
    
    for (int i = 0; i < data_size; i++) {
        data1[i] = rand() % 200 - 100;  /* Range: -100 to 100 */
        data2[i] = data1[i] + (rand() % 20 - 10);  /* Slightly different */
    }
    
    /* Execute different code paths based on configuration */
    int hot_counter = 0;
    
    /* Always run hot functions many times */
    for (int i = 0; i < g_iterations / 10; i++) {
        hot_function_a(&hot_counter);
    }
    
    /* Run hot function with data */
    hot_function_b(data1, data_size);
    
    /* Conditional execution based on algorithm choice */
    switch (g_algorithm) {
        case 1:
            algorithm_a(data1, data_size);
            for (int i = 0; i < g_iterations; i++) {
                mode_dependent_function(1);
            }
            break;
        case 2:
            algorithm_b(data1, data_size);
            for (int i = 0; i < g_iterations / 2; i++) {
                mode_dependent_function(2);
            }
            break;
        case 3:
            /* Mixed execution */
            algorithm_a(data1, data_size / 2);
            algorithm_b(data1 + data_size / 2, data_size - data_size / 2);
            for (int i = 0; i < g_iterations / 4; i++) {
                mode_dependent_function(3);
            }
            break;
        default:
            /* Rare path */
            cold_function_a();
            break;
    }
    
    /* Threshold-dependent execution */
    threshold_function(data2, data_size);
    
    /* External function calls (from other object files) */
    process_data_hot(data1, data_size, g_threshold);
    process_data_cold(data2, data_size);
    
    /* Calculate final checksum for deterministic output */
    int result = checksum(data1, data_size) + checksum(data2, data_size) + hot_counter;
    
    if (g_verbose) {
        printf("Result checksum: %d\n", result);
    } else {
        printf("%d\n", result);
    }
    
    free(data1);
    free(data2);
    
    return 0;
}
