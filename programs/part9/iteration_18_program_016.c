/* gcov_tool_test.c - Multi-run profile generator for gcov-tool overlap analysis */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Forward declarations for functions in different compilation units */
extern void process_data_hot(int *data, int size, int threshold);
extern void process_data_cold(int *data, int size);
extern int  external_algorithm(int *data, int size, int mode);

/* Global configuration */
static int g_verbose = 0;
static int g_iterations = 1000;
static int g_seed = 42;
static int g_mode = 0;
static int g_threshold = 50;

/* Hot function - runs many times */
__attribute__((hot)) 
void hot_function_a(int *data, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        if (data[i] > g_threshold) {
            sum += data[i] * 2;  /* Hot path */
        } else {
            sum += data[i];      /* Cold path */
        }
        
        /* Nested condition for branch coverage */
        if (i % 3 == 0) {
            if (data[i] % 2 == 0) {
                data[i] >>= 1;
            } else {
                data[i] <<= 1;
            }
        } else if (i % 5 == 0) {
            data[i] = -data[i];
        }
    }
    
    if (g_verbose) printf("Hot function A processed %d elements\n", size);
}

/* Another hot function with different pattern */
__attribute__((hot))
void hot_function_b(int *data, int size) {
    long long product = 1;
    for (int i = 0; i < size; i++) {
        /* Complex branching */
        switch (data[i] % 4) {
            case 0:
                product *= (data[i] + 1);
                break;
            case 1:
                product *= (data[i] - 1);
                break;
            case 2:
                product *= (data[i] * 2);
                break;
            case 3:
                product *= (data[i] / 2);
                break;
        }
        
        /* Loop with varying iterations */
        for (int j = 0; j < (data[i] % 10); j++) {
            product += j;
        }
    }
    
    if (g_verbose && (product % 1000 == 0)) {
        printf("Hot function B: large product detected\n");
    }
}

/* Cold function - runs rarely */
__attribute__((cold))
void cold_function_a(int *data, int size) {
    if (size < 10) return;  /* Early return */
    
    int found = 0;
    for (int i = 0; i < size; i++) {
        if (data[i] == g_seed) {
            found = 1;
            break;
        }
    }
    
    if (found && g_verbose) {
        printf("Seed value found in data\n");
    }
}

/* Medium frequency function */
__attribute__((noinline))
void medium_function(int *data, int size, int multiplier) {
    static int call_count = 0;
    call_count++;
    
    if (call_count % 100 == 0 && g_verbose) {
        printf("Medium function called %d times\n", call_count);
    }
    
    for (int i = 0; i < size; i++) {
        /* Conditional with multiple branches */
        if (data[i] < 0) {
            data[i] = abs(data[i]);
        } else if (data[i] > 1000) {
            data[i] = 1000;
        } else {
            data[i] *= multiplier;
        }
    }
}

/* Function with deep nesting */
__attribute__((noinline))
void complex_nested_function(int *data, int size) {
    int result = 0;
    for (int i = 0; i < size; i++) {
        if (data[i] > 0) {
            if (data[i] < 100) {
                if (data[i] % 3 == 0) {
                    result += data[i] * 3;
                } else if (data[i] % 3 == 1) {
                    result += data[i] * 2;
                } else {
                    result += data[i];
                }
            } else {
                result += data[i] / 2;
            }
        } else {
            result -= (-data[i]) / 2;
        }
    }
    
    /* Dead code that might be eliminated by optimizer */
    #ifdef ENABLE_DEAD_CODE
    if (result < 0) {
        printf("This should never print\n");
    }
    #endif
}

/* Main processing function */
void process_all_data(int *data, int size) {
    /* Call sequence depends on mode */
    switch (g_mode) {
        case 0:  /* Mode 0: Hot-heavy */
            for (int i = 0; i < g_iterations / 10; i++) {
                hot_function_a(data, size);
                hot_function_b(data, size);
            }
            medium_function(data, size, 2);
            break;
            
        case 1:  /* Mode 1: Balanced */
            for (int i = 0; i < g_iterations / 100; i++) {
                hot_function_a(data, size);
                medium_function(data, size, 3);
            }
            cold_function_a(data, size);
            complex_nested_function(data, size);
            break;
            
        case 2:  /* Mode 2: Cold-heavy */
            cold_function_a(data, size);
            for (int i = 0; i < 10; i++) {
                medium_function(data, size, 1);
            }
            complex_nested_function(data, size);
            break;
            
        case 3:  /* Mode 3: External algorithm */
            external_algorithm(data, size, g_seed % 3);
            break;
            
        default:
            /* Mix of all functions */
            hot_function_a(data, size);
            medium_function(data, size, 4);
            cold_function_a(data, size);
            complex_nested_function(data, size);
            process_data_hot(data, size, g_threshold);
    }
    
    /* Additional processing based on threshold */
    if (g_threshold > 30) {
        process_data_hot(data, size, g_threshold);
    } else {
        process_data_cold(data, size);
    }
}

/* Helper function to generate data */
int* generate_data(int size) {
    int *data = malloc(size * sizeof(int));
    if (!data) return NULL;
    
    srand(g_seed);
    for (int i = 0; i < size; i++) {
        data[i] = rand() % 1000;
    }
    
    return data;
}

/* Calculate checksum to prevent dead code elimination */
long long calculate_checksum(int *data, int size) {
    long long checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum = (checksum * 31 + data[i]) % 1000000007;
    }
    return checksum;
}

/* Parse command line arguments */
void parse_args(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            g_seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) {
            g_iterations = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc) {
            g_mode = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--threshold") == 0 && i + 1 < argc) {
            g_threshold = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--verbose") == 0) {
            g_verbose = 1;
        } else if (strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [--seed N] [--iterations M] [--mode MODE] [--threshold T] [--verbose]\n", argv[0]);
            exit(0);
        }
    }
}

int main(int argc, char **argv) {
    parse_args(argc, argv);
    
    const int data_size = 1000;
    int *data = generate_data(data_size);
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Process data multiple times to generate varying profiles */
    for (int run = 0; run < g_iterations; run++) {
        if (run % 100 == 0) {
            /* Occasionally modify data */
            data[run % data_size] = (run * g_seed) % 1000;
        }
        
        process_all_data(data, data_size);
    }
    
    /* Output deterministic result */
    long long checksum = calculate_checksum(data, data_size);
    printf("Result: %lld\n", checksum);
    
    free(data);
    return 0;
}
