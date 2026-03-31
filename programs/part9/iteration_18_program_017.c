/* gcov_tool_test.c - Program to generate varied GCOV profiles for overlap analysis */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ========== OBJECT-LEVEL SEPARATION (for -o flag) ========== */
/* Functions in this file will be compiled separately for object-level analysis */

/* Hot function attribute for compiler hints */
#define HOT __attribute__((hot))
#define COLD __attribute__((cold))
#define NOINLINE __attribute__((noinline))

/* ========== FUNCTION DECLARATIONS ========== */
/* These will be defined in different compilation units */
extern void process_data(int* data, int size, int mode);
extern void analyze_results(int* data, int size, int threshold);
extern void validate_output(int checksum, int expected);

/* ========== NAMESPACE SIMULATION (for -F flag) ========== */
/* Using static functions with same name in different scopes */
static void helper_function(int x); /* This one */
static void helper_function(int x) { /* Implementation later */ }

/* Different "namespace" via struct */
struct math_ops {
    static int multiply(int a, int b);
    static int divide(int a, int b);
};

/* ========== HOT/COLD FUNCTION DEFINITIONS ========== */
/* Function that runs many times - should be HOT */
HOT NOINLINE void hot_loop_processor(int* data, int size, int iterations) {
    volatile int temp; /* volatile to prevent optimization */
    for (int iter = 0; iter < iterations; iter++) {
        for (int i = 0; i < size; i++) {
            /* Various branches for coverage */
            if (data[i] > 1000) {
                data[i] = data[i] / 2;
            } else if (data[i] < 100) {
                data[i] = data[i] * 3 + 1;
            } else {
                data[i] = data[i] - 50;
            }
            
            /* Nested condition */
            if (i % 3 == 0) {
                if (data[i] % 2 == 0) {
                    temp = data[i] * 2;
                } else {
                    temp = data[i] / 2;
                }
            }
        }
    }
}

/* Function that runs rarely - should be COLD */
COLD NOINLINE void cold_error_handler(int error_code) {
    switch (error_code) {
        case 1:
            fprintf(stderr, "Error 1: Invalid input\n");
            break;
        case 2:
            fprintf(stderr, "Error 2: Memory allocation failed\n");
            break;
        case 3:
            fprintf(stderr, "Error 3: Computation overflow\n");
            break;
        case 4:
            fprintf(stderr, "Error 4: Underflow detected\n");
            break;
        case 5:
            fprintf(stderr, "Error 5: Timeout occurred\n");
            break;
        default:
            fprintf(stderr, "Error %d: Unknown error\n", error_code);
    }
}

/* ========== VARIABLE EXECUTION FUNCTIONS (for different runs) ========== */
NOINLINE int algorithm_a(int* data, int size, int seed) {
    int sum = 0;
    srand(seed);
    
    for (int i = 0; i < size; i++) {
        /* Branch probability depends on seed */
        if (rand() % 100 < 70) { /* 70% true in some runs */
            data[i] = data[i] * 2;
            sum += data[i];
        } else {
            data[i] = data[i] / 2;
            sum -= data[i];
        }
        
        /* Nested branch with different probability */
        if (i % 2 == 0) {
            if (rand() % 100 < 30) { /* 30% true */
                data[i] += 1;
            }
        }
    }
    return sum;
}

NOINLINE int algorithm_b(int* data, int size, int seed) {
    int product = 1;
    srand(seed * 2); /* Different seed pattern */
    
    for (int i = 0; i < size; i++) {
        /* Different branch probabilities */
        if (rand() % 100 < 40) { /* 40% true */
            data[i] = data[i] + 100;
            product *= (data[i] % 1000) + 1;
        } else if (rand() % 100 < 80) { /* 40% true */
            data[i] = data[i] - 50;
            product /= (data[i] % 100) + 1;
        } else { /* 20% true */
            data[i] = data[i] * 3;
            product += data[i];
        }
    }
    return product;
}

/* ========== FULLY QUALIFIED NAME SIMULATION ========== */
/* Static function with same name as in another file */
static void helper_function(int x) {
    /* Different implementation than other file */
    if (x > 0) {
        printf("Positive helper: %d\n", x);
    } else {
        printf("Non-positive helper: %d\n", x);
    }
}

/* Another static with same name in different scope */
static void process_item(int item) {
    /* Complex branching */
    switch (item % 5) {
        case 0: printf("Case 0\n"); break;
        case 1: printf("Case 1\n"); break;
        case 2: printf("Case 2\n"); break;
        case 3: printf("Case 3\n"); break;
        case 4: printf("Case 4\n"); break;
    }
}

/* ========== MAIN PROGRAM WITH VARIABLE PATHS ========== */
int main(int argc, char** argv) {
    int mode = 1;
    int seed = 42;
    int iterations = 1000;
    int data_size = 100;
    int hot_threshold = 5000;
    
    /* Parse command line arguments for different run profiles */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc) {
            mode = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) {
            iterations = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--size") == 0 && i + 1 < argc) {
            data_size = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--threshold") == 0 && i + 1 < argc) {
            hot_threshold = atoi(argv[++i]);
        }
    }
    
    printf("Running with mode=%d, seed=%d, iterations=%d, size=%d\n", 
           mode, seed, iterations, data_size);
    
    /* Initialize data with seed-dependent values */
    int* data = (int*)malloc(data_size * sizeof(int));
    if (!data) {
        cold_error_handler(2);
        return 1;
    }
    
    srand(seed);
    for (int i = 0; i < data_size; i++) {
        data[i] = rand() % 2000;
    }
    
    int result = 0;
    
    /* Branch based on mode - creates different execution paths */
    switch (mode) {
        case 1:
            /* Path 1: Heavy hot loop usage */
            hot_loop_processor(data, data_size, iterations);
            result = algorithm_a(data, data_size, seed);
            break;
            
        case 2:
            /* Path 2: Different algorithm mix */
            hot_loop_processor(data, data_size, iterations / 2);
            result = algorithm_b(data, data_size, seed);
            break;
            
        case 3:
            /* Path 3: More cold code execution */
            for (int i = 0; i < 3; i++) {
                cold_error_handler(i);
            }
            result = algorithm_a(data, data_size, seed * 3);
            break;
            
        case 4:
            /* Path 4: Mixed execution */
            hot_loop_processor(data, data_size, iterations * 2); /* Very hot */
            result = algorithm_b(data, data_size, seed);
            for (int i = 0; i < data_size; i += 10) {
                process_item(data[i] % 100);
            }
            break;
            
        default:
            /* Path 5: Default with all functions */
            hot_loop_processor(data, data_size, iterations);
            result = algorithm_a(data, data_size, seed);
            result += algorithm_b(data, data_size, seed + 1);
            helper_function(result % 100);
    }
    
    /* Additional processing based on threshold */
    if (result > hot_threshold) {
        /* Hot path */
        for (int i = 0; i < data_size; i++) {
            data[i] = data[i] % 100;
        }
    } else {
        /* Cold path */
        cold_error_handler(0);
    }
    
    /* Call external functions (from other object files) */
    process_data(data, data_size, mode);
    analyze_results(data, data_size, hot_threshold);
    
    /* Calculate final checksum for output */
    int checksum = 0;
    for (int i = 0; i < data_size; i++) {
        checksum = (checksum * 31 + data[i]) % 1000000;
    }
    
    validate_output(checksum, result);
    
    printf("Final checksum: %d\n", checksum);
    printf("Result: %d\n", result);
    
    free(data);
    return 0;
}
