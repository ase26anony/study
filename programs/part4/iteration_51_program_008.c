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
        /* Non-trivial but simple computation to keep counter in register */
        arr[i] = arr[i] * 3 + 7;
    }
    /* Handle i=0 case separately to maintain > condition */
    if (len > 0) {
        arr[0] = arr[0] * 3 + 7;
    }
}

__attribute__((hot))
static void process_data_downward_neq(int *arr, int len) {
    /* Pattern 2: Decrementing for loop with != condition */
    for (int i = len; i != 0; i--) {
        /* Different computation pattern */
        arr[i-1] = (arr[i-1] << 1) | 1;
    }
}

__attribute__((hot))
static void process_data_while_decrement(int *arr, int len) {
    /* Pattern 3: While loop that decrements counter */
    int n = len;
    while (n--) {
        /* Another computation variant */
        arr[n] += n * 2;
    }
}

__attribute__((hot))
static void process_data_nested_loops(int *arr, int len) {
    /* Pattern 4: Nested loops with inner decrementing counter */
    int outer_iters = 10;
    
    for (int outer = 0; outer < outer_iters; outer++) {
        /* Inner loop with decrementing counter */
        for (int i = len - 1; i >= 0; i--) {
            /* Mix of operations to prevent other optimizations */
            arr[i] = (arr[i] ^ 0x55) + outer;
        }
    }
}

__attribute__((hot))
static void process_data_mixed_patterns(int *arr, int len) {
    /* Combine multiple patterns in one function */
    int temp_len = len;
    
    /* First pattern */
    while (temp_len-- > 0) {
        arr[temp_len] = arr[temp_len] * 2 - 1;
    }
    
    /* Reset and use different pattern */
    temp_len = len;
    for (int i = temp_len; i > 0; i--) {
        arr[i-1] = arr[i-1] + (i % 256);
    }
}

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
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        data[i] = rand() % 1000;
    }
    
    printf("Testing doloop optimization patterns on array of size %d\n", size);
    printf("Running %d hot iterations...\n", HOT_LOOP_COUNT);
    
    /* Execute hot loops many times to encourage optimization */
    for (int iter = 0; iter < HOT_LOOP_COUNT; iter++) {
        /* Alternate between different patterns */
        switch (iter % 5) {
            case 0:
                process_data_downward_for(data, size);
                break;
            case 1:
                process_data_downward_neq(data, size);
                break;
            case 2:
                process_data_while_decrement(data, size);
                break;
            case 3:
                process_data_nested_loops(data, size);
                break;
            case 4:
                process_data_mixed_patterns(data, size);
                break;
        }
        
        /* Prevent compiler from optimizing away loops */
        if (iter % 100 == 0) {
            checksum ^= compute_checksum(data, size);
        }
    }
    
    /* Final checksum */
    checksum ^= compute_checksum(data, size);
    printf("Final checksum: 0x%08x\n", checksum);
    
    free(data);
    return 0;
}
