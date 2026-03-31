/* test_gcov_overlap.c - Main program for gcov-tool overlap testing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Forward declarations for functions in other files */
extern void process_data(int* data, int size, int mode);
extern void hot_loop_function(int iterations);
extern void cold_function(void);
extern int search_algorithm(int* data, int size, int target, int algo);
extern void math_operations(int seed);
extern void data_transform(int* data, int size, int transform_type);

/* Global configuration */
static int verbose = 0;
static int use_fullname_test = 0;

/* Function with complex branching for coverage */
__attribute__((noinline))
static void branchy_function(int seed, int* counter) {
    if (seed < 0) {
        *counter += 1;
        if (seed < -100) {
            *counter *= 2;
        } else if (seed < -50) {
            *counter += 100;
        }
    } else if (seed > 0) {
        for (int i = 0; i < (seed % 10); i++) {
            *counter += i;
            if (i == 5) {
                *counter -= 10;
            }
        }
    } else {
        *counter = 0;
    }
    
    switch (seed % 4) {
        case 0:
            *counter += 1000;
            break;
        case 1:
            *counter += 2000;
            if (*counter > 1500) {
                *counter -= 500;
            }
            break;
        case 2:
            *counter += 3000;
            break;
        case 3:
            *counter += 4000;
            break;
        default:
            *counter = -1;
    }
}

/* Hot function - runs many times */
__attribute__((hot))
static void hot_helper(int iterations) {
    int sum = 0;
    for (int i = 0; i < iterations; i++) {
        sum += i;
        if (i % 100 == 0) {
            sum -= 50;
        }
    }
    /* Prevent optimization */
    if (sum < 0) printf("Impossible\n");
}

/* Cold function - runs rarely */
__attribute__((cold))
static void cold_helper(void) {
    printf("Cold function executed\n");
}

/* Another function with different name but same simple name */
static void duplicate_name_func(int x) {
    printf("Main file duplicate_name_func: %d\n", x);
}

int main(int argc, char** argv) {
    int seed = 42;
    int iterations = 1000;
    int algorithm = 1;
    int mode = 0;
    int hot_threshold = 5000;
    
    /* Parse command line arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) {
            iterations = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--algorithm") == 0 && i + 1 < argc) {
            algorithm = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc) {
            mode = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--verbose") == 0) {
            verbose = 1;
        } else if (strcmp(argv[i], "--fullname-test") == 0) {
            use_fullname_test = 1;
        }
    }
    
    srand(seed);
    
    /* Create test data */
    int data_size = 1000;
    int* data = malloc(data_size * sizeof(int));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data differently based on seed */
    for (int i = 0; i < data_size; i++) {
        data[i] = rand() % 10000;
        if (seed % 3 == 0) {
            data[i] += i;
        } else if (seed % 3 == 1) {
            data[i] -= i * 2;
        } else {
            data[i] *= (i % 10);
        }
    }
    
    int counter = 0;
    
    /* Execute functions with varying frequencies */
    
    /* Always executed */
    branchy_function(seed, &counter);
    
    /* Hot path - many iterations */
    if (iterations > hot_threshold) {
        hot_loop_function(iterations / 100);
        hot_helper(iterations / 10);
    }
    
    /* Cold path - rarely executed */
    if (seed % 100 == 0) {
        cold_function();
        cold_helper();
    }
    
    /* Process data based on mode */
    process_data(data, data_size, mode);
    
    /* Search with different algorithms */
    int target = data[rand() % data_size];
    int found = search_algorithm(data, data_size, target, algorithm);
    
    /* Math operations - varies by seed */
    math_operations(seed);
    
    /* Data transform - different types based on mode */
    data_transform(data, data_size, mode % 3);
    
    /* Call duplicate name function */
    duplicate_name_func(seed);
    
    /* Calculate checksum for verification */
    long long checksum = 0;
    for (int i = 0; i < data_size; i++) {
        checksum += data[i];
        checksum ^= (data[i] << (i % 16));
    }
    checksum += counter;
    checksum += found;
    
    printf("Result: seed=%d, iterations=%d, algorithm=%d, checksum=%lld\n",
           seed, iterations, algorithm, checksum);
    
    free(data);
    return 0;
}
