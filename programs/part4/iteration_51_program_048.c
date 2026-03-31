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
        arr[i] = arr[i] * 3 + 7;
    }
    /* Handle element 0 separately to avoid i >= 0 condition */
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
    /* Pattern 3: While loop that decrements counter */
    int n = len;
    while (n--) {
        /* Another computation variant */
        arr[n] = arr[n] + n;
    }
}

__attribute__((hot))
static void process_data_nested(int *arr, int len) {
    /* Pattern 4: Nested loops with inner decrementing counter */
    int outer_iters = 10;
    
    for (int outer = 0; outer < outer_iters; outer++) {
        /* Inner loop with decrementing counter */
        for (int i = len - 1; i > 0; i--) {
            /* Complex enough to not be optimized away */
            arr[i] = (arr[i] ^ outer) + i;
        }
        if (len > 0) {
            arr[0] = (arr[0] ^ outer);
        }
    }
}

__attribute__((hot))
static unsigned long process_all_patterns(int *arr, int len) {
    unsigned long checksum = 0;
    
    /* Execute all loop patterns */
    process_data_downward_for(arr, len);
    process_data_downward_for_ne(arr, len);
    process_data_while_decrement(arr, len);
    process_data_nested(arr, len);
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < len; i++) {
        checksum += arr[i];
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int *data;
    int data_size = ARRAY_SIZE;
    unsigned long total_checksum = 0;
    
    /* Use command line argument for size if provided */
    if (argc > 1) {
        data_size = atoi(argv[1]);
        if (data_size <= 0) data_size = ARRAY_SIZE;
    }
    
    printf("Testing doloop optimization with array size: %d\n", data_size);
    
    /* Allocate and initialize array */
    data = (int *)malloc(data_size * sizeof(int));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (int i = 0; i < data_size; i++) {
        data[i] = rand() % 100;
    }
    
    /* Hot loop: call processing function many times */
    clock_t start = clock();
    
    for (int iteration = 0; iteration < HOT_LOOP_COUNT; iteration++) {
        /* Process data with all loop patterns */
        total_checksum += process_all_patterns(data, data_size);
        
        /* Occasionally re-initialize to vary data */
        if (iteration % 100 == 0) {
            for (int i = 0; i < data_size; i++) {
                data[i] = (data[i] + iteration) % 100;
            }
        }
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Total checksum: %lu\n", total_checksum);
    printf("Elapsed time: %.3f seconds\n", elapsed);
    printf("Iterations: %d\n", HOT_LOOP_COUNT);
    
    free(data);
    return 0;
}
