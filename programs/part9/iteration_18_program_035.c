/* gcov_tool_test.c - Generates varied GCOV profiles for overlap analysis */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Forward declarations for functions in different compilation units */
extern void process_data_hot(int *data, int size, int threshold);
extern void process_data_cold(int *data, int size);

/* Global configuration */
static int global_seed = 0;
static int iteration_multiplier = 1;
static int algorithm_mode = 0;

/* __attribute__ directives to control optimization/instrumentation */
#define NOINLINE __attribute__((noinline))
#define HOT __attribute__((hot))
#define COLD __attribute__((cold))

/* ========== HOT FUNCTIONS (high execution counts) ========== */

HOT NOINLINE
void hot_loop_processor(int *array, int size) {
    volatile int sum = 0; /* volatile prevents optimization */
    for (int i = 0; i < size * iteration_multiplier; i++) {
        /* Complex branching with varying probabilities */
        if (i % 3 == 0) {
            sum += array[i % size] * 2;
        } else if (i % 3 == 1 && algorithm_mode == 1) {
            sum += array[i % size] / 2;
        } else {
            sum += array[i % size];
        }
        
        /* Nested condition */
        if (sum > 1000000) {
            sum = sum % 1000;
        }
    }
    printf("[HOT] Processed %d iterations, final sum: %d\n", 
           size * iteration_multiplier, sum);
}

HOT NOINLINE
void matrix_multiply_sim(int rows, int cols) {
    int counter = 0;
    for (int i = 0; i < rows * iteration_multiplier; i++) {
        for (int j = 0; j < cols; j++) {
            /* Branch with seed-dependent behavior */
            if ((i + j + global_seed) % 5 == 0) {
                counter += i * j;
            } else if ((i * j) % 7 == 0) {
                counter -= j;
            } else {
                counter++;
            }
            
            /* Switch with varying case distribution */
            switch ((i + global_seed) % 4) {
                case 0: counter <<= 1; break;
                case 1: counter >>= 1; break;
                case 2: counter ^= 0xFF; break;
                default: counter |= 0x01; break;
            }
        }
    }
    printf("[HOT] Matrix sim: %d ops\n", counter);
}

/* ========== COLD FUNCTIONS (low execution counts) ========== */

COLD NOINLINE
void cold_initializer(int *data, int size) {
    if (size <= 0) return;
    
    /* Different initialization patterns based on mode */
    if (algorithm_mode == 0) {
        for (int i = 0; i < size; i++) {
            data[i] = (i * 13 + global_seed) % 100;
        }
    } else if (algorithm_mode == 1) {
        for (int i = 0; i < size; i++) {
            data[i] = (i * 17 - global_seed) % 100;
        }
    } else {
        for (int i = 0; i < size; i++) {
            data[i] = i % 50;
        }
    }
    printf("[COLD] Initialized %d elements\n", size);
}

COLD NOINLINE
void validation_check(int *data, int size) {
    int errors = 0;
    for (int i = 0; i < size && i < 10; i++) {
        if (data[i] < 0 || data[i] > 1000) {
            errors++;
        }
    }
    if (errors > 0) {
        printf("[COLD] Found %d potential errors\n", errors);
    }
}

/* ========== MODE-DEPENDENT FUNCTIONS ========== */

NOINLINE
void mode_specific_operation(int *data, int size) {
    int temp = 0;
    
    /* Execution path depends heavily on algorithm_mode */
    switch (algorithm_mode) {
        case 0: /* Bubble-sort like passes */
            for (int i = 0; i < size - 1; i++) {
                for (int j = 0; j < size - i - 1; j++) {
                    if (data[j] > data[j + 1]) {
                        temp = data[j];
                        data[j] = data[j + 1];
                        data[j + 1] = temp;
                    }
                }
            }
            break;
            
        case 1: /* Selection style */
            for (int i = 0; i < size; i++) {
                int min_idx = i;
                for (int j = i + 1; j < size; j++) {
                    if (data[j] < data[min_idx]) {
                        min_idx = j;
                    }
                }
                temp = data[min_idx];
                data[min_idx] = data[i];
                data[i] = temp;
            }
            break;
            
        case 2: /* Just shuffle */
            for (int i = 0; i < size; i++) {
                int j = (i * 31 + global_seed) % size;
                temp = data[i];
                data[i] = data[j];
                data[j] = temp;
            }
            break;
            
        default:
            /* Linear scan with branches */
            for (int i = 0; i < size; i++) {
                if (data[i] % 2 == 0) {
                    data[i] *= 2;
                } else if (data[i] % 3 == 0) {
                    data[i] /= 3;
                }
            }
            break;
    }
}

/* ========== DEAD CODE (for different build variants) ========== */

#ifdef VARIANT_A
NOINLINE
void variant_a_specific(int *data, int size) {
    printf("Variant A specific code\n");
    for (int i = 0; i < size; i++) {
        data[i] = (data[i] << 2) | 0x03;
    }
}
#endif

#ifdef VARIANT_B  
NOINLINE
void variant_b_specific(int *data, int size) {
    printf("Variant B specific code\n");
    for (int i = 0; i < size; i++) {
        data[i] = (data[i] >> 2) & 0x3F;
    }
}
#endif

/* ========== MAIN DRIVER ========== */

int main(int argc, char *argv[]) {
    /* Parse command-line arguments for variability */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            global_seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) {
            iteration_multiplier = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--algorithm") == 0 && i + 1 < argc) {
            algorithm_mode = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [--seed N] [--iterations M] [--algorithm 0|1|2]\n", argv[0]);
            return 0;
        }
    }
    
    srand(global_seed);
    printf("Starting with seed=%d, iterations=%d, mode=%d\n",
           global_seed, iteration_multiplier, algorithm_mode);
    
    /* Create data arrays */
    const int data_size = 100;
    int *data1 = (int*)malloc(data_size * sizeof(int));
    int *data2 = (int*)malloc(data_size * sizeof(int));
    
    if (!data1 || !data2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Execute functions in varying order based on mode */
    cold_initializer(data1, data_size);
    cold_initializer(data2, data_size / 2);
    
    /* Hot functions - many iterations */
    hot_loop_processor(data1, data_size);
    matrix_multiply_sim(50, 40);
    
    /* Mode-dependent execution */
    mode_specific_operation(data1, data_size);
    
    /* External object file functions */
    process_data_hot(data1, data_size, 50);  /* Defined in another file */
    process_data_cold(data2, data_size / 2); /* Defined in another file */
    
    /* Conditional compilation blocks */
    #ifdef VARIANT_A
    variant_a_specific(data1, data_size);
    #endif
    
    #ifdef VARIANT_B
    variant_b_specific(data2, data_size / 2);
    #endif
    
    /* Validation */
    validation_check(data1, data_size);
    validation_check(data2, data_size / 2);
    
    /* Compute checksum for verification */
    unsigned long checksum = 0;
    for (int i = 0; i < data_size; i++) {
        checksum = (checksum * 31 + data1[i]) % 1000000007;
    }
    for (int i = 0; i < data_size / 2; i++) {
        checksum = (checksum * 17 + data2[i]) % 1000000007;
    }
    
    printf("Final checksum: %lu\n", checksum);
    
    free(data1);
    free(data2);
    
    return 0;
}
