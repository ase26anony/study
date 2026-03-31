/* gcov_tool_test.c - Program to generate varied GCOV profiles for overlap analysis */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Compiler directives to control instrumentation */
#define HOT_LOOP_COUNT 10000
#define COLD_LOOP_COUNT 10

/* Function attributes to preserve structure */
#define NOINLINE __attribute__((noinline))
#define HOT __attribute__((hot))
#define COLD __attribute__((cold))

/* Different source files would normally have these in separate .c files */
/* For demonstration, we use static functions to simulate different compilation units */

/* ========== Object File 1 Simulation ========== */
static int obj1_compare_asc(const void *a, const void *b) NOINLINE COLD {
    int ia = *(const int*)a;
    int ib = *(const int*)b;
    if (ia < ib) return -1;
    if (ia > ib) return 1;
    return 0;
}

static int obj1_compare_desc(const void *a, const void *b) NOINLINE COLD {
    int ia = *(const int*)a;
    int ib = *(const int*)b;
    if (ia > ib) return -1;
    if (ia < ib) return 1;
    return 0;
}

static void obj1_bubble_sort(int *arr, int n, int ascending) NOINLINE {
    int i, j, temp;
    for (i = 0; i < n-1; i++) {
        for (j = 0; j < n-i-1; j++) {
            int should_swap = 0;
            if (ascending) {
                should_swap = (arr[j] > arr[j+1]);
            } else {
                should_swap = (arr[j] < arr[j+1]);
            }
            if (should_swap) {
                temp = arr[j];
                arr[j] = arr[j+1];
                arr[j+1] = temp;
            }
        }
    }
}

/* ========== Object File 2 Simulation ========== */
static int obj2_find_median(int *arr, int n) NOINLINE {
    if (n <= 0) return 0;
    
    int *copy = malloc(n * sizeof(int));
    if (!copy) return 0;
    
    memcpy(copy, arr, n * sizeof(int));
    qsort(copy, n, sizeof(int), obj1_compare_asc);
    
    int median = copy[n/2];
    free(copy);
    return median;
}

static long obj2_calculate_sum(int *arr, int n) NOINLINE HOT {
    long sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
        /* Hot loop with many iterations */
        for (int j = 0; j < HOT_LOOP_COUNT/1000; j++) {
            sum += (sum % 1000);
        }
    }
    return sum;
}

/* ========== Object File 3 Simulation ========== */
static void obj3_process_array(int *arr, int n, int mode) NOINLINE {
    switch (mode) {
        case 0: /* Square elements */
            for (int i = 0; i < n; i++) {
                arr[i] = arr[i] * arr[i];
            }
            break;
        case 1: /* Add index */
            for (int i = 0; i < n; i++) {
                arr[i] += i;
            }
            break;
        case 2: /* Modulo operation */
            for (int i = 0; i < n; i++) {
                arr[i] = arr[i] % 100;
            }
            break;
        default:
            for (int i = 0; i < n; i++) {
                arr[i] = -arr[i];
            }
    }
}

static int obj3_count_evens(int *arr, int n) NOINLINE COLD {
    int count = 0;
    for (int i = 0; i < n; i++) {
        if (arr[i] % 2 == 0) {
            count++;
        }
    }
    return count;
}

/* ========== Main Program Functions ========== */
static void generate_random_array(int *arr, int n, int seed) NOINLINE {
    srand(seed);
    for (int i = 0; i < n; i++) {
        arr[i] = rand() % 1000;
    }
}

static void generate_sorted_array(int *arr, int n, int ascending) NOINLINE {
    for (int i = 0; i < n; i++) {
        arr[i] = ascending ? i : n - i;
    }
}

static void process_hot_path(int *arr, int n) NOINLINE HOT {
    /* This function should have high execution counts */
    long checksum = 0;
    for (int iter = 0; iter < HOT_LOOP_COUNT; iter++) {
        for (int i = 0; i < n; i++) {
            checksum += arr[i] * iter;
            if (checksum % 2 == 0) {
                arr[i] += 1;
            } else {
                arr[i] -= 1;
            }
        }
    }
    /* Prevent dead code elimination */
    if (checksum == 0) printf("Impossible\n");
}

