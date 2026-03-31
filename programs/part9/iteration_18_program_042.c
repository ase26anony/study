/* gcov_tool_test.c - Program to generate varied GCOV profiles for testing gcov-tool overlap analysis */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Forward declarations for functions in different compilation units */
extern void process_data_hot(int *data, int size, int threshold);
extern void process_data_cold(int *data, int size);
extern int checksum(const int *data, int size);

/* Global configuration */
static int verbose = 0;
static int use_fullname_demo = 0;

/* ========== Functions for -f (function-level) testing ========== */

/* Hot function - runs many times */
__attribute__((hot, noinline))
void hot_function_a(int *data, int size, int multiplier) {
    long long sum = 0;
    for (int i = 0; i < size; i++) {
        if (data[i] > 100) {           /* Branch with varying probability */
            sum += data[i] * multiplier;
        } else {
            sum += data[i];
        }
        /* Nested loop for high execution count */
        for (int j = 0; j < 10; j++) {
            if (j % 3 == 0) {
                sum += j;
            }
        }
    }
    if (verbose) printf("Hot A sum: %lld\n", sum);
}

/* Another hot function with different pattern */
__attribute__((hot, noinline))
void hot_function_b(int *data, int size, int divisor) {
    long long product = 1;
    for (int i = 0; i < size; i++) {
        switch (data[i] % 5) {         /* Switch with varying cases */
            case 0: product *= 2; break;
            case 1: product *= 3; break;
            case 2: product *= data[i]; break;
            case 3: product /= (divisor ? divisor : 1); break;
            default: product += data[i]; break;
        }
        /* Complex conditional */
        if (data[i] > 50 && data[i] < 200) {
            product -= 1;
        } else if (data[i] >= 200) {
            product += 1000;
        }
    }
    if (verbose && product > 1000000) printf("Large product in B\n");
}

/* Cold function - runs few times */
__attribute__((cold, noinline))
void cold_function_a(int *data, int size) {
    int min = data[0], max = data[0];
    for (int i = 1; i < (size < 5 ? size : 5); i++) {  /* Limited iterations */
        if (data[i] < min) min = data[i];
        if (data[i] > max) max = data[i];
    }
    if (verbose) printf("Cold A range: %d to %d\n", min, max);
}

/* Medium frequency function */
__attribute__((noinline))
void medium_function(int *data, int size, int mode) {
    static int call_count = 0;
    call_count++;
    
    if (mode == 1) {
        for (int i = 0; i < size; i += 2) {
            data[i] = data[i] * 2;
        }
    } else if (mode == 2) {
        for (int i = 1; i < size; i += 2) {
            data[i] = data[i] / 2;
        }
    } else {
        /* Dead code path for some runs */
        #ifdef VARIANT_A
        for (int i = 0; i < size; i++) {
            data[i] = -data[i];
        }
        #endif
    }
    
    if (call_count % 100 == 0 && verbose) {
        printf("Medium function called %d times\n", call_count);
    }
}

/* Function with deep nesting */
__attribute__((noinline))
void complex_nested_function(int *data, int size, int seed) {
    int threshold = seed % 100;
    int count = 0;
    
    for (int i = 0; i < size; i++) {
        if (data[i] > threshold) {
            count++;
            if (data[i] > threshold * 2) {
                for (int j = 0; j < 3; j++) {
                    if (j == 1) {
                        data[i] += j;
                    } else if (j == 2) {
                        data[i] -= 1;
                    }
                }
            }
        } else if (data[i] < threshold / 2) {
            data[i] += 5;
        } else {
            /* Do nothing branch */
        }
    }
    
    if (count > size / 2 && verbose) {
        printf("Many values above threshold\n");
    }
}

/* ========== Functions for -F (fullname) testing ========== */

/* Static function with same name in different contexts */
static void helper_function(int *data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] % 256;
    }
}

/* Namespace simulation using struct */
struct math_ops {
    int (*compute)(int a, int b);
};

