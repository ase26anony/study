/* gcov_tool_test.c - Generates varied GCOV profiles for overlap analysis */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Forward declarations for functions in different compilation units */
extern void process_data(int *arr, int n, int mode);
extern void analyze_results(int *arr, int n, int threshold);

/* Global configuration */
static int global_seed = 0;
static int iteration_count = 1000;
static int algorithm_mode = 0;
static int use_fullname_mode = 0;

/* Hot/Cold function attributes */
__attribute__((hot)) void hot_function_1(int iterations);
__attribute__((cold)) void cold_function_1(void);
__attribute__((noinline)) void critical_path_function(int mode);

/* ========== MODULE 1 FUNCTIONS ========== */

/* Function with varying execution paths based on mode */
__attribute__((noinline)) 
void process_data(int *arr, int n, int mode) {
    int i, j;
    long sum = 0;
    
    /* Hot loop - runs many times */
    for (i = 0; i < iteration_count; i++) {
        if (mode == 1) {
            /* Path A: Simple accumulation */
            for (j = 0; j < n; j++) {
                sum += arr[j];
                if (arr[j] > 1000) sum -= 500;  /* Branch with varying probability */
            }
        } else if (mode == 2) {
            /* Path B: More complex transformation */
            for (j = 0; j < n; j++) {
                arr[j] = (arr[j] * 3) / 2;
                if (arr[j] % 2 == 0) {
                    sum += arr[j];
                } else {
                    sum -= arr[j] / 2;
                }
            }
        } else {
            /* Path C: Random modifications */
            for (j = 0; j < n; j++) {
                int r = rand() % 100;
                if (r < 30) arr[j] += r;
                else if (r < 60) arr[j] -= r;
                else arr[j] *= 2;
                sum += arr[j];
            }
        }
        
        /* Nested conditional with varying probability */
        if (sum > 1000000) {
            sum /= 2;
        } else if (sum < -500000) {
            sum *= -1;
        }
    }
    
    /* Final processing */
    if (sum > 0) {
        printf("Positive sum: %ld\n", sum);
    } else {
        printf("Non-positive sum: %ld\n", sum);
    }
}

/* Hot function - runs many times */
__attribute__((hot)) 
void hot_function_1(int iterations) {
    int i;
    volatile int counter = 0;  /* volatile prevents optimization */
    
    for (i = 0; i < iterations * 10; i++) {
        counter += (i % 17) * 3;
        if (counter > 1000) {
            counter /= 2;
        }
        
        /* Inner loop with varying execution count */
        #pragma GCC unroll 0  /* Prevent unrolling */
        for (int j = 0; j < (i % 5); j++) {
            counter += j * 2;
        }
    }
}

/* Cold function - rarely called */
__attribute__((cold))
void cold_function_1(void) {
    static int call_count = 0;
    call_count++;
    
    /* Complex switch with many cases */
    switch (call_count % 7) {
        case 0: printf("Cold: State A\n"); break;
        case 1: printf("Cold: State B\n"); break;
        case 2: printf("Cold: State C\n"); break;
        case 3: printf("Cold: State D\n"); break;
        case 4: printf("Cold: State E\n"); break;
        case 5: printf("Cold: State F\n"); break;
        case 6: printf("Cold: State G\n"); break;
    }
}

/* ========== MODULE 2 FUNCTIONS ========== */

/* Another processing function with different characteristics */
__attribute__((noinline))
void analyze_results(int *arr, int n, int threshold) {
    int hot_count = 0, cold_count = 0;
    int i;
    
    for (i = 0; i < n; i++) {
        if (arr[i] > threshold) {
            hot_count++;
            /* Hot path processing */
            arr[i] = arr[i] * 0.8;
        } else {
            cold_count++;
            /* Cold path processing */
            arr[i] = arr[i] * 1.2;
        }
        
        /* Nested conditionals */
        if (i % 3 == 0) {
            if (arr[i] % 2 == 0) {
                arr[i] += 1;
            } else {
                arr[i] -= 1;
            }
        }
    }
    
    printf("Hot elements: %d, Cold elements: %d\n", hot_count, cold_count);
    
    /* Threshold-based branching */
    if (hot_count > cold_count * 2) {
        printf("Very hot dataset\n");
    } else if (hot_count > cold_count) {
        printf("Moderately hot dataset\n");
    } else {
        printf("Cold dataset\n");
    }
}

