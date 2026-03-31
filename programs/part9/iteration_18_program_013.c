#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Forward declarations for functions in other files */
void process_data_hot(int *data, int size, int threshold);
void process_data_cold(int *data, int size);
int analyze_pattern(int *data, int size, int mode);
void complex_algorithm_a(int *data, int size, int seed);
void complex_algorithm_b(int *data, int size, int seed);

/* Global configuration */
typedef struct {
    int seed;
    int iterations;
    int algorithm;
    int threshold;
    int use_full_analysis;
} Config;

/* Hot function - runs many times */
__attribute__((hot)) 
void hot_loop_function(int *data, int size, int iterations) {
    for (int iter = 0; iter < iterations; iter++) {
        int sum = 0;
        for (int i = 0; i < size; i++) {
            if (data[i] > 100) {
                sum += data[i] * 2;
            } else if (data[i] > 50) {
                sum += data[i];
            } else {
                sum += data[i] / 2;
            }
            
            /* Nested condition for more branches */
            switch (data[i] % 4) {
                case 0:
                    data[i] += 1;
                    break;
                case 1:
                    data[i] -= 1;
                    break;
                case 2:
                    data[i] *= 2;
                    break;
                case 3:
                    data[i] /= 2;
                    break;
            }
        }
        
        /* Another level of branching */
        if (sum > 1000) {
            for (int i = 0; i < size / 2; i++) {
                data[i] = (data[i] * sum) % 1000;
            }
        } else if (sum > 500) {
            for (int i = size / 2; i < size; i++) {
                data[i] = (data[i] + sum) % 1000;
            }
        }
    }
}

/* Cold function - runs rarely */
__attribute__((cold))
void cold_analysis_function(int *data, int size) {
    if (size < 10) {
        printf("Data too small for cold analysis\n");
        return;
    }
    
    int rare_counter = 0;
    for (int i = 0; i < size; i++) {
        /* This condition is rarely true */
        if (data[i] == 777) {
            rare_counter++;
            printf("Found rare value at index %d\n", i);
        }
    }
    
    if (rare_counter > 0) {
        printf("Total rare values: %d\n", rare_counter);
    }
}

/* Medium frequency function */
__attribute__((noinline))
void medium_frequency_func(int *data, int size, int mode) {
    static int call_count = 0;
    call_count++;
    
    if (mode == 1) {
        for (int i = 0; i < size; i++) {
            if (i % 2 == 0) {
                data[i] = data[i] * 3 + 1;
            } else {
                data[i] = data[i] / 3 + 5;
            }
        }
    } else if (mode == 2) {
        for (int i = 0; i < size; i++) {
            data[i] = (data[i] * data[i]) % 1000;
        }
    } else {
        for (int i = 0; i < size; i++) {
            data[i] = 1000 - data[i];
        }
    }
    
    /* Complex switch */
    switch (call_count % 5) {
        case 0:
            data[0] += 100;
            break;
        case 1:
            data[size-1] -= 100;
            break;
        case 2:
            data[size/2] *= 2;
            break;
        case 3:
            /* Do nothing */
            break;
        case 4:
            for (int i = 0; i < size && i < 5; i++) {
                data[i] = 0;
            }
            break;
    }
}

/* Function with many execution paths */
__attribute__((noinline))
int multi_path_function(int *data, int size, int threshold) {
    int result = 0;
    int hot_path = 0;
    int cold_path = 0;
    
    for (int i = 0; i < size; i++) {
        if (data[i] > threshold * 2) {
            /* Hot path - frequently executed */
            result += data[i] * 3;
            hot_path++;
            
            if (data[i] > threshold * 3) {
                result += 100;
            } else if (data[i] > threshold * 2.5) {
                result += 50;
            }
        } else if (data[i] > threshold) {
            /* Medium path */
            result += data[i] * 2;
            
            switch (data[i] % 3) {
                case 0:
                    result += 10;
                    break;
                case 1:
                    result += 20;
                    break;
                case 2:
                    result += 30;
                    break;
            }
        } else {
            /* Cold path - rarely executed */
            result += data[i];
            cold_path++;
            
            if (data[i] < threshold / 2) {
                result -= 5;
            }
        }
        
        /* Nested condition */
        if (i % 10 == 0) {
            if (data[i] % 2 == 0) {
                result += 1;
            } else {
                result -= 1;
            }
        }
    }
    
    return result;
}

