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

/* Function 1: Hot path function with high execution count */
HOT NOINLINE
void hot_function_a(int *data, int size) {
    int sum = 0;
    for (int i = 0; i < g_iterations; i++) {
        for (int j = 0; j < size; j++) {
            if (data[j] > g_threshold) {
                sum += data[j] * 2;  /* Hot branch */
            } else {
                sum += data[j];      /* Cold branch */
            }
        }
    }
    if (g_verbose) printf("Hot function A sum: %d\n", sum);
}

/* Function 2: Another hot function with nested conditionals */
HOT NOINLINE
void hot_function_b(int *data, int size) {
    long long product = 1;
    for (int i = 0; i < g_iterations / 2; i++) {
        for (int j = 0; j < size; j++) {
            switch (data[j] % 5) {
                case 0: product *= 1; break;
                case 1: product *= 2; break;
                case 2: product *= 3; break;
                case 3: product *= 4; break;
                case 4: product *= 5; break;
                default: product = 0; break;
            }
            if (product > 1000000) product = 1;
        }
    }
    if (g_verbose) printf("Hot function B product: %lld\n", product);
}

/* Function 3: Cold function with minimal execution */
COLD NOINLINE
void cold_function_a(int *data, int size) {
    if (size == 0) return;
    
    int min = data[0], max = data[0];
    for (int i = 1; i < size; i++) {
        if (data[i] < min) min = data[i];
        if (data[i] > max) max = data[i];
    }
    if (g_verbose) printf("Cold function A range: %d\n", max - min);
}

/* Function 4: Medium execution function */
NOINLINE
void medium_function(int *data, int size) {
    int count = 0;
    for (int i = 0; i < size; i++) {
        if (data[i] % 2 == 0) {
            count++;
            #ifdef VARIANT_A
            data[i] += 1;
            #else
            data[i] -= 1;
            #endif
        }
    }
    if (g_verbose) printf("Medium function even count: %d\n", count);
}

/* Function 5: Algorithm-specific function */
NOINLINE
void algorithm_specific(int *data, int size) {
    if (g_algorithm == 1) {
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
    } else if (g_algorithm == 2) {
        /* Selection sort variant */
        for (int i = 0; i < size - 1; i++) {
            int min_idx = i;
            for (int j = i + 1; j < size; j++) {
                if (data[j] < data[min_idx]) {
                    min_idx = j;
                }
            }
            int temp = data[min_idx];
            data[min_idx] = data[i];
            data[i] = temp;
        }
    } else {
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
    }
}

/* Function 6: Random walk function - varies greatly between runs */
NOINLINE
void random_walk_function(int *data, int size) {
    srand(g_seed);
    int position = size / 2;
    int steps = 100;
    
    for (int i = 0; i < steps; i++) {
        int direction = rand() % 4;
        switch (direction) {
            case 0: if (position > 0) position--; break;
            case 1: if (position < size - 1) position++; break;
            case 2: position = (position + 2) % size; break;
            case 3: position = (position - 2 + size) % size; break;
        }
        data[position] += rand() % 10;
    }
}

/* Function 7: Threshold-based processing */
NOINLINE
void threshold_function(int *data, int size) {
    int hot_count = 0;
    for (int i = 0; i < size; i++) {
        if (data[i] > g_threshold) {
            hot_count++;
            data[i] = data[i] * 2;  /* Hot path */
        } else {
            data[i] = data[i] / 2;  /* Cold path */
        }
    }
    if (g_verbose) printf("Threshold function hot count: %d\n", hot_count);
}

/* Main execution orchestrator */
int main(int argc, char *argv[]) {
    /* Parse command-line arguments for variability */
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
    
    /* Initialize data array with seed-dependent values */
    const int data_size = 100;
    int *data = malloc(data_size * sizeof(int));
    
    srand(g_seed);
    for (int i = 0; i < data_size; i++) {
        data[i] = rand() % 1000;
    }
    
    /* Execute functions in varying order based on algorithm */
    if (g_algorithm == 1) {
        hot_function_a(data, data_size);
        medium_function(data, data_size);
        cold_function_a(data, data_size);
        algorithm_specific(data, data_size);
        hot_function_b(data, data_size);
    } else if (g_algorithm == 2) {
        random_walk_function(data, data_size);
        threshold_function(data, data_size);
        algorithm_specific(data, data_size);
        hot_function_a(data, data_size);
        cold_function_a(data, data_size);
    } else {
        algorithm_specific(data, data_size);
        hot_function_b(data, data_size);
        medium_function(data, data_size);
        threshold_function(data, data_size);
        random_walk_function(data, data_size);
    }
    
    /* Call external functions (from other compilation units) */
    process_data_hot(data, data_size, g_threshold);
    process_data_cold(data, data_size);
    
    /* Calculate and output deterministic result */
    int result = checksum(data, data_size);
    printf("Result checksum: %d (seed=%d, algo=%d, iter=%d, thresh=%d)\n",
           result, g_seed, g_algorithm, g_iterations, g_threshold);
    
    free(data);
    return 0;
}