static int add_operation(int a, int b) {
    return a + b;
}

static int multiply_operation(int a, int b) {
    return a * b;
}

/* ========== Main algorithm with configurable behavior ========== */

__attribute__((noinline))
void run_algorithm(int *data, int size, int algorithm, int iterations) {
    switch (algorithm) {
        case 1: /* Bubble sort variant */
            for (int iter = 0; iter < iterations; iter++) {
                for (int i = 0; i < size - 1; i++) {
                    if (data[i] > data[i + 1]) {
                        int temp = data[i];
                        data[i] = data[i + 1];
                        data[i + 1] = temp;
                    }
                }
            }
            hot_function_a(data, size, 2);
            break;
            
        case 2: /* Selection sort variant */
            for (int iter = 0; iter < iterations; iter++) {
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
            }
            hot_function_b(data, size, 3);
            break;
            
        case 3: /* Custom algorithm */
            #pragma GCC unroll 0  /* Prevent unrolling */
            for (int i = 0; i < iterations; i++) {
                medium_function(data, size, i % 3);
                if (i % 10 == 0) {
                    cold_function_a(data, size);
                }
                complex_nested_function(data, size, i);
            }
            break;
            
        default:
            /* Linear processing */
            for (int i = 0; i < size; i++) {
                data[i] = (data[i] * 13 + 7) % 1000;
            }
            break;
    }
}

/* ========== Main function with configurable execution ========== */

int main(int argc, char *argv[]) {
    /* Parse command-line arguments */
    int seed = 42;
    int algorithm = 1;
    int data_size = 1000;
    int iterations = 100;
    int threshold = 50;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--algorithm") == 0 && i + 1 < argc) {
            algorithm = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--size") == 0 && i + 1 < argc) {
            data_size = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) {
            iterations = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--threshold") == 0 && i + 1 < argc) {
            threshold = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--verbose") == 0) {
            verbose = 1;
        } else if (strcmp(argv[i], "--fullname-demo") == 0) {
            use_fullname_demo = 1;
        }
    }
    
    /* Initialize random data */
    srand(seed);
    int *data = malloc(data_size * sizeof(int));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    for (int i = 0; i < data_size; i++) {
        data[i] = rand() % 1000;
    }
    
    /* Execute based on configuration */
    if (use_fullname_demo) {
        /* Demonstrate functions that would show full names */
        struct math_ops ops = {add_operation};
        for (int i = 0; i < data_size; i += 2) {
            data[i] = ops.compute(data[i], data[i + 1]);
        }
        ops.compute = multiply_operation;
        for (int i = 0; i < data_size; i += 2) {
            data[i] = ops.compute(data[i], 2);
        }
        helper_function(data, data_size);
    }
    
    /* Main processing with hot/cold paths */
    clock_t start = clock();
    
    /* Always call some functions */
    cold_function_a(data, data_size);
    medium_function(data, data_size, algorithm);
    
    /* Conditional hot path */
    if (iterations > threshold) {
        process_data_hot(data, data_size, threshold);
        for (int i = 0; i < iterations / 100; i++) {
            hot_function_a(data, data_size, 3);
        }
    } else {
        process_data_cold(data, data_size);
        hot_function_b(data, data_size, 2);
    }
    
    /* Run main algorithm */
    run_algorithm(data, data_size, algorithm, iterations);
    
    /* More conditional execution */
    if (seed % 3 == 0) {
        complex_nested_function(data, data_size, seed);
    } else if (seed % 3 == 1) {
        medium_function(data, data_size, 2);
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    /* Calculate and output result */
    int result = checksum(data, data_size);
    printf("Result: %d (seed=%d, algo=%d, size=%d, iter=%d, time=%.3fs)\n",
           result, seed, algorithm, data_size, iterations, elapsed);
    
    if (verbose) {
        printf("Data sample: %d %d %d\n", data[0], data[1], data[2]);
    }
    
    free(data);
    return 0;
}
