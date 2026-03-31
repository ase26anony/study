/* gcov_tool_test.c - Multi-run profile generator for gcov-tool overlap analysis */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Function prototypes targeting specific gcov-tool options */
__attribute__((noinline)) static void cold_function_1(void);
__attribute__((noinline)) static void cold_function_2(void);
__attribute__((hot)) void hot_loop_function(int iterations);
__attribute__((noinline)) void medium_work_function(int mode);
__attribute__((noinline)) int data_processor(int *data, int size, int threshold);
__attribute__((noinline)) void branchy_function(int seed, int *counter);
__attribute__((cold)) void rarely_called(void);

/* Global counters for varied execution profiles */
static int global_counter = 0;
static int call_sequence[100];
static int seq_index = 0;

/* Different namespaces via static functions in same file */
static void helper_a(void) { global_counter += 1; }
static void helper_b(void) { global_counter += 2; }

/* COLD: Will run only in specific modes */
__attribute__((cold)) 
static void cold_function_1(void) {
    if (global_counter % 1000 == 0) {
        printf("[COLD1] Rare condition met\n");
    }
    /* Complex branching */
    for (int i = 0; i < 3; i++) {
        if (i % 2 == 0) {
            helper_a();
        } else {
            helper_b();
        }
    }
}

__attribute__((cold))
static void cold_function_2(void) {
    /* Dead code path that might be taken with specific seeds */
    if (global_counter > 10000) {
        printf("[COLD2] High counter path\n");
        for (int j = 0; j < 5; j++) {
            global_counter += j;
        }
    } else {
        global_counter -= 1;
    }
}

/* HOT: High iteration loops for -h and -t options */
__attribute__((hot))
void hot_loop_function(int iterations) {
    int local_sum = 0;
    /* Prevent aggressive unrolling */
    #pragma GCC unroll 0
    for (int i = 0; i < iterations; i++) {
        /* Variable branching inside hot loop */
        if (i % 37 == 0) {
            local_sum += i * 2;
        } else if (i % 13 == 0) {
            local_sum += i / 2;
        } else {
            local_sum += i;
        }
        
        /* Nested condition for deeper analysis */
        if (i % 100 == 0) {
            for (int j = 0; j < 10; j++) {
                local_sum += (j % 3 == 0) ? j : -j;
            }
        }
    }
    global_counter += local_sum % 1000;
    call_sequence[seq_index++ % 100] = 1;
}

/* Medium workload with mode-dependent behavior */
__attribute__((noinline))
void medium_work_function(int mode) {
    int arr[50];
    int result = 0;
    
    /* Initialize based on mode */
    for (int i = 0; i < 50; i++) {
        if (mode == 1) {
            arr[i] = i * 2 + (i % 7);
        } else if (mode == 2) {
            arr[i] = (i * 3) % 97;
        } else {
            arr[i] = i;
        }
    }
    
    /* Process based on mode */
    switch (mode) {
        case 1:
            for (int i = 0; i < 50; i++) {
                if (arr[i] > 25) result += arr[i];
            }
            break;
        case 2:
            for (int i = 49; i >= 0; i--) {
                if (arr[i] < 50) result -= arr[i];
            }
            break;
        case 3:
            for (int i = 0; i < 50; i += 2) {
                result += arr[i] * arr[i+1];
            }
            break;
        default:
            for (int i = 0; i < 50; i++) {
                result += arr[i];
            }
    }
    
    global_counter += result % 255;
    call_sequence[seq_index++ % 100] = 2;
}

/* Complex data processor with threshold */
__attribute__((noinline))
int data_processor(int *data, int size, int threshold) {
    int count = 0;
    int sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Multiple branching conditions */
        if (data[i] > threshold) {
            sum += data[i];
            count++;
            if (data[i] > threshold * 2) {
                sum += data[i] / 2;
                count++;
            }
        } else if (data[i] < -threshold) {
            sum -= data[i];
            count--;
        } else {
            /* Nested if-else */
            if (i % 3 == 0) {
                sum += 1;
            } else if (i % 3 == 1) {
                sum += 2;
            } else {
                sum += 3;
            }
        }
        
        /* Inner loop with condition */
        for (int j = 0; j < 3 && j < i; j++) {
            if (data[i] % (j+2) == 0) {
                sum += j;
            }
        }
    }
    
    call_sequence[seq_index++ % 100] = 3;
    return sum / (count ? count : 1);
}

