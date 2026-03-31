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
        /* Non-trivial but simple computation to prevent removal */
        arr[i] = arr[i] * 3 + 7;
    }
    /* Handle element 0 separately to avoid i >= 0 condition */
    if (len > 0) {
        arr[0] = arr[0] * 3 + 7;
    }
}

__attribute__((hot))
static void process_data_downward_neq(int *arr, int len) {
    /* Pattern 2: Decrementing for loop with != condition */
    for (int i = len; i != 0; i--) {
        /* Different computation to avoid CSE between loops */
        arr[i - 1] = arr[i - 1] * 2 + arr[i % len];
    }
}

__attribute__((hot))
static void process_data_while_decrement(int *arr, int len) {
    /* Pattern 3: While loop that decrements counter */
    unsigned int n = len;
    while (n--) {
        /* Another distinct computation */
        arr[n] = (arr[n] << 1) | 1;
    }
}

__attribute__((hot))
static void process_data_nested_loops(int *arr, int len) {
    /* Pattern 4: Nested loops with inner decrementing counter */
    int outer_iters = len / 100;
    if (outer_iters < 1) outer_iters = 1;
    
    for (int j = 0; j < outer_iters; j++) {
        /* Inner loop with decrementing counter */
        int inner_len = 100;
        for (int i = inner_len - 1; i > 0; i--) {
            int idx = j * 100 + i;
            if (idx < len) {
                arr[idx] = arr[idx] + j - i;
            }
        }
    }
}

__attribute__((hot))
static unsigned long process_all_patterns(int *arr, int len) {
    unsigned long checksum = 0;
    
    /* Call all loop patterns to increase coverage chance */
    process_data_downward_for(arr, len);
    process_data_downward_neq(arr, len);
    process_data_while_decrement(arr, len);
    process_data_nested_loops(arr, len);
    
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
    for (int iter = 0; iter < HOT_LOOP_COUNT; iter++) {
        total_checksum += process_all_patterns(data, data_size);
        
        /* Occasionally re-initialize to vary patterns */
        if (iter % 100 == 99) {
            for (int i = 0; i < data_size; i++) {
                data[i] = (data[i] + iter) % 100;
            }
        }
    }
    
    printf("Final checksum: %lu\n", total_checksum);
    
    /* Use result to prevent optimization */
    if (total_checksum == 0) {
        printf("Zero checksum (unlikely)\n");
    }
    
    free(data);
    return 0;
}
