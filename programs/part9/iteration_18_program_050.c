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
static int g_mode = 0;
static int g_seed = 0;
static int g_iterations = 1000;
static int g_threshold = 50;

/* Function 1: Hot path function with many branches */
HOT NOINLINE
void hot_function_a(int *data, int size, int threshold) {
    int i, j;
    long sum = 0;
    
    /* Multiple nested conditionals */
    for (i = 0; i < size; i++) {
        if (data[i] > threshold) {
            sum += data[i];
            if (data[i] > threshold * 2) {
                sum *= 2;
                for (j = 0; j < 10; j++) {
                    if (j % 3 == 0) sum += j;
                }
            } else if (data[i] > threshold * 1.5) {
                sum += threshold;
            }
        } else if (data[i] < -threshold) {
            sum -= data[i];
        } else {
            /* Cold path - rarely executed */
            if (g_mode == 2) sum += 1;
        }
        
        /* Switch with multiple cases */
        switch (i % 5) {
            case 0: sum += 1; break;
            case 1: sum += 2; break;
            case 2: sum += 3; break;
            case 3: sum += 4; break;
            case 4: sum += 5; break;
        }
    }
    
    if (g_verbose) printf("Hot function A sum: %ld\n", sum);
}

/* Function 2: Another hot function with different pattern */
HOT NOINLINE
void hot_function_b(int *data, int size, int threshold) {
    int i;
    long product = 1;
    
    for (i = 0; i < size; i++) {
        /* Complex conditional chain */
        if (data[i] % 2 == 0) {
            product *= (data[i] > 0 ? data[i] : 1);
            if (data[i] > threshold) {
                product += threshold;
                if (g_mode == 1) product *= 2;
            }
        } else {
            product += data[i];
            if (data[i] < 0 && g_mode == 3) {
                product /= 2;
            }
        }
        
        /* Loop with early exit */
        int k = 0;
        while (k < 3) {
            if (product > 1000000) break;
            product += k;
            k++;
        }
    }
    
    if (g_verbose) printf("Hot function B product: %ld\n", product);
}

/* Function 3: Cold function - rarely called */
COLD NOINLINE
void cold_function_c(int *data, int size) {
    int i;
    static int call_count = 0;
    call_count++;
    
    /* This function only does work in specific modes */
    if (g_mode == 1 || g_mode == 3) {
        for (i = 0; i < size / 10; i++) {
            if (data[i] % 7 == 0) {
                data[i] = -data[i];
            }
        }
    }
    
    if (g_verbose) printf("Cold function C called %d times\n", call_count);
}

/* Function 4: Mode-dependent function */
NOINLINE
void mode_specific_function(int *data, int size) {
    int i;
    
    switch (g_mode) {
        case 0: /* Baseline mode */
            for (i = 0; i < size; i++) {
                data[i] = data[i] * 2;
            }
            break;
            
        case 1: /* Aggressive mode */
            for (i = 0; i < size; i++) {
                if (data[i] > 0) {
                    data[i] *= 3;
                } else {
                    data[i] /= 2;
                }
            }
            break;
            
        case 2: /* Conservative mode */
            for (i = 0; i < size; i++) {
                data[i] = data[i] > 0 ? data[i] + 10 : data[i] - 10;
            }
            break;
            
        case 3: /* Random mode */
            for (i = 0; i < size; i++) {
                data[i] += rand() % 20 - 10;
            }
            break;
    }
}

/* Function 5: Recursive function for depth coverage */
NOINLINE
int recursive_function(int n, int depth) {
    if (depth > 10) return n;
    if (n <= 1) return 1;
    
    int result;
    if (n % 2 == 0) {
        result = recursive_function(n / 2, depth + 1) + depth;
    } else {
        result = recursive_function(3 * n + 1, depth + 1) - depth;
    }
    
    /* Conditional based on mode */
    if (g_mode == 2) result *= 2;
    else if (g_mode == 3) result /= 2;
    
    return result;
}