static void process_cold_path(int *arr, int n) NOINLINE COLD {
    /* This function should have low execution counts */
    if (n < 2) return;
    
    int temp = arr[0];
    arr[0] = arr[n-1];
    arr[n-1] = temp;
    
    /* Very short loop */
    for (int i = 0; i < COLD_LOOP_COUNT; i++) {
        arr[i % n] += i;
    }
}

static void analyze_array(int *arr, int n, int analysis_mode) NOINLINE {
    /* Complex function with many branches for coverage */
    long sum = obj2_calculate_sum(arr, n);
    int median = obj2_find_median(arr, n);
    int evens = obj3_count_evens(arr, n);
    
    if (analysis_mode == 0) {
        printf("Basic analysis: sum=%ld, median=%d, evens=%d\n", 
               sum, median, evens);
    } else if (analysis_mode == 1) {
        int threshold = median;
        int above = 0, below = 0;
        for (int i = 0; i < n; i++) {
            if (arr[i] > threshold) above++;
            else if (arr[i] < threshold) below++;
        }
        printf("Threshold analysis: above=%d, below=%d, equal=%d\n",
               above, below, n - above - below);
    } else {
        /* Complex nested conditions */
        int max_val = arr[0];
        int min_val = arr[0];
        for (int i = 1; i < n; i++) {
            if (arr[i] > max_val) {
                max_val = arr[i];
                if (max_val > 900) {
                    for (int j = 0; j < 5; j++) {
                        arr[i] -= j;
                    }
                }
            } else if (arr[i] < min_val) {
                min_val = arr[i];
                if (min_val < 100) {
                    arr[i] += 50;
                }
            }
        }
        printf("Range analysis: min=%d, max=%d, range=%d\n",
               min_val, max_val, max_val - min_val);
    }
}

/* Dead code that might be compiled out in some builds */
#ifdef USE_DEAD_CODE
static void dead_code_function(void) NOINLINE {
    printf("This function is never called\n");
    int x = 0;
    for (int i = 0; i < 100; i++) {
        x += i * i;
        if (x > 1000) break;
    }
}
#endif

int main(int argc, char *argv[]) {
    int array_size = 100;
    int seed = 42;
    int mode = 0;
    int iterations = 1;
    int use_hot_path = 1;
    int analysis_mode = 0;
    
    /* Parse command line arguments for variability */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--seed") == 0 && i+1 < argc) {
            seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--size") == 0 && i+1 < argc) {
            array_size = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--mode") == 0 && i+1 < argc) {
            mode = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--iterations") == 0 && i+1 < argc) {
            iterations = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--analysis") == 0 && i+1 < argc) {
            analysis_mode = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--no-hot") == 0) {
            use_hot_path = 0;
        } else if (strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [--seed N] [--size N] [--mode 0|1|2] "
                   "[--iterations N] [--analysis 0|1|2] [--no-hot]\n", argv[0]);
            return 0;
        }
    }
    
    if (array_size <= 0) array_size = 100;
    if (iterations <= 0) iterations = 1;
    
    int *array = malloc(array_size * sizeof(int));
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    long total_checksum = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Generate different array based on mode */
        if (mode == 0) {
            generate_random_array(array, array_size, seed + iter);
        } else if (mode == 1) {
            generate_sorted_array(array, array_size, 1);
        } else {
            generate_sorted_array(array, array_size, 0);
        }
        
        /* Process array with different algorithms */
        if (use_hot_path) {
            process_hot_path(array, array_size);
        } else {
            process_cold_path(array, array_size);
        }
        
        /* Sort based on iteration parity */
        if (iter % 2 == 0) {
            obj1_bubble_sort(array, array_size, 1);
        } else {
            qsort(array, array_size, sizeof(int), obj1_compare_desc);
        }
        
        /* Additional processing */
        obj3_process_array(array, array_size, iter % 3);
        
        /* Analysis with different modes */
        analyze_array(array, array_size, analysis_mode);
        
        /* Calculate checksum for output (prevent dead code elimination) */
        for (int i = 0; i < array_size; i++) {
            total_checksum += array[i];
            total_checksum = total_checksum % 1000000;
        }
    }
    
    printf("Final checksum: %ld\n", total_checksum);
    
    free(array);
    return 0;
}
