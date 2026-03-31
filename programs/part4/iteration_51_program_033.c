/* test_doloop.c - Test program for GCC doloop optimization coverage */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define HOT_LOOP_COUNT 1000
#define ARRAY_SIZE 10000

/* Mark function as hot to encourage loop optimization */
__attribute__((hot))
static void process_data_downward_for(int *arr, int len) {
    /* Pattern 1: Basic decrementing for loop with > condition */
    for (int i = len - 1; i > 0; i--) {
        arr[i] = arr[i] * 3 + 7;
    }
    /* Handle element 0 separately to avoid i >= 0 comparison */
    if (len > 0) {
        arr[0] = arr[0] * 3 + 7;
    }
}

__attribute__((hot))
static void process_data_downward_for_ne(int *arr, int len) {
    /* Pattern 2: Decrementing for loop with != condition */
    for (int i = len; i != 0; i--) {
        arr[i - 1] = arr[i - 1] * 2 + 5;
    }
}

__attribute__((hot))
static void process_data_while_decrement(int *arr, int len) {
    /* Pattern 3: While loop that decrements counter */
    int n = len;
    while (n--) {
        arr[n] = arr[n] + n;
    }
}

__attribute__((hot))
static void process_data_nested_loops(int *arr, int len) {
    /* Pattern 4: Nested loops with inner decrementing counter */
    int outer_iters = 10;
    
    for (int outer = 0; outer < outer_iters; outer++) {
        /* Inner loop with decrementing counter */
        for (int i = len - 1; i > 0; i--) {
            arr[i] = (arr[i] + arr[i - 1]) / 2;
        }
    }
}

__attribute__((hot))
static void process_data_mixed_patterns(int *arr, int len) {
    /* Use all patterns in sequence */
    process_data_downward_for(arr, len);
    process_data_downward_for_ne(arr, len);
    process_data_while_decrement(arr, len);
    process_data_nested_loops(arr, len);
}

static unsigned long compute_checksum(int *arr, int len) {
    unsigned long sum = 0;
    for (int i = 0; i < len; i++) {
        sum += (unsigned long)arr[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    int *data;
    int size = ARRAY_SIZE;
    unsigned long total_checksum = 0;
    
    /* Use command line argument for size if provided */
    if (argc > 1) {
        size = atoi(argv[1]);
        if (size <= 0) size = ARRAY_SIZE;
    }
    
    /* Allocate and initialize array */
    data = (int *)malloc(size * sizeof(int));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        data[i] = rand() % 100;
    }
    
    /* Hot loop: call processing functions many times */
    for (int iter = 0; iter < HOT_LOOP_COUNT; iter++) {
        /* Alternate between different patterns to exercise various paths */
        if (iter % 4 == 0) {
            process_data_downward_for(data, size);
        } else if (iter % 4 == 1) {
            process_data_downward_for_ne(data, size);
        } else if (iter % 4 == 2) {
            process_data_while_decrement(data, size);
        } else {
            process_data_mixed_patterns(data, size);
        }
        
        /* Prevent compiler from optimizing away loops */
        total_checksum += compute_checksum(data, size);
    }
    
    printf("Final checksum: %lu\n", total_checksum);
    
    /* Verify data was actually modified */
    int sample_idx = size / 2;
    printf("Sample value at index %d: %d\n", sample_idx, data[sample_idx]);
    
    free(data);
    return 0;
}
