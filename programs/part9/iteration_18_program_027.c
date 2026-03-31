#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "algorithms.h"
#include "utils.h"
#include "hot_cold.h"

/* Global configuration */
typedef struct {
    int seed;
    int iterations;
    int algorithm;
    int data_size;
    int hot_threshold;
    int use_fullname_test;
} Config;

static void parse_args(int argc, char *argv[], Config *cfg) {
    cfg->seed = time(NULL);
    cfg->iterations = 1000;
    cfg->algorithm = 0;
    cfg->data_size = 1000;
    cfg->hot_threshold = 100;
    cfg->use_fullname_test = 0;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            cfg->seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) {
            cfg->iterations = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--algorithm") == 0 && i + 1 < argc) {
            cfg->algorithm = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--size") == 0 && i + 1 < argc) {
            cfg->data_size = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--hot-threshold") == 0 && i + 1 < argc) {
            cfg->hot_threshold = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--fullname-test") == 0) {
            cfg->use_fullname_test = 1;
        }
    }
}

/* __attribute__((noinline)) ensures function-level profiling */
__attribute__((noinline))
static unsigned long process_data(int *data, int size, Config *cfg) {
    unsigned long checksum = 0;
    
    /* Varying execution paths based on algorithm */
    switch (cfg->algorithm) {
        case 0:
            checksum = bubble_sort_and_sum(data, size);
            break;
        case 1:
            checksum = quick_sort_and_sum(data, size);
            break;
        case 2:
            checksum = insertion_sort_and_sum(data, size);
            break;
        case 3:
            checksum = selection_sort_and_sum(data, size);
            break;
        default:
            checksum = default_algorithm(data, size);
    }
    
    return checksum;
}

__attribute__((noinline))
static void generate_data(int *data, int size, int seed) {
    srand(seed);
    for (int i = 0; i < size; i++) {
        data[i] = rand() % 10000;
    }
}

int main(int argc, char *argv[]) {
    Config cfg;
    parse_args(argc, argv, &cfg);
    
    printf("Running with: seed=%d, iterations=%d, algorithm=%d, size=%d\n",
           cfg.seed, cfg.iterations, cfg.algorithm, cfg.data_size);
    
    /* Allocate and initialize data */
    int *data = malloc(cfg.data_size * sizeof(int));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    generate_data(data, cfg.data_size, cfg.seed);
    
    /* Main processing loop - creates hot/cold regions */
    unsigned long total_checksum = 0;
    
    /* HOT region - runs many times */
    for (int i = 0; i < cfg.iterations; i++) {
        /* Vary data slightly each iteration */
        if (i % 100 == 0) {
            data[rand() % cfg.data_size] = rand() % 10000;
        }
        
        total_checksum += process_data(data, cfg.data_size, &cfg);
        
        /* Call hot functions */
        if (i < cfg.hot_threshold) {
            hot_function_a(i);
            hot_function_b(i);
        } else {
            cold_function_a(i);
        }
    }
    
    /* COLD region - runs once */
    if (cfg.use_fullname_test) {
        /* Test functions with same name in different scopes */
        utils_process(data, cfg.data_size);
        algorithms_process(data, cfg.data_size);
    }
    
    /* Final verification */
    verify_result(data, cfg.data_size, total_checksum);
    
    printf("Checksum: %lu\n", total_checksum);
    
    free(data);
    return 0;
}
