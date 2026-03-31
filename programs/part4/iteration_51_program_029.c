/* test_doloop.c - Program to trigger doloop optimization validation logic */
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
        arr[i] = arr[i] * 3 + arr[i - 1];
    }
    /* Handle last element separately to avoid out-of-bounds */
    if (len > 0) {
        arr[0] = arr[0] * 3;
    }
}

__attribute__((hot))
static void process_data_downward_neq(int *arr, int len) {
    /* Pattern 2: Decrementing for loop with != condition */
    for (int i = len; i != 0; i--) {
        /* Different computation pattern */
        arr[i - 1] = (arr[i - 1] << 1) | 0x1;
    }
}

__attribute__((hot))
static void process_data_while_decrement(int *arr, int len) {
    /* Pattern 3: While loop with decrement */
    int n = len;
    while (n--) {
        /* Another computation variant */
        arr[n] += n * 7;
    }
}

__attribute__((hot))
static void process_data_nested_loops(int *arr, int len) {
    /* Pattern 4: Nested loops with inner decrementing counter */
    int outer_iters = 10;
    
    for (int outer = 0; outer < outer_iters; outer++) {
        /* Inner loop with decrementing counter */
        for (int i = len - 1; i >= 0; i--) {
            /* Complex enough to not be optimized away */
            arr[i] = (arr[i] ^ 0x55) + outer;
        }
    }
}

__attribute__((hot))
static void process_data_mixed_patterns(int *arr, int len) {
    /* Use all patterns in sequence */
    process_data_downward_for(arr, len);
    process_data_downward_neq(arr, len);
    process_data_while_decrement(arr, len);
    process_data_nested_loops(arr, len);
}

/* Compute checksum to prevent dead code elimination */
static int compute_checksum(int *arr, int len) {
    int sum = 0;
    for (int i = 0; i < len; i++) {
        sum ^= arr[i];
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
    
    printf("Processing %d elements with %d hot iterations...\n", 
           size, HOT_LOOP_COUNT);
    
    /* Execute hot loops many times to encourage optimization */
    for (int iter = 0; iter < HOT_LOOP_COUNT; iter++) {
        /* Alternate between different loop patterns */
        if (iter % 4 == 0) {
            process_data_downward_for(data, size);
        } else if (iter % 4 == 1) {
            process_data_downward_neq(data, size);
        } else if (iter % 4 == 2) {
            process_data_while_decrement(data, size);
        } else {
            process_data_nested_loops(data, size);
        }
        
        /* Also call the mixed function occasionally */
        if (iter % 10 == 0) {
            process_data_mixed_patterns(data, size);
        }
    }
    
    /* Compute and print checksum to ensure computation isn't eliminated */
    checksum = compute_checksum(data, size);
    printf("Final checksum: 0x%08x\n", checksum);
    
    free(data);
    return 0;
}
