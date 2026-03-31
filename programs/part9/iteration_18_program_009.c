#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

/* Forward declarations for functions in other files */
extern void process_data(int* data, int size, int mode, int threshold);
extern void analyze_results(int* data, int size, int algorithm);
extern void hot_loop_function(int iterations);
extern void cold_helper_function(void);

/* Global configuration */
static int verbose = 0;
static int use_fullname_test = 0;

/* Function targeting -f (function-level summary) */
__attribute__((noinline))
static void data_generator(int* buffer, int size, int seed) {
    srand(seed);
    for (int i = 0; i < size; i++) {
        buffer[i] = rand() % 1000;
    }
}

/* Another function for -f */
__attribute__((noinline))
static void data_transform(int* data, int size, int factor) {
    for (int i = 0; i < size; i++) {
        /* Complex branching for varied profiles */
        if (data[i] > 500) {
            data[i] = data[i] * factor / 100;
        } else if (data[i] > 200) {
            data[i] = data[i] + factor;
        } else {
            data[i] = data[i] - factor;
        }
        
        /* Nested conditionals */
        if (i % 3 == 0) {
            data[i] += (factor % 7);
        } else if (i % 3 == 1) {
            data[i] -= (factor % 5);
        }
    }
}

/* Hot function for -h and -t flags */
__attribute__((hot))
static void hot_processing(int* data, int size, int iterations) {
    for (int iter = 0; iter < iterations; iter++) {
        for (int i = 0; i < size; i++) {
            /* Intensive computation */
            data[i] = (data[i] * 1103515245 + 12345) & 0x7fffffff;
            
            /* Branch with high probability */
            if (data[i] % 100 < 95) {  /* 95% taken */
                data[i] >>= 1;
            }
        }
    }
}

/* Cold function for contrast */
__attribute__((cold))
static void cold_validation(int* data, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        if (data[i] < 0) {
            printf("Warning: negative value at %d\n", i);
            sum++;
        }
    }
    if (sum > size / 10) {
        printf("Many negative values: %d\n", sum);
    }
}

/* Function with same name as in other file (for -F fullname testing) */
static void utility_function(int x) {
    /* Different implementation than in utils.c */
    printf("Main utility: %d\n", x * 2);
}

/* Main processing pipeline */
static int run_pipeline(int data_size, int seed, int mode, 
                       int hot_iterations, int threshold) {
    int* data = malloc(data_size * sizeof(int));
    if (!data) return -1;
    
    /* Vary execution paths based on mode */
    switch (mode) {
        case 1:
            data_generator(data, data_size, seed);
            data_transform(data, data_size, 50);
            hot_processing(data, data_size, hot_iterations);
            break;
        case 2:
            data_generator(data, data_size, seed + 1);
            hot_processing(data, data_size, hot_iterations / 2);
            data_transform(data, data_size, 75);
            break;
        case 3:
            data_generator(data, data_size, seed * 2);
            for (int i = 0; i < 3; i++) {
                data_transform(data, data_size, 25 + i * 10);
            }
            break;
        default:
            data_generator(data, data_size, seed);
            break;
    }
    
    /* Call functions from other object files */
    process_data(data, data_size, mode, threshold);
    analyze_results(data, data_size, mode % 3);
    
    /* Conditional cold function call */
    if (mode % 4 == 0) {
        cold_validation(data, data_size);
    }
    
    /* Call local utility */
    if (use_fullname_test) {
        utility_function(data_size);
    }
    
    /* Calculate checksum for verification */
    int checksum = 0;
    for (int i = 0; i < data_size; i++) {
        checksum = (checksum * 31 + data[i]) & 0xFFFF;
    }
    
    free(data);
    return checksum;
}

int main(int argc, char** argv) {
    int data_size = 1000;
    int seed = 42;
    int mode = 1;
    int hot_iterations = 1000;
    int threshold = 500;
    int runs = 1;
    
    /* Parse command line arguments for variability */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--size") == 0 && i + 1 < argc) {
            data_size = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc) {
            mode = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) {
            hot_iterations = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--threshold") == 0 && i + 1 < argc) {
            threshold = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--runs") == 0 && i + 1 < argc) {
            runs = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--verbose") == 0) {
            verbose = 1;
        } else if (strcmp(argv[i], "--fullname-test") == 0) {
            use_fullname_test = 1;
        }
    }
    
    /* Run multiple times for varied profiles */
    int total_checksum = 0;
    for (int run = 0; run < runs; run++) {
        int current_seed = seed + run * 17;
        int current_mode = (mode + run) % 4;
        int current_iterations = hot_iterations * (1 + run % 3);
        
        if (verbose) {
            printf("Run %d: seed=%d, mode=%d, iterations=%d\n",
                   run, current_seed, current_mode, current_iterations);
        }
        
        int result = run_pipeline(data_size, current_seed, current_mode,
                                 current_iterations, threshold);
        total_checksum = (total_checksum + result) & 0xFFFF;
    }
    
    printf("Final checksum: %04X\n", total_checksum);
    return 0;
}
