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
        /* Non-trivial computation to prevent removal */
        arr[i] = arr[i] * 3 + 7;
    }
    /* Handle i=0 case separately to maintain > condition */
    if (len > 0) {
        arr[0] = arr[0] * 3 + 7;
    }
}

__attribute__((hot))
static void process_data_downward_for_ne(int *arr, int len) {
    /* Pattern 2: Decrementing for loop with != condition */
    for (int i = len; i != 0; i--) {
        /* Different computation pattern */
        arr[i - 1] = (arr[i - 1] << 1) | 1;
    }
}

__attribute__((hot))
static void process_data_while_decrement(int *arr, int len) {
    /* Pattern 3: While loop with decrement */
    int n = len;
    while (n--) {
        /* Another computation variant */
        arr[n] = arr[n] + n;
    }
}

__attribute__((hot))
static void process_data_nested_loops(int *arr, int len) {
    /* Pattern 4: Nested loops with inner decrementing counter */
    int outer_iters = 10;
    
    for (int j = 0; j < outer_iters; j++) {
        /* Inner loop with decrementing counter */
        for (int i = len - 1; i >= 0; i--) {
            /* Complex enough to prevent unrolling */
            arr[i] = (arr[i] * j) + (i & 0xFF);
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

static int compute_checksum(int *arr, int len) {
    int sum = 0;
    for (int i = 0; i < len; i++) {
        sum ^= arr[i];  /* XOR checksum to prevent elimination */
    }
    return sum;
}

int main(int argc, char *argv[]) {
    int *data;
    int size = ARRAY_SIZE;
    int checksum = 0;
    
    /* Use command line argument for size if provided */
    if (argc > 1) {
        size = atoi(argv[1]);
        if (size <= 0) size = ARRAY_SIZE;
    }
    
    printf("Testing doloop optimization with array size: %d\n", size);
    
    /* Allocate and initialize array */
    data = (int *)malloc(size * sizeof(int));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        data[i] = rand() % 1000;
    }
    
    /* Hot loop: call processing function many times */
    for (int iter = 0; iter < HOT_LOOP_COUNT; iter++) {
        process_data_mixed_patterns(data, size);
        
        /* Occasionally compute checksum to prevent dead code elimination */
        if (iter % 100 == 0) {
            checksum ^= compute_checksum(data, size);
        }
    }
    
    /* Final checksum computation */
    checksum ^= compute_checksum(data, size);
    printf("Final checksum: %d\n", checksum);
    
    free(data);
    return 0;
}
