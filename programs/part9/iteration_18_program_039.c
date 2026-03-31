#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Forward declarations for functions in other files */
extern void process_data_hot(int *data, int size, int threshold);
extern void process_data_cold(int *data, int size);
extern int search_algorithm(int *data, int size, int target, int mode);
extern void matrix_operations(int size, int mode);

/* Global configuration */
static int verbose = 0;
static int use_fullname_demo = 0; /* For demonstrating -F option */

/* Different modes to vary execution paths */
typedef enum {
    MODE_QUICK,
    MODE_NORMAL,
    MODE_EXHAUSTIVE,
    MODE_RANDOM
} RunMode;

/* __attribute__((noinline)) ensures functions aren't inlined */
__attribute__((noinline))
static void parse_arguments(int argc, char *argv[], int *seed, RunMode *mode, 
                           int *iterations, int *threshold) {
    *seed = 42;  /* Default seed */
    *mode = MODE_NORMAL;
    *iterations = 1000;
    *threshold = 50;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            *seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc) {
            if (strcmp(argv[i+1], "quick") == 0) *mode = MODE_QUICK;
            else if (strcmp(argv[i+1], "normal") == 0) *mode = MODE_NORMAL;
            else if (strcmp(argv[i+1], "exhaustive") == 0) *mode = MODE_EXHAUSTIVE;
            else if (strcmp(argv[i+1], "random") == 0) *mode = MODE_RANDOM;
            i++;
        } else if (strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) {
            *iterations = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--threshold") == 0 && i + 1 < argc) {
            *threshold = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--verbose") == 0) {
            verbose = 1;
        } else if (strcmp(argv[i], "--fullname-demo") == 0) {
            use_fullname_demo = 1;
        }
    }
}

/* Hot function - runs many times */
__attribute__((hot))
static void hot_loop_operation(int *data, int size, int iterations) {
    long long sum = 0;
    for (int i = 0; i < iterations; i++) {
        /* This loop runs many times - will be "hot" */
        for (int j = 0; j < size; j++) {
            if (data[j] > 0) {
                sum += data[j];
            } else {
                sum -= data[j];
            }
        }
        
        /* Nested conditions for branch coverage */
        if (i % 100 == 0 && sum > 1000000) {
            data[0] = sum % 1000;
        } else if (i % 50 == 0) {
            data[size-1] = i % 100;
        }
    }
    
    if (verbose) {
        printf("Hot loop completed, sum: %lld\n", sum);
    }
}

/* Cold function - runs few times */
__attribute__((cold))
static void cold_initialization(int *data, int size, int seed) {
    srand(seed);
    for (int i = 0; i < size; i++) {
        data[i] = rand() % 1000;
        
        /* Multiple branches for coverage */
        if (i % 3 == 0) {
            data[i] *= 2;
        } else if (i % 5 == 0) {
            data[i] /= 2;
        } else {
            data[i] += seed % 100;
        }
    }
}

/* Function with complex decision tree */
__attribute__((noinline))
static int complex_decision_maker(int value, RunMode mode) {
    int result = value;
    
    switch (mode) {
        case MODE_QUICK:
            if (value < 100) result *= 2;
            else if (value < 500) result += 100;
            else result /= 2;
            break;
            
        case MODE_NORMAL:
            for (int i = 0; i < 10; i++) {
                if (value % (i+2) == 0) {
                    result += i;
                }
            }
            break;
            
        case MODE_EXHAUSTIVE:
            #pragma GCC unroll 0
            for (int i = 0; i < 50; i++) {
                if (value > i * 20) {
                    result -= i;
                }
            }
            break;
            
        case MODE_RANDOM:
            result = rand() % 1000;
            break;
    }
    
    /* More branching */
    if (result > 1000) {
        return result % 1000;
    } else if (result < 0) {
        return -result;
    } else {
        return result;
    }
}

/* Another function for -f (function-level) option */
__attribute__((noinline))
static void data_transformation(int *data, int size, int factor) {
    for (int i = 0; i < size; i++) {
        int original = data[i];
        
        if (factor > 0) {
            data[i] = (data[i] * factor) % 10000;
            if (data[i] > 5000) {
                data[i] = 5000;
            }
        } else {
            data[i] = data[i] / (-factor);
        }
        
        /* Nested condition */
        if (original != data[i] && verbose) {
            printf("Transformed %d -> %d\n", original, data[i]);
        }
    }
}

/* Static function with same name as in another file - for -F option */
static void helper_function(int *data, int size) {
    /* This is test_program.c's helper_function */
    for (int i = 0; i < size; i += 2) {
        data[i] = (data[i] * 3) / 2;
    }
}

int main(int argc, char *argv[]) {
    int seed, iterations, threshold;
    RunMode mode;
    
    parse_arguments(argc, argv, &seed, &mode, &iterations, &threshold);
    
    const int data_size = 1000;
    int *data = malloc(data_size * sizeof(int));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with different patterns based on mode */
    cold_initialization(data, data_size, seed);
    
    /* Call hot function - will have high counts */
    hot_loop_operation(data, data_size, iterations);
    
    /* Process with varying complexity based on mode */
    for (int i = 0; i < data_size; i++) {
        data[i] = complex_decision_maker(data[i], mode);
    }
    
    /* Transform data */
    data_transformation(data, data_size, mode + 1);
    
    /* Call object file functions */
    process_data_hot(data, data_size, threshold);
    process_data_cold(data, data_size);
    
    /* Perform search with different algorithms */
    int target = seed % 1000;
    int found = search_algorithm(data, data_size, target, mode);
    
    /* Matrix operations for additional coverage */
    matrix_operations(50, mode);
    
    /* Call local helper if fullname demo is enabled */
    if (use_fullname_demo) {
        helper_function(data, data_size);
    }
    
    /* Calculate checksum for verification */
    long long checksum = 0;
    for (int i = 0; i < data_size; i++) {
        checksum = (checksum * 31 + data[i]) % 1000000007;
    }
    
    printf("Result checksum: %lld\n", checksum);
    printf("Search result: %s\n", found ? "FOUND" : "NOT FOUND");
    
    free(data);
    return 0;
}