/* Function with many branches based on seed */
__attribute__((noinline))
void branchy_function(int seed, int *counter) {
    int local = seed;
    
    /* Chain of if-else-if */
    if (local % 2 == 0) {
        *counter += 10;
        if (local % 4 == 0) {
            *counter += 5;
        }
    } else if (local % 3 == 0) {
        *counter += 7;
    } else if (local % 5 == 0) {
        *counter += 3;
    } else {
        *counter += 1;
    }
    
    /* Switch statement with fall-through */
    switch (local % 7) {
        case 0:
            *counter *= 2;
            break;
        case 1:
            *counter += 100;
            /* Fall through */
        case 2:
            *counter += 50;
            break;
        case 3:
        case 4:
            *counter += 25;
            break;
        default:
            *counter += 10;
    }
    
    call_sequence[seq_index++ % 100] = 4;
}

__attribute__((cold))
void rarely_called(void) {
    /* This should only be called in specific modes */
    printf("[RARE] Executing rarely called function\n");
    for (int i = 0; i < 10; i++) {
        global_counter += (i % 3 == 0) ? i : -i;
    }
    call_sequence[seq_index++ % 100] = 5;
}

/* Main function with configurable execution paths */
int main(int argc, char *argv[]) {
    int mode = 1;
    int seed = 42;
    int iterations = 1000;
    int threshold = 50;
    int use_cold = 0;
    
    /* Parse command line arguments for different profiles */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--mode") == 0 && i+1 < argc) {
            mode = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--seed") == 0 && i+1 < argc) {
            seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--iterations") == 0 && i+1 < argc) {
            iterations = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--threshold") == 0 && i+1 < argc) {
            threshold = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--use-cold") == 0) {
            use_cold = 1;
        }
    }
    
    /* Initialize random with seed for reproducible but varied profiles */
    srand(seed);
    
    /* Initialize data array with seed-dependent values */
    int data_size = 100 + (seed % 100);
    int *data = malloc(data_size * sizeof(int));
    for (int i = 0; i < data_size; i++) {
        data[i] = (rand() % 200) - 100;  /* Values between -100 and 100 */
    }
    
    /* Reset sequence tracker */
    seq_index = 0;
    global_counter = 0;
    
    /* Execute functions in different orders based on mode */
    if (mode == 1) {
        /* Mode 1: Hot-heavy profile */
        hot_loop_function(iterations * 10);
        medium_work_function(1);
        hot_loop_function(iterations / 2);
        data_processor(data, data_size, threshold);
        branchy_function(seed, &global_counter);
    } else if (mode == 2) {
        /* Mode 2: Balanced profile */
        medium_work_function(2);
        branchy_function(seed * 2, &global_counter);
        hot_loop_function(iterations);
        data_processor(data, data_size, threshold / 2);
        medium_work_function(3);
    } else if (mode == 3) {
        /* Mode 3: Cold-inclusive profile */
        if (use_cold) {
            cold_function_1();
            rarely_called();
        }
        hot_loop_function(iterations * 3);
        data_processor(data, data_size, threshold * 2);
        branchy_function(seed * 3, &global_counter);
        if (seed % 5 == 0) {
            cold_function_2();
        }
    } else {
        /* Default: Mixed profile */
        for (int i = 0; i < 3; i++) {
            hot_loop_function(iterations / (i+1));
            medium_work_function(i+1);
        }
        data_processor(data, data_size, threshold);
        branchy_function(seed, &global_counter);
    }
    
    /* Additional conditional execution */
    if (seed % 7 == 0) {
        /* Extra hot loop for some seeds */
        hot_loop_function(500);
    }
    
    if (threshold > 75) {
        /* Different processing for high thresholds */
        int extra_data[20];
        for (int i = 0; i < 20; i++) {
            extra_data[i] = rand() % 100;
        }
        data_processor(extra_data, 20, threshold - 50);
    }
    
    /* Calculate checksum to prevent dead code elimination */
    int checksum = global_counter;
    for (int i = 0; i < (seq_index < 100 ? seq_index : 100); i++) {
        checksum = (checksum * 31 + call_sequence[i]) % 1000000;
    }
    
    /* Output deterministic result */
    printf("Mode: %d, Seed: %d, Iterations: %d, Checksum: %d\n", 
           mode, seed, iterations, checksum);
    
    free(data);
    return 0;
}
