/* gcov_tool_test.c - Program to generate varied GCOV profiles for testing gcov-tool overlap analysis */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Forward declarations for functions in different compilation units */
extern void process_data_hot(int *data, int size, int threshold);
extern void process_data_cold(int *data, int size);
extern int external_computation(int x, int mode);

/* Global configuration */
static int g_verbose = 0;
static int g_mode = 0;
static int g_seed = 0;
static int g_iterations = 1000;
static int g_threshold = 50;

/* __attribute__ directives to control instrumentation */
#define NOINLINE __attribute__((noinline))
#define HOT __attribute__((hot))
#define COLD __attribute__((cold))

/* Function 1: Hot function with high execution count */
HOT NOINLINE
void hot_loop_processor(int *array, int size) {
    int sum = 0;
    /* This loop runs many times - will be "hot" */
    for (int i = 0; i < g_iterations; i++) {
        for (int j = 0; j < size; j++) {
            if (array[j] > g_threshold) {
                sum += array[j] * 2;
            } else if (array[j] < -g_threshold) {
                sum -= array[j];
            } else {
                sum += 1;
            }
        }
    }
    if (g_verbose) printf("Hot loop sum: %d\n", sum);
}

/* Function 2: Cold function with low execution count */
COLD NOINLINE
void cold_data_validator(int *array, int size) {
    int valid = 1;
    /* This runs only once - will be "cold" */
    for (int i = 0; i < size; i++) {
        if (array[i] == 0) {
            valid = 0;
            break;
        }
    }
    if (!valid && g_verbose) {
        printf("Warning: Zero value found in data\n");
    }
}

/* Function 3: Mode-dependent execution path */
NOINLINE
void mode_specific_operation(int *data, int size) {
    switch (g_mode) {
        case 1:
            /* Path 1: Process all elements */
            for (int i = 0; i < size; i++) {
                data[i] = data[i] * 3 + 1;
            }
            break;
        case 2:
            /* Path 2: Process only even indices */
            for (int i = 0; i < size; i += 2) {
                data[i] = data[i] / 2;
            }
            break;
        case 3:
            /* Path 3: Conditional processing */
            for (int i = 0; i < size; i++) {
                if (data[i] % 2 == 0) {
                    data[i] += 100;
                } else {
                    data[i] -= 50;
                }
            }
            break;
        default:
            /* Default path */
            for (int i = 0; i < size; i++) {
                data[i] = abs(data[i]);
            }
    }
}

/* Function 4: Recursive function for varied call depth */
NOINLINE
int recursive_computation(int n, int depth) {
    if (depth <= 0 || n <= 1) {
        return 1;
    }
    
    if (n % 2 == 0) {
        return recursive_computation(n / 2, depth - 1) + 1;
    } else {
        return recursive_computation(3 * n + 1, depth - 1) + n;
    }
}

/* Function 5: Complex nested conditionals */
NOINLINE
void complex_decision_maker(int *data, int size) {
    for (int i = 0; i < size; i++) {
        int val = data[i];
        
        if (val > 1000) {
            if (val % 3 == 0) {
                data[i] = val * 2;
            } else if (val % 3 == 1) {
                data[i] = val + 777;
            } else {
                data[i] = val - 555;
            }
        } else if (val > 100) {
            data[i] = (val << 2) | 0x0F;
        } else if (val > 10) {
            data[i] = val * val;
        } else {
            data[i] = -val;
        }
        
        /* Additional nested condition */
        if (g_mode == 2 && data[i] < 0) {
            data[i] = external_computation(data[i], g_mode);
        }
    }
}

/* Function 6: Seed-dependent random processing */
NOINLINE
void random_branch_executor(int *data, int size) {
    /* Use seed to create different branch patterns across runs */
    srand(g_seed);
    
    for (int i = 0; i < size; i++) {
        int r = rand() % 100;
        
        if (r < 30) {
            data[i] += r;
        } else if (r < 60) {
            data[i] -= r;
        } else if (r < 80) {
            data[i] *= (r % 5) + 1;
        } else {
            data[i] /= (r % 3) + 1;
        }
    }
}

/* Function 7: Threshold-based filtering (for -t option) */
NOINLINE
void threshold_filter(int *data, int size, int threshold) {
    int count = 0;
    for (int i = 0; i < size; i++) {
        if (data[i] > threshold) {
            count++;
            if (g_mode == 1) {
                data[i] = threshold;
            }
        } else if (data[i] < -threshold) {
            count--;
            if (g_mode == 2) {
                data[i] = -threshold;
            }
        }
    }
    if (g_verbose) printf("Threshold filter count: %d\n", count);
}

/* Main driver function */
int main(int argc, char *argv[]) {
    /* Parse command-line arguments to create varied profiles */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc) {
            g_mode = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            g_seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) {
            g_iterations = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--threshold") == 0 && i + 1 < argc) {
            g_threshold = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--verbose") == 0) {
            g_verbose = 1;
        }
    }
    
    if (g_seed == 0) {
        g_seed = time(NULL);
    }
    
    /* Initialize data array */
    const int data_size = 100;
    int *data = malloc(data_size * sizeof(int));
    srand(g_seed);
    
    for (int i = 0; i < data_size; i++) {
        data[i] = rand() % 2000 - 1000;  /* Values between -1000 and 1000 */
    }
    
    /* Execute functions in different orders based on mode */
    cold_data_validator(data, data_size);
    
    if (g_mode == 1) {
        random_branch_executor(data, data_size);
        mode_specific_operation(data, data_size);
        hot_loop_processor(data, data_size);
    } else if (g_mode == 2) {
        mode_specific_operation(data, data_size);
        complex_decision_maker(data, data_size);
        threshold_filter(data, data_size, g_threshold);
    } else {
        complex_decision_maker(data, data_size);
        random_branch_executor(data, data_size);
        hot_loop_processor(data, data_size);
    }
    
    /* Call external functions (from other object files) */
    process_data_hot(data, data_size, g_threshold);
    process_data_cold(data, data_size);
    
    /* Recursive computation with mode-dependent depth */
    int rec_depth = (g_mode == 1) ? 20 : (g_mode == 2) ? 10 : 5;
    int rec_result = recursive_computation(data[0] % 100, rec_depth);
    
    /* Final processing */
    threshold_filter(data, data_size, g_threshold * 2);
    
    /* Calculate checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < data_size; i++) {
        checksum += data[i];
        checksum ^= (checksum << 13);
        checksum ^= (checksum >> 7);
        checksum ^= (checksum << 17);
    }
    checksum += rec_result;
    
    printf("Result checksum: %llu (Mode: %d, Seed: %d)\n", 
           checksum, g_mode, g_seed);
    
    free(data);
    return 0;
}