/* Function with name collision (for -F fullname testing) */
static void helper_function(int x) {
    /* Different implementation than in other file */
    int result = x * x + 2 * x + 1;
    if (result > 100) {
        result /= 2;
    }
}

/* ========== MAIN PROGRAM ========== */

/* Critical path function with complex branching */
__attribute__((noinline))
void critical_path_function(int mode) {
    int i;
    int local_data[50];
    
    /* Initialize with pattern based on mode */
    for (i = 0; i < 50; i++) {
        local_data[i] = (i * mode + global_seed) % 100;
    }
    
    /* Mode-dependent processing */
    switch (mode % 4) {
        case 0:
            /* Bubble sort variant */
            for (i = 0; i < 49; i++) {
                for (int j = 0; j < 49 - i; j++) {
                    if (local_data[j] > local_data[j + 1]) {
                        int temp = local_data[j];
                        local_data[j] = local_data[j + 1];
                        local_data[j + 1] = temp;
                    }
                }
            }
            break;
            
        case 1:
            /* Search with early exit */
            int target = 50;
            for (i = 0; i < 50; i++) {
                if (local_data[i] == target) {
                    break;
                }
            }
            break;
            
        case 2:
            /* Mathematical transformations */
            for (i = 0; i < 50; i++) {
                local_data[i] = local_data[i] * local_data[i];
                if (local_data[i] > 1000) {
                    local_data[i] = local_data[i] % 1000;
                }
            }
            break;
            
        case 3:
            /* Random walk */
            for (i = 1; i < 50; i++) {
                int step = rand() % 3 - 1;
                local_data[i] = local_data[i - 1] + step;
            }
            break;
    }
    
    /* Call helper */
    helper_function(local_data[0]);
}

/* Dead code for different build variants */
#ifdef VARIANT_A
void variant_a_specific(void) {
    printf("Variant A specific code\n");
    /* Complex calculations only in variant A */
    for (int i = 0; i < 100; i++) {
        volatile int x = i * i;
        (void)x;
    }
}
#else
void variant_b_specific(void) {
    printf("Variant B specific code\n");
    /* Different calculations for variant B */
    for (int i = 0; i < 50; i++) {
        volatile int x = i * 2;
        (void)x;
    }
}
#endif

int main(int argc, char *argv[]) {
    int data[1000];
    int i, mode = 0;
    
    /* Parse command line arguments */
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            global_seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) {
            iteration_count = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--algorithm") == 0 && i + 1 < argc) {
            algorithm_mode = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--fullname") == 0) {
            use_fullname_mode = 1;
        } else if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc) {
            mode = atoi(argv[++i]);
        }
    }
    
    /* Seed RNG for reproducible but varied profiles */
    if (global_seed == 0) {
        global_seed = time(NULL);
    }
    srand(global_seed);
    
    printf("Running with seed=%d, iterations=%d, algorithm=%d, mode=%d\n",
           global_seed, iteration_count, algorithm_mode, mode);
    
    /* Initialize data with seed-dependent values */
    for (i = 0; i < 1000; i++) {
        data[i] = rand() % 2000;
    }
    
    /* Execute different code paths based on mode */
    if (mode == 0) {
        /* Path 0: All functions, balanced */
        process_data(data, 1000, algorithm_mode);
        hot_function_1(iteration_count / 10);
        analyze_results(data, 1000, 1000);
        critical_path_function(algorithm_mode);
        cold_function_1();
    } else if (mode == 1) {
        /* Path 1: Heavy on hot functions */
        for (int j = 0; j < 5; j++) {
            hot_function_1(iteration_count / 5);
            process_data(data, 500, 1);
        }
        analyze_results(data, 1000, 500);
    } else if (mode == 2) {
        /* Path 2: Mix with many cold calls */
        process_data(data, 1000, 2);
        for (int j = 0; j < 20; j++) {
            cold_function_1();
        }
        critical_path_function(2);
    } else if (mode == 3) {
        /* Path 3: Algorithm-intensive */
        for (int j = 0; j < 3; j++) {
            critical_path_function(j);
            process_data(data, 800, 3);
        }
        hot_function_1(iteration_count / 20);
    }
    
    /* Build variant specific code */
    #ifdef VARIANT_A
    variant_a_specific();
    #else
    variant_b_specific();
    #endif
    
    /* Calculate checksum to prevent dead code elimination */
    long checksum = 0;
    for (i = 0; i < 1000; i++) {
        checksum += data[i];
        checksum ^= (data[i] << (i % 16));
    }
    
    printf("Final checksum: %ld\n", checksum);
    return 0;
}
