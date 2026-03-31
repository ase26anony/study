/* gcov_tool_test.c - Test program for gcov-tool overlap analysis */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Compiler directives to preserve function boundaries */
#define NOINLINE __attribute__((noinline))
#define HOT __attribute__((hot))
#define COLD __attribute__((cold))

/* Global configuration */
static int g_verbose = 0;
static int g_iterations = 1000;
static int g_seed = 42;
static int g_algorithm = 0;
static int g_data_size = 1000;

/* Forward declarations */
NOINLINE static void bubble_sort(int *arr, int n);
NOINLINE static void insertion_sort(int *arr, int n);
NOINLINE static void selection_sort(int *arr, int n);
NOINLINE COLD void slow_algorithm(int *arr, int n);
NOINLINE HOT void fast_algorithm(int *arr, int n);
NOINLINE static void process_data(int *arr, int n, int mode);
NOINLINE static int validate_sorted(int *arr, int n);
NOINLINE static unsigned long compute_checksum(int *arr, int n);

/* Different compilation units would have these in separate files */
NOINLINE static void helper_swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

/* Function with same name but different scope (simulating different files) */
NOINLINE static void log_message(const char *msg) {
    if (g_verbose) {
        printf("LOG: %s\n", msg);
    }
}

/* Another log_message in "different compilation unit" */
NOINLINE static void log_message_debug(const char *msg) {
    /* Empty in this variant, could be enabled with compile flag */
    (void)msg;
}

/* Cold function - rarely called */
NOINLINE COLD void slow_algorithm(int *arr, int n) {
    log_message("Entering slow algorithm");
    
    /* Inefficient O(n^3) algorithm */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            for (int k = 0; k < n; k++) {
                if (arr[i] > arr[j]) {
                    helper_swap(&arr[i], &arr[j]);
                }
            }
        }
    }
    
    log_message("Exiting slow algorithm");
}

/* Hot function - called many times */
NOINLINE HOT void fast_algorithm(int *arr, int n) {
    /* Quick sort implementation */
    if (n < 2) return;
    
    int pivot = arr[n / 2];
    int *left = arr;
    int *right = arr + n - 1;
    
    while (left <= right) {
        while (*left < pivot) left++;
        while (*right > pivot) right--;
        if (left <= right) {
            helper_swap(left, right);
            left++;
            right--;
        }
    }
    
    fast_algorithm(arr, right - arr + 1);
    fast_algorithm(left, arr + n - left);
}

NOINLINE static void bubble_sort(int *arr, int n) {
    log_message("Starting bubble sort");
    for (int i = 0; i < n - 1; i++) {
        int swapped = 0;
        for (int j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                helper_swap(&arr[j], &arr[j + 1]);
                swapped = 1;
            }
        }
        if (!swapped) break;
    }
}

NOINLINE static void insertion_sort(int *arr, int n) {
    log_message("Starting insertion sort");
    for (int i = 1; i < n; i++) {
        int key = arr[i];
        int j = i - 1;
        
        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

NOINLINE static void selection_sort(int *arr, int n) {
    log_message("Starting selection sort");
    for (int i = 0; i < n - 1; i++) {
        int min_idx = i;
        for (int j = i + 1; j < n; j++) {
            if (arr[j] < arr[min_idx]) {
                min_idx = j;
            }
        }
        if (min_idx != i) {
            helper_swap(&arr[i], &arr[min_idx]);
        }
    }
}

NOINLINE static void process_data(int *arr, int n, int mode) {
    /* Varied execution paths based on mode */
    switch (mode) {
        case 0:
            bubble_sort(arr, n);
            break;
        case 1:
            insertion_sort(arr, n);
            break;
        case 2:
            selection_sort(arr, n);
            break;
        case 3:
            fast_algorithm(arr, n);
            break;
        case 4:
            slow_algorithm(arr, n);
            break;
        default:
            /* Mixed strategy */
            if (n > 500) {
                fast_algorithm(arr, n);
            } else {
                insertion_sort(arr, n);
            }
            break;
    }
}

NOINLINE static int validate_sorted(int *arr, int n) {
    for (int i = 1; i < n; i++) {
        if (arr[i] < arr[i - 1]) {
            return 0;
        }
    }
    return 1;
}

NOINLINE static unsigned long compute_checksum(int *arr, int n) {
    unsigned long sum = 0;
    for (int i = 0; i < n; i++) {
        sum = (sum * 31 + arr[i]) % 1000000007;
    }
    return sum;
}

/* Dead code that can be enabled via compile-time flag */
#ifdef ENABLE_EXTRA_FEATURES
NOINLINE static void extra_feature(int *arr, int n) {
    /* This function only exists in some compilation variants */
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] ^ 0x55;
    }
}
#endif

int main(int argc, char *argv[]) {
    /* Parse command line arguments to create varied profiles */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            g_seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) {
            g_iterations = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--algorithm") == 0 && i + 1 < argc) {
            g_algorithm = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--size") == 0 && i + 1 < argc) {
            g_data_size = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--verbose") == 0) {
            g_verbose = 1;
        } else if (strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [--seed N] [--iterations M] [--algorithm A] [--size S] [--verbose]\n", argv[0]);
            return 0;
        }
    }
    
    /* Initialize random number generator with seed */
    srand(g_seed);
    
    /* Allocate and initialize data */
    int *data = (int *)malloc(g_data_size * sizeof(int));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Generate random data */
    for (int i = 0; i < g_data_size; i++) {
        data[i] = rand() % 10000;
    }
    
    /* Main processing loop - creates hot/cold regions */
    unsigned long total_checksum = 0;
    
    for (int iter = 0; iter < g_iterations; iter++) {
        /* Make a copy of data for this iteration */
        int *working_data = (int *)malloc(g_data_size * sizeof(int));
        memcpy(working_data, data, g_data_size * sizeof(int));
        
        /* Occasionally use a different algorithm */
        int mode = g_algorithm;
        if (iter % 100 == 0) {
            mode = (mode + 1) % 5;  /* Cycle through algorithms */
        }
        
        /* Process the data */
        process_data(working_data, g_data_size, mode);
        
        /* Validate and compute checksum */
        if (validate_sorted(working_data, g_data_size)) {
            total_checksum += compute_checksum(working_data, g_data_size);
        } else {
            fprintf(stderr, "Sort failed at iteration %d\n", iter);
        }
        
        free(working_data);
        
        /* Occasionally call cold function */
        if (iter % 1000 == 999) {
            int *temp = (int *)malloc(10 * sizeof(int));
            for (int i = 0; i < 10; i++) temp[i] = rand() % 100;
            slow_algorithm(temp, 10);
            free(temp);
        }
    }
    
    /* Output deterministic result */
    printf("Checksum: %lu\n", total_checksum % 1000000007);
    
    free(data);
    return 0;
}