void initialize_data(int *data, int size, int seed) {
    srand(seed);
    for (int i = 0; i < size; i++) {
        data[i] = rand() % 1000;
    }
}

void print_usage(const char *progname) {
    printf("Usage: %s [options]\n", progname);
    printf("Options:\n");
    printf("  --seed=N          Random seed (default: time(NULL))\n");
    printf("  --iterations=N    Hot loop iterations (default: 1000)\n");
    printf("  --algorithm=N     Algorithm variant 1-3 (default: 1)\n");
    printf("  --threshold=N     Threshold for hot/cold paths (default: 300)\n");
    printf("  --full-analysis   Enable full analysis mode\n");
}

Config parse_args(int argc, char *argv[]) {
    Config config = {
        .seed = (int)time(NULL),
        .iterations = 1000,
        .algorithm = 1,
        .threshold = 300,
        .use_full_analysis = 0
    };
    
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "--seed=", 7) == 0) {
            config.seed = atoi(argv[i] + 7);
        } else if (strncmp(argv[i], "--iterations=", 13) == 0) {
            config.iterations = atoi(argv[i] + 13);
        } else if (strncmp(argv[i], "--algorithm=", 12) == 0) {
            config.algorithm = atoi(argv[i] + 12);
        } else if (strncmp(argv[i], "--threshold=", 12) == 0) {
            config.threshold = atoi(argv[i] + 12);
        } else if (strcmp(argv[i], "--full-analysis") == 0) {
            config.use_full_analysis = 1;
        } else if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            exit(0);
        }
    }
    
    return config;
}

int main(int argc, char *argv[]) {
    Config config = parse_args(argc, argv);
    
    const int DATA_SIZE = 1000;
    int *data = malloc(DATA_SIZE * sizeof(int));
    
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    printf("Running with seed=%d, iterations=%d, algorithm=%d, threshold=%d\n",
           config.seed, config.iterations, config.algorithm, config.threshold);
    
    /* Initialize data with seed */
    initialize_data(data, DATA_SIZE, config.seed);
    
    /* Execute hot function many times */
    hot_loop_function(data, DATA_SIZE, config.iterations);
    
    /* Execute based on algorithm choice */
    switch (config.algorithm) {
        case 1:
            complex_algorithm_a(data, DATA_SIZE, config.seed);
            break;
        case 2:
            complex_algorithm_b(data, DATA_SIZE, config.seed);
            break;
        case 3:
            /* Mix of both */
            complex_algorithm_a(data, DATA_SIZE / 2, config.seed);
            complex_algorithm_b(data + DATA_SIZE / 2, DATA_SIZE / 2, config.seed + 1);
            break;
        default:
            printf("Unknown algorithm, using default\n");
            complex_algorithm_a(data, DATA_SIZE, config.seed);
    }
    
    /* Process data with hot/cold functions */
    process_data_hot(data, DATA_SIZE, config.threshold);
    
    if (config.use_full_analysis) {
        process_data_cold(data, DATA_SIZE);
    }
    
    /* Medium frequency function */
    medium_frequency_func(data, DATA_SIZE, config.algorithm);
    
    /* Multi-path function */
    int result = multi_path_function(data, DATA_SIZE, config.threshold);
    
    /* Pattern analysis */
    int pattern_result = analyze_pattern(data, DATA_SIZE, config.algorithm);
    
    /* Cold function - rarely called */
    if (config.seed % 100 == 0) {  /* Only 1% of runs */
        cold_analysis_function(data, DATA_SIZE);
    }
    
    /* Calculate checksum for verification */
    unsigned long checksum = 0;
    for (int i = 0; i < DATA_SIZE; i++) {
        checksum = (checksum * 31 + data[i]) % 1000000007;
    }
    checksum = (checksum + result + pattern_result) % 1000000007;
    
    printf("Result: %d, Pattern: %d, Checksum: %lu\n", 
           result, pattern_result, checksum);
    
    free(data);
    return 0;
}
