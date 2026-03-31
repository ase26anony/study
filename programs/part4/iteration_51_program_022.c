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
        /* Non-trivial computation to prevent loop removal */
        arr[i] = arr[i] * 3 + 7;
    }
    /* Handle i=0 case separately to avoid zero comparison in loop */
    if (len > 0) {
        arr[0] = arr[0] * 3 + 7;
    }
}

__attribute__((hot))
static void process_data_downward_while(int *arr, int len) {
    /* Pattern 2: While loop with decrementing counter */
    int n = len;
    while (n-- > 0) {
        arr[n] = arr[n] * 2 + 1;
    }
}

__attribute__((hot))
static void process_data_downward_ne(int *arr, int len) {
    /* Pattern 3: For loop with != condition */
    for (int i = len; i != 0; i--) {
        arr[i - 1] = arr[i - 1] + i;
    }
}

__attribute__((hot))
static void process_data_nested(int *arr, int len) {
    /* Pattern 4: Nested loops with inner decrementing counter */
    int outer_iters = 10;
    
    for (int outer = 0; outer < outer_iters; outer++) {
        /* Inner loop with decrementing counter */
        for (int i = len - 1; i >= 0; i--) {
            /* More complex computation to prevent optimization */
            arr[i] = (arr[i] << 1) | (arr[i] >> 31);
        }
    }
}

__attribute__((hot))
static void process_data_mixed(int *arr, int len) {
    /* Pattern 5: Multiple decrementing loops in same function */
    int temp = len;
    
    /* First loop: while with post-decrement */
    while (temp--) {
        arr[temp] += temp * 2;
    }
    
    /* Second loop: for with pre-decrement style */
    for (int j = len - 1; j > 0; j--) {
        arr[j] ^= 0xFF;
    }
}

static unsigned long compute_checksum(int *arr, int len) {
    unsigned long sum = 0;
    for (int i = 0; i < len; i++) {
        sum += (unsigned long)arr[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    int *array;
    int size = ARRAY_SIZE;
    unsigned long checksum = 0;
    
    /* Use command line argument for size if provided */
    if (argc > 1) {
        size = atoi(argv[1]);
        if (size <= 0) size = ARRAY_SIZE;
    }
    
    /* Allocate and initialize array */
    array = (int *)malloc(size * sizeof(int));
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        array[i] = rand() % 100;
    }
    
    printf("Processing %d elements with %d hot iterations...\n", size, HOT_LOOP_COUNT);
    
    /* Execute hot loops multiple times to encourage optimization */
    for (int iter = 0; iter < HOT_LOOP_COUNT; iter++) {
        /* Call different loop patterns to increase coverage chance */
        process_data_downward_for(array, size);
        process_data_downward_while(array, size);
        process_data_downward_ne(array, size);
        process_data_nested(array, size);
        process_data_mixed(array, size);
        
        /* Occasionally recompute to prevent monotonic overflow */
        if (iter % 100 == 0) {
            for (int i = 0; i < size; i++) {
                array[i] = array[i] % 1000;
            }
        }
    }
    
    /* Compute and print checksum to prevent dead code elimination */
    checksum = compute_checksum(array, size);
    printf("Final checksum: %lu\n", checksum);
    
    free(array);
    return 0;
}