/* Function 6: Loop-intensive function */
HOT NOINLINE
void loop_intensive_function(int *data, int size) {
    int i, j;
    
    /* Outer loop - hot */
    for (i = 0; i < g_iterations; i++) {
        /* Inner loop - varies by mode */
        int inner_loops = (g_mode == 1) ? 100 : 50;
        for (j = 0; j < inner_loops; j++) {
            int idx = (i * j) % size;
            if (idx < size) {
                data[idx] += (i * j) % 100;
                
                /* Nested condition */
                if (data[idx] > 1000) {
                    data[idx] = 1000;
                    if (g_verbose && j % 1000 == 0) {
                        printf("Capped at 1000\n");
                    }
                }
            }
        }
        
        /* Early break condition */
        if (i > 500 && g_mode == 3) break;
    }
}

/* Function 7: Threshold-based function */
NOINLINE
void threshold_function(int *data, int size) {
    int i;
    int hot_count = 0, cold_count = 0;
    
    for (i = 0; i < size; i++) {
        if (data[i] > g_threshold) {
            hot_count++;
            data[i] = data[i] * 2;  /* Hot path transformation */
            
            /* Additional hot processing */
            if (data[i] > g_threshold * 3) {
                data[i] /= 2;
                hot_count++;
            }
        } else {
            cold_count++;
            data[i] = data[i] / 2;  /* Cold path transformation */
        }
    }
    
    if (g_verbose) {
        printf("Hot elements: %d, Cold elements: %d\n", hot_count, cold_count);
    }
}

/* Main driver */
int main(int argc, char *argv[]) {
    int i;
    
    /* Parse command line arguments */
    for (i = 1; i < argc; i++) {
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
        } else if (strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [--mode N] [--seed N] [--iterations N] [--threshold N] [--verbose]\n", argv[0]);
            return 0;
        }
    }
    
    /* Seed RNG based on mode and seed */
    if (g_seed == 0) {
        g_seed = time(NULL) + g_mode * 1000;
    }
    srand(g_seed);
    
    if (g_verbose) {
        printf("Mode: %d, Seed: %d, Iterations: %d, Threshold: %d\n",
               g_mode, g_seed, g_iterations, g_threshold);
    }
    
    /* Create test data */
    const int data_size = 1000;
    int *data = malloc(data_size * sizeof(int));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data differently based on mode */
    for (i = 0; i < data_size; i++) {
        switch (g_mode) {
            case 0: data[i] = rand() % 100; break;
            case 1: data[i] = rand() % 200 - 100; break;  /* Includes negatives */
            case 2: data[i] = rand() % 50; break;         /* Smaller range */
            case 3: data[i] = rand() % 300; break;        /* Larger range */
            default: data[i] = i % 100;
        }
    }
    
    /* Execute functions in different orders based on mode */
    if (g_mode == 0) {
        hot_function_a(data, data_size, g_threshold);
        hot_function_b(data, data_size, g_threshold);
        cold_function_c(data, data_size);
    } else if (g_mode == 1) {
        hot_function_b(data, data_size, g_threshold);
        mode_specific_function(data, data_size);
        loop_intensive_function(data, data_size);
    } else if (g_mode == 2) {
        cold_function_c(data, data_size);
        threshold_function(data, data_size);
        recursive_function(data_size / 10, 0);
    } else if (g_mode == 3) {
        loop_intensive_function(data, data_size);
        hot_function_a(data, data_size, g_threshold);
        mode_specific_function(data, data_size);
        threshold_function(data, data_size);
    }
    
    /* Always call some functions for overlap */
    mode_specific_function(data, data_size);
    threshold_function(data, data_size);
    
    /* Process data with external functions */
    process_data_hot(data, data_size, g_threshold);
    process_data_cold(data, data_size);
    
    /* Calculate and output checksum */
    int result = checksum(data, data_size);
    printf("Result: %d\n", result);
    
    free(data);
    return 0;
}
