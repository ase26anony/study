#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Forward declarations for functions in other files */
void process_data(int* data, int size, int mode);
void analyze_results(int* data, int size, int threshold);
void hot_function(int iterations);
void cold_function(void);
int helper_calc(int a, int b, int mode);
void nested_conditions(int value, int depth);

/* Global configuration */
typedef struct {
    int seed;
    int iterations;
    int algorithm;
    int threshold;
    int verbose;
    int mode;
} Config;

/* Function targeting -f (function-level) option */
__attribute__((noinline)) 
void complex_function_a(int* arr, int n, int param) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (param > 0) {
            arr[i] = arr[i] * 2 + param;
            if (i % 3 == 0) {
                sum += arr[i] * 3;
            } else if (i % 3 == 1) {
                sum += arr[i] * 2;
            } else {
                sum += arr[i];
            }
        } else {
            arr[i] = arr[i] / 2 - param;
            switch (i % 4) {
                case 0: sum += arr[i] * 4; break;
                case 1: sum += arr[i] * 3; break;
                case 2: sum += arr[i] * 2; break;
                default: sum += arr[i]; break;
            }
        }
        
        /* Nested loop for hot/cold differentiation */
        for (int j = 0; j < (param % 5); j++) {
            arr[i] += j;
        }
    }
    printf("Function A result: %d\n", sum);
}

/* Another function for function-level analysis */
__attribute__((noinline))
void complex_function_b(int* arr, int n, int param) {
    int product = 1;
    for (int i = 0; i < n; i++) {
        if (arr[i] > 0) {
            product *= (arr[i] % 10) + 1;
            if (product > 1000000) {
                product /= 10;
            }
        } else if (arr[i] < 0) {
            product /= 2;
        } else {
            product += 1;
        }
        
        /* Different execution path based on param */
        if (param % 2 == 0) {
            for (int k = 0; k < 3; k++) {
                arr[i] += k * param;
            }
        } else {
            arr[i] -= param;
        }
    }
    printf("Function B result: %d\n", product);
}

/* Hot function targeting -h and -t options */
__attribute__((hot, noinline))
void hot_loop_function(int iterations) {
    volatile int counter = 0;
    for (int i = 0; i < iterations * 1000; i++) {
        counter += i % 7;
        if (counter > 1000) {
            counter /= 2;
        }
        
        /* Nested hot loop */
        for (int j = 0; j < 10; j++) {
            counter += j % 3;
        }
    }
    printf("Hot loop completed: %d\n", counter);
}

/* Cold function for contrast */
__attribute__((cold, noinline))
void cold_function_simple(void) {
    printf("Cold function executed\n");
    /* Minimal execution */
    int x = 0;
    for (int i = 0; i < 3; i++) {
        x += i;
    }
}

static void parse_arguments(int argc, char** argv, Config* config) {
    /* Default values */
    config->seed = time(NULL);
    config->iterations = 1000;
    config->algorithm = 1;
    config->threshold = 50;
    config->verbose = 0;
    config->mode = 0;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            config->seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) {
            config->iterations = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--algorithm") == 0 && i + 1 < argc) {
            config->algorithm = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--threshold") == 0 && i + 1 < argc) {
            config->threshold = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--verbose") == 0) {
            config->verbose = 1;
        } else if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc) {
            config->mode = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [--seed N] [--iterations M] [--algorithm A] "
                   "[--threshold T] [--verbose] [--mode M]\n", argv[0]);
            exit(0);
        }
    }
}

int main(int argc, char** argv) {
    Config config;
    parse_arguments(argc, argv, &config);
    
    /* Seed RNG for different execution paths */
    srand(config.seed);
    
    /* Create data array with variable size */
    int data_size = 100 + (config.seed % 100);
    int* data = malloc(data_size * sizeof(int));
    
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data differently based on seed */
    for (int i = 0; i < data_size; i++) {
        data[i] = rand() % 1000;
        if (config.mode == 1) {
            data[i] += i * 10;
        } else if (config.mode == 2) {
            data[i] -= i * 5;
        }
    }
    
    /* Execute different code paths based on algorithm */
    switch (config.algorithm) {
        case 1:
            complex_function_a(data, data_size, config.mode);
            hot_loop_function(config.iterations / 10);
            break;
        case 2:
            complex_function_b(data, data_size, config.mode);
            hot_loop_function(config.iterations);
            break;
        case 3:
            complex_function_a(data, data_size, config.mode);
            complex_function_b(data, data_size, config.mode);
            hot_loop_function(config.iterations * 2);
            break;
        default:
            complex_function_a(data, data_size, config.mode);
            break;
    }
    
    /* Call functions from other compilation units */
    process_data(data, data_size, config.mode);
    analyze_results(data, data_size, config.threshold);
    
    /* Conditional execution for coverage variability */
    if (config.iterations > 5000) {
        hot_function(config.iterations / 1000);
    } else {
        cold_function();
    }
    
    /* Execute hot/cold functions based on threshold */
    if (config.threshold > 75) {
        hot_loop_function(100);
    } else {
        cold_function_simple();
    }
    
    /* Nested conditions for branch coverage */
    for (int i = 0; i < 10; i++) {
        nested_conditions(i, config.mode % 3);
    }
    
    /* Calculate checksum for verification */
    unsigned long checksum = 0;
    for (int i = 0; i < data_size; i++) {
        checksum = (checksum * 31 + data[i]) % 1000000007;
    }
    
    printf("Final checksum: %lu\n", checksum);
    printf("Config: seed=%d, iterations=%d, algorithm=%d, threshold=%d, mode=%d\n",
           config.seed, config.iterations, config.algorithm, 
           config.threshold, config.mode);
    
    free(data);
    return 0;
}
