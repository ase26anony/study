/* gcov_tool_test.c - Multi-run profile generator for gcov-tool testing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Compiler directives to preserve structure */
#define HOT_LOOP_ITERATIONS 10000
#define COLD_LOOP_ITERATIONS 5

/* Function attributes to control inlining and optimization */
__attribute__((noinline)) static void cold_function_alpha(void);
__attribute__((noinline, hot)) void hot_search_algorithm(int *data, int size, int target, int mode);
__attribute__((noinline)) static int helper_compute_hash(int *data, int size);
__attribute__((noinline, cold)) void rarely_called_validator(int *data, int size);

/* Different namespaces via static functions in same file */
static void process_data_a(int *arr, int n, int mode);
static void process_data_b(int *arr, int n, int mode);

/* Global to track execution patterns */
static int execution_pattern = 0;

/* ========== FUNCTION DEFINITIONS ========== */

/* Cold function - rarely executed */
__attribute__((cold)) 
void rarely_called_validator(int *data, int size) {
    if (size <= 0) return;
    
    int sum = 0;
    for (int i = 0; i < size; i++) {
        if (data[i] < 0) {
            sum -= data[i];
        } else {
            sum += data[i];
        }
        
        /* Branch with low probability */
        if (data[i] == 0xDEADBEEF) {  /* Extremely unlikely */
            printf("Impossible value found!\n");
        }
    }
    
    /* Dead code for coverage */
    #ifdef VALIDATOR_EXTRA
    printf("Extra validation mode\n");
    #endif
}

/* Hot function with complex branching */
__attribute__((hot))
void hot_search_algorithm(int *data, int size, int target, int mode) {
    int found = 0;
    int comparisons = 0;
    
    /* Different search algorithms based on mode */
    switch (mode % 3) {
        case 0:  /* Linear search */
            #pragma GCC unroll 0
            for (int i = 0; i < size; i++) {
                comparisons++;
                if (data[i] == target) {
                    found = 1;
                    break;
                }
                /* Nested condition */
                if (i > size/2 && !found) {
                    comparisons += 2;  /* Extra penalty */
                }
            }
            break;
            
        case 1:  /* Binary search (assumes sorted) */
            {
                int left = 0, right = size - 1;
                while (left <= right) {
                    comparisons++;
                    int mid = left + (right - left) / 2;
                    
                    if (data[mid] == target) {
                        found = 1;
                        break;
                    } else if (data[mid] < target) {
                        left = mid + 1;
                    } else {
                        right = mid - 1;
                    }
                    
                    /* Additional branch */
                    if (comparisons > size/2) {
                        comparisons++;  /* Extra count */
                    }
                }
            }
            break;
            
        case 2:  /* Jump search */
            {
                int step = (int)sqrt(size);
                int prev = 0;
                while (data[(step < size ? step : size) - 1] < target) {
                    comparisons++;
                    prev = step;
                    step += (int)sqrt(size);
                    if (step >= size) {
                        step = size;
                        break;
                    }
                }
                while (data[prev] < target) {
                    comparisons++;
                    prev++;
                    if (prev == (step < size ? step : size)) {
                        break;
                    }
                }
                if (data[prev] == target) {
                    found = 1;
                }
            }
            break;
    }
    
    /* Record execution pattern */
    execution_pattern = (execution_pattern << 2) | (mode % 3);
}

/* Static function with same name in this file */
static void process_data_a(int *arr, int n, int mode) {
    int temp;
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (mode == 1) {
                /* Ascending */
                if (arr[j] > arr[j + 1]) {
                    temp = arr[j];
                    arr[j] = arr[j + 1];
                    arr[j + 1] = temp;
                }
            } else {
                /* Descending */
                if (arr[j] < arr[j + 1]) {
                    temp = arr[j];
                    arr[j] = arr[j + 1];
                    arr[j + 1] = temp;
                }
            }
        }
    }
}

/* Another static function with same name (for -F testing) */
static void process_data_b(int *arr, int n, int mode) {
    /* Quick sort style partitioning */
    if (n <= 1) return;
    
    int pivot = arr[n / 2];
    int i = 0, j = n - 1;
    
    while (i <= j) {
        if (mode == 1) {
            while (arr[i] < pivot) i++;
            while (arr[j] > pivot) j--;
        } else {
            while (arr[i] > pivot) i++;
            while (arr[j] < pivot) j--;
        }
        
        if (i <= j) {
            int temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
            i++;
            j--;
        }
    }
    
    /* Recursive calls with different probabilities */
    if (mode == 1 && j > 0) {
        process_data_b(arr, j + 1, mode);
    }
    if (mode == 2 && n - i > 0) {
        process_data_b(arr + i, n - i, mode);
    }
}

