/* gcov_tool_test.c - Program to generate varied GCOV profiles for overlap analysis */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Forward declarations for functions in different compilation units */
extern void process_data_hot(int *data, int size, int threshold);
extern void process_data_cold(int *data, int size);
extern int external_compute(int x, int mode);

/* Global configuration */
static int g_verbose = 0;
static int g_iterations = 1000;
static int g_seed = 42;
static int g_algorithm = 1;
static int g_threshold = 500;

/* Hot function - runs many times */
__attribute__((hot, noinline))
static void hot_function_a(int *counter) {
    for (int i = 0; i < 100; i++) {
        (*counter) += i * 2;
        if (i % 3 == 0) {
            (*counter) -= i;
        } else if (i % 7 == 0) {
            (*counter) *= 2;
        }
    }
}

/* Another hot function with different pattern */
__attribute__((hot, noinline))
static void hot_function_b(int *data, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        if (data[i] > g_threshold) {
            sum += data[i] * 3;
        } else if (data[i] < -g_threshold) {
            sum -= data[i] * 2;
        } else {
            sum += data[i];
        }
        
        /* Nested loop for more execution counts */
        for (int j = 0; j < 5; j++) {
            if ((i + j) % 2 == 0) {
                sum += j;
            }
        }
    }
    data[0] = sum;
}

/* Cold function - runs rarely */
__attribute__((cold, noinline))
static void cold_function_a(void) {
    static int call_count = 0;
    call_count++;
    
    if (call_count == 1) {
        if (g_verbose) printf("Cold function first call\n");
    } else if (call_count < 5) {
        if (g_verbose) printf("Cold function early call %d\n", call_count);
    } else {
        if (g_verbose) printf("Cold function late call %d\n", call_count);
    }
}

/* Function with complex branching */
__attribute__((noinline))
static int complex_branching(int x, int y, int mode) {
    int result = 0;
    
    switch (mode % 4) {
        case 0:
            result = x + y;
            if (result > 1000) result /= 2;
            break;
        case 1:
            result = x - y;
            if (result < 0) result = -result;
            break;
        case 2:
            result = x * y;
            if (result % 7 == 0) result += 1;
            break;
        case 3:
            result = (x + y) / 2;
            if (x > y) result += 100;
            break;
        default:
            result = 0;
    }
    
    /* Additional conditional logic */
    if (x > g_threshold && y < g_threshold) {
        result += 500;
    } else if (x < g_threshold && y > g_threshold) {
        result -= 500;
    }
    
    return result;
}

/* Function called in different patterns based on algorithm */
__attribute__((noinline))
static void algorithm_specific(int *data, int size, int algo) {
    int temp = 0;
    
    if (algo == 1) {
        /* Algorithm 1: Linear processing */
        for (int i = 0; i < size; i++) {
            data[i] = complex_branching(data[i], i, 0);
            if (i % 100 == 0) cold_function_a();
        }
    } else if (algo == 2) {
        /* Algorithm 2: Skip processing */
        for (int i = 0; i < size; i += 2) {
            data[i] = complex_branching(data[i], i, 1);
        }
    } else {
        /* Algorithm 3: Reverse processing */
        for (int i = size - 1; i >= 0; i--) {
            data[i] = complex_branching(data[i], i, 2);
        }
    }
    
    /* Always do some work */
    hot_function_a(&temp);
    data[size-1] += temp;
}

/* Main processing function */
__attribute__((noinline))
static void process_main(int *data, int size) {
    int hot_counter = 0;
    
    /* Vary execution based on seed */
    srand(g_seed);
    
    /* Initial processing */
    for (int i = 0; i < size; i++) {
        data[i] = rand() % 2000 - 1000;
    }
    
    /* Call hot functions many times */
    for (int i = 0; i < g_iterations; i++) {
        hot_function_a(&hot_counter);
        if (i % 100 == 0) {
            hot_function_b(data, size);
        }
        
        /* Vary calls based on algorithm */
        switch (g_algorithm) {
            case 1:
                if (i % 50 == 0) cold_function_a();
                break;
            case 2:
                if (i % 200 == 0) cold_function_a();
                break;
            case 3:
                if (i % 500 == 0) cold_function_a();
                break;
        }
    }
    
    /* Algorithm-specific processing */
    algorithm_specific(data, size, g_algorithm);
    
    /* Call external functions */
    process_data_hot(data, size, g_threshold);
    process_data_cold(data, size);
    
    /* Final computation using external object */
    data[0] = external_compute(data[0], g_algorithm);
}

/* Parse command line arguments */
static void parse_args(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            g_seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) {
            g_iterations = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--algorithm") == 0 && i + 1 < argc) {
            g_algorithm = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--threshold") == 0 && i + 1 < argc) {
            g_threshold = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--verbose") == 0) {
            g_verbose = 1;
        } else if (strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [--seed N] [--iterations M] [--algorithm A] [--threshold T] [--verbose]\n", argv[0]);
            exit(0);
        }
    }
}

int main(int argc, char **argv) {
    const int data_size = 1000;
    int *data = malloc(data_size * sizeof(int));
    int checksum = 0;
    
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Parse command line to vary behavior */
    parse_args(argc, argv);
    
    if (g_verbose) {
        printf("Running with seed=%d, iterations=%d, algorithm=%d, threshold=%d\n",
               g_seed, g_iterations, g_algorithm, g_threshold);
    }
    
    /* Main processing */
    process_main(data, data_size);
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < data_size; i++) {
        checksum = (checksum * 31 + data[i]) % 1000000007;
    }
    
    printf("Result checksum: %d\n", checksum);
    
    free(data);
    return 0;
}
