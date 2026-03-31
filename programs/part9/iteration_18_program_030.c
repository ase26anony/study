/* gcov_tool_test.c - Multi-run profile generator for gcov-tool overlap analysis */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Function attributes to control instrumentation */
#define NOINLINE __attribute__((noinline))
#define HOT __attribute__((hot))
#define COLD __attribute__((cold))

/* Global configuration */
static int g_mode = 0;
static int g_iterations = 1000;
static int g_seed = 42;
static int g_algorithm = 0;
static int g_use_fullname_test = 0;

/* ========== HOT FUNCTIONS (high execution count) ========== */

HOT NOINLINE
void hot_loop_processor(int *data, int size) {
    long long sum = 0;
    for (int i = 0; i < size * g_iterations / 100; i++) {
        int idx = i % size;
        if (data[idx] > 1000) {
            sum += data[idx] * 2;
        } else if (data[idx] > 500) {
            sum += data[idx];
        } else {
            sum += data[idx] / 2;
        }
        
        /* Branch that varies by mode */
        if (g_mode == 1 && (i % 17) == 0) {
            sum -= 50;
        } else if (g_mode == 2 && (i % 23) == 0) {
            sum += 100;
        }
    }
    printf("Hot loop result: %lld\n", sum);
}

HOT NOINLINE
void matrix_multiply_sim(int size) {
    volatile int temp = 0;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            for (int k = 0; k < size; k++) {
                temp = (temp + i * j * k) % 10007;
                
                /* Different path based on algorithm */
                switch (g_algorithm) {
                    case 0:
                        if (temp > 5000) temp /= 2;
                        break;
                    case 1:
                        if (temp < 3000) temp *= 3;
                        break;
                    case 2:
                        temp = (temp ^ 0xFF) & 0x7F;
                        break;
                    default:
                        temp = temp % 97;
                }
            }
        }
    }
    printf("Matrix sim temp: %d\n", temp);
}

/* ========== COLD FUNCTIONS (low execution count) ========== */

COLD NOINLINE
void cold_initializer(int *data, int size) {
    if (g_seed == 0) g_seed = 1;
    srand(g_seed);
    
    for (int i = 0; i < size; i++) {
        data[i] = rand() % 2000;
        
        /* Different initialization patterns */
        if (g_mode == 1) {
            data[i] += i * 10;
        } else if (g_mode == 2) {
            data[i] -= i * 5;
        }
        
        /* Rare condition */
        if (i == size - 1 && g_use_fullname_test) {
            data[i] = 9999;
        }
    }
}

COLD NOINLINE
void validation_check(int *data, int size) {
    int errors = 0;
    for (int i = 0; i < size; i++) {
        if (data[i] < 0) errors++;
        if (data[i] > 10000) errors++;
    }
    if (errors > 0) {
        printf("Validation errors: %d\n", errors);
    }
}

/* ========== MEDIUM FREQUENCY FUNCTIONS ========== */

NOINLINE
void data_transformer(int *data, int size) {
    for (int i = 0; i < size; i++) {
        /* Multiple branching paths */
        if (data[i] % 3 == 0) {
            data[i] = data[i] * 2 + 1;
        } else if (data[i] % 3 == 1) {
            data[i] = data[i] / 2;
            if (g_mode == 2) data[i] += 100;
        } else {
            data[i] = data[i] ^ 0x55;
        }
        
        /* Threshold check for hot/cold analysis */
        if (data[i] > 1500 && g_algorithm == 1) {
            data[i] -= 200;
        }
    }
}

NOINLINE
void recursive_processor(int *data, int start, int end, int depth) {
    if (start >= end || depth > 5) return;
    
    int mid = (start + end) / 2;
    
    /* Process based on mode */
    if (g_mode == 0) {
        data[mid] = (data[start] + data[end]) / 2;
    } else if (g_mode == 1) {
        data[mid] = data[start] * data[end] % 1000;
    } else {
        data[mid] = (data[start] ^ data[end]) & 0xFF;
    }
    
    /* Recursive calls with varying probability */
    if (rand() % 100 < 70) {
        recursive_processor(data, start, mid, depth + 1);
    }
    if (rand() % 100 < 70) {
        recursive_processor(data, mid, end, depth + 1);
    }
}

/* ========== FULLNAME TEST FUNCTIONS (same name in different "scopes") ========== */

/* Simulating different compilation units with same function name */
NOINLINE
static void helper_function(int *x) {
    *x = (*x + 1) % 100;
    if (g_use_fullname_test) *x += 50;
}

/* Another "helper_function" with different behavior */
NOINLINE  
void helper_function_alt(int *x) {
    *x = (*x * 3) % 97;
    if (g_mode == 1) *x += 10;
}

/* ========== MAIN DRIVER ========== */

int main(int argc, char *argv[]) {
    /* Parse command line arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc) {
            g_mode = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) {
            g_iterations = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            g_seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--algorithm") == 0 && i + 1 < argc) {
            g_algorithm = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--fullname-test") == 0) {
            g_use_fullname_test = 1;
        }
    }
    
    printf("Running with mode=%d, iterations=%d, seed=%d, algorithm=%d\n",
           g_mode, g_iterations, g_seed, g_algorithm);
    
    /* Initialize data */
    const int data_size = 100;
    int *data = malloc(data_size * sizeof(int));
    
    cold_initializer(data, data_size);
    validation_check(data, data_size);
    
    /* Process data through various functions */
    data_transformer(data, data_size);
    
    /* Hot path execution */
    hot_loop_processor(data, data_size);
    
    /* Recursive processing */
    recursive_processor(data, 0, data_size - 1, 0);
    
    /* Matrix simulation (hot) */
    matrix_multiply_sim(20 + g_mode * 5);
    
    /* Helper function calls */
    for (int i = 0; i < data_size / 10; i++) {
        if (i % 3 == 0) {
            helper_function(&data[i]);
        } else {
            helper_function_alt(&data[i]);
        }
    }
    
    /* Final computation and output */
    long long checksum = 0;
    for (int i = 0; i < data_size; i++) {
        checksum = (checksum * 31 + data[i]) % 1000000007;
        
        /* Final conditional that varies by run */
        if (g_mode == 0 && data[i] > 1000) checksum++;
        if (g_mode == 1 && data[i] < 500) checksum += 2;
        if (g_mode == 2 && (data[i] % 7) == 0) checksum += 3;
    }
    
    printf("Final checksum: %lld\n", checksum);
    
    free(data);
    return 0;
}
