/* gcov_tool_test.c - Multi-run profile generator for gcov-tool testing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Forward declarations for functions in different compilation units */
extern void process_data(int* data, int size, int threshold);
extern void analyze_results(const int* data, int size, int mode);

/* Global configuration */
static int g_verbose = 0;
static int g_hot_threshold = 1000;

/* ========== HOT FUNCTIONS (high execution counts) ========== */

/* Mark as hot to influence compiler heuristics */
__attribute__((hot, noinline))
static void hot_loop_processor(int* data, int size, int multiplier) {
    long long sum = 0;
    /* This loop runs many times - will be "hot" */
    for (int i = 0; i < size * multiplier; i++) {
        int idx = i % size;
        data[idx] = (data[idx] * 1103515245 + 12345) & 0x7fffffff;
        sum += data[idx];
        
        /* Branch with varying probability */
        if (data[idx] % 3 == 0) {
            data[idx] >>= 1;
        } else if (data[idx] % 7 == 0) {
            data[idx] <<= 1;
        }
    }
    if (g_verbose) printf("Hot loop sum: %lld\n", sum);
}

__attribute__((hot, noinline))
static void matrix_multiply(int size, int iterations) {
    /* Simulated matrix multiplication - many iterations */
    for (int iter = 0; iter < iterations; iter++) {
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                int val = (i * j + iter) % 256;
                /* Multiple conditional branches */
                if (val > 128) {
                    val = 255 - val;
                } else if (val > 64) {
                    val = val * 2;
                }
                
                if (iter % 2 == 0) {
                    val = ~val & 0xff;
                }
            }
        }
    }
}

/* ========== COLD FUNCTIONS (low execution counts) ========== */

__attribute__((cold, noinline))
static void cold_initializer(int* data, int size, int seed) {
    /* Called only once per run */
    srand(seed);
    for (int i = 0; i < size; i++) {
        data[i] = rand() % 1000;
    }
    
    /* Rarely executed branch */
    if (seed == 0xdeadbeef) {  /* Never true in normal runs */
        printf("Impossible seed!\n");
    }
}

__attribute__((cold, noinline))
static void validation_check(const int* data, int size) {
    /* Called once at the end */
    int errors = 0;
    for (int i = 0; i < size; i++) {
        if (data[i] < 0) errors++;
    }
    if (errors > 0 && g_verbose) {
        printf("Validation found %d errors\n", errors);
    }
}

/* ========== MEDIUM FREQUENCY FUNCTIONS ========== */

__attribute__((noinline))
static void data_transform(int* data, int size, int mode) {
    /* Execution count depends on mode */
    switch (mode) {
        case 1:
            for (int i = 0; i < size; i++) {
                data[i] = data[i] * 2 + 1;
            }
            break;
        case 2:
            for (int i = 0; i < size; i++) {
                if (i % 2 == 0) {
                    data[i] = data[i] / 2;
                } else {
                    data[i] = data[i] * 3;
                }
            }
            break;
        case 3:
            for (int i = 0; i < size; i++) {
                data[i] = (data[i] << 2) | (data[i] >> 6);
            }
            break;
        default:
            /* Rare path */
            for (int i = 0; i < size; i++) {
                data[i] = ~data[i];
            }
    }
}

__attribute__((noinline))
static int recursive_function(int n, int depth) {
    /* Recursive function with varying depth */
    if (n <= 0 || depth >= 10) return 1;
    
    int result = 0;
    if (n % 2 == 0) {
        result = recursive_function(n / 2, depth + 1) + 
                 recursive_function(n - 1, depth + 1);
    } else {
        result = recursive_function(n - 2, depth + 1) * 2;
    }
    
    return result + n;
}

/* ========== MAIN PROGRAM ========== */

int main(int argc, char** argv) {
    int mode = 1;
    int seed = 42;
    int iterations = 100;
    int data_size = 1000;
    
    /* Parse command line arguments to vary execution paths */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc) {
            mode = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) {
            iterations = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--size") == 0 && i + 1 < argc) {
            data_size = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--verbose") == 0) {
            g_verbose = 1;
        } else if (strcmp(argv[i], "--hot-threshold") == 0 && i + 1 < argc) {
            g_hot_threshold = atoi(argv[++i]);
        }
    }
    
    if (g_verbose) {
        printf("Mode: %d, Seed: %d, Iterations: %d, Size: %d\n", 
               mode, seed, iterations, data_size);
    }
    
    /* Allocate and initialize data */
    int* data = malloc(data_size * sizeof(int));
    if (!data) return 1;
    
    /* Call cold function (once per run) */
    cold_initializer(data, data_size, seed);
    
    /* Vary execution based on mode */
    long long total = 0;
    
    /* Hot path - many iterations */
    if (mode == 1 || mode == 3) {
        hot_loop_processor(data, data_size, iterations);
        matrix_multiply(50, iterations / 10);
    }
    
    /* Medium frequency operations */
    data_transform(data, data_size, mode);
    
    /* Process with external function (different compilation unit) */
    process_data(data, data_size, g_hot_threshold);
    
    /* Recursive calls - depth varies with seed */
    int rec_depth = seed % 15;
    total += recursive_function(rec_depth, 0);
    
    /* More mode-dependent execution */
    if (mode == 2 || mode == 4) {
        for (int i = 0; i < iterations; i++) {
            for (int j = 0; j < data_size / 10; j++) {
                data[j] = (data[j] + i) % 1000;
                if (data[j] > 500) {
                    data[j] -= 250;
                } else if (data[j] < 100) {
                    data[j] += 150;
                }
            }
        }
    }
    
    /* Analyze results with external function */
    analyze_results(data, data_size, mode);
    
    /* Final validation (cold) */
    validation_check(data, data_size);
    
    /* Compute checksum for verification */
    long long checksum = 0;
    for (int i = 0; i < data_size; i++) {
        checksum = (checksum * 31 + data[i]) & 0xffffffff;
    }
    checksum += total;
    
    printf("Result: %llx\n", checksum);
    
    free(data);
    return 0;
}