__attribute__((noinline))
static int helper_compute_hash(int *data, int size) {
    unsigned int hash = 0;
    for (int i = 0; i < size; i++) {
        hash = (hash << 4) ^ (hash >> 28) ^ (unsigned int)data[i];
        
        /* Branch with 50% probability */
        if (hash & 0x1000) {
            hash ^= 0xABCD;
        } else {
            hash ^= 0x1234;
        }
    }
    return (int)(hash & 0x7FFFFFFF);
}

static void cold_function_alpha(void) {
    /* This function should have very low counts */
    static int call_count = 0;
    call_count++;
    
    /* Complex but rarely executed branching */
    if (call_count % 100 == 0) {
        printf("Alpha milestone: %d\n", call_count);
    }
    
    /* Nested conditions */
    for (int i = 0; i < COLD_LOOP_ITERATIONS; i++) {
        if (i % 2 == 0) {
            if (i % 3 == 0) {
                call_count += 0;  /* No-op for coverage */
            }
        }
    }
}

/* ========== SECOND OBJECT FILE FUNCTIONS ========== */
/* These would be in a separate file for multi-object testing */

/* In file2.c:
__attribute__((noinline)) 
void data_transform(int *arr, int size, int mode) {
    for (int i = 0; i < size; i++) {
        if (mode == 1) {
            arr[i] = arr[i] * 2 + 1;
        } else if (mode == 2) {
            arr[i] = (arr[i] << 1) ^ 0x5A5A;
        } else {
            arr[i] = ~arr[i];
        }
    }
}

__attribute__((noinline, hot))
void hot_loop_operation(int *arr, int size, int iterations) {
    for (int iter = 0; iter < iterations; iter++) {
        for (int i = 0; i < size; i++) {
            arr[i] = (arr[i] + iter) % 1000;
            if (arr[i] < 500) {
                arr[i] += size;
            } else {
                arr[i] -= iter;
            }
        }
    }
}
*/

/* ========== MAIN PROGRAM ========== */

int main(int argc, char *argv[]) {
    int mode = 0;
    int seed = 0;
    int data_size = 1000;
    int hot_iterations = HOT_LOOP_ITERATIONS;
    
    /* Parse command line arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc) {
            mode = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--size") == 0 && i + 1 < argc) {
            data_size = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) {
            hot_iterations = atoi(argv[++i]);
        }
    }
    
    /* Initialize random seed */
    if (seed == 0) {
        seed = (int)time(NULL);
    }
    srand(seed);
    
    /* Allocate and initialize data */
    int *data = (int *)malloc(data_size * sizeof(int));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    for (int i = 0; i < data_size; i++) {
        data[i] = rand() % 10000;
    }
    
    /* Execute different code paths based on mode */
    switch (mode % 4) {
        case 0:
            /* Path A: Mostly hot functions */
            hot_search_algorithm(data, data_size, rand() % 10000, mode);
            process_data_a(data, data_size, 1);
            break;
            
        case 1:
            /* Path B: Mix of hot and cold */
            hot_search_algorithm(data, data_size / 2, rand() % 10000, mode);
            cold_function_alpha();
            process_data_b(data, data_size, 2);
            break;
            
        case 2:
            /* Path C: Different hot function pattern */
            for (int i = 0; i < hot_iterations / 100; i++) {
                hot_search_algorithm(data, 100, rand() % 10000, mode);
            }
            rarely_called_validator(data, 10);
            break;
            
        case 3:
            /* Path D: All functions */
            hot_search_algorithm(data, data_size, rand() % 10000, mode);
            cold_function_alpha();
            process_data_a(data, data_size, 1);
            process_data_b(data, data_size, 2);
            rarely_called_validator(data, data_size);
            break;
    }
    
    /* Compute final hash for output */
    int final_hash = helper_compute_hash(data, data_size);
    
    /* Additional hot loop based on mode */
    if (mode % 2 == 0) {
        for (int i = 0; i < hot_iterations; i++) {
            for (int j = 0; j < data_size / 100; j++) {
                data[j] = (data[j] + i) % 1000;
            }
        }
    }
    
    /* Output deterministic result */
    printf("Result: mode=%d, seed=%d, hash=%08x, pattern=%08x\n", 
           mode, seed, final_hash, execution_pattern);
    
    free(data);
    return 0;
}
