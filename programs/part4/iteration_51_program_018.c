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
        /* Non-trivial but simple operation to prevent removal */
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
        /* Different operation to keep loop distinct */
        arr[i - 1] = (arr[i - 1] << 1) | 1;
    }
}

__attribute__((hot))
static void process_data_while_decrement(int *arr, int len) {
    /* Pattern 3: While loop with decrement */
    int n = len;
    while (n--) {
        /* Another variation of computation */
        arr[n] += n * 2;
    }
}

__attribute__((hot))
static void process_data_nested(int *arr, int len) {
    /* Pattern 4: Nested loops with inner decrementing counter */
    int outer_iters = 10;
    
    for (int outer = 0; outer < outer_iters; outer++) {
        /* Inner loop with decrementing counter */
        for (int i = len - 1; i >= 0; i--) {
            /* Simple accumulation */
            arr[i] += outer;
        }
    }
}

__attribute__((hot))
static unsigned long process_all_patterns(int *arr, int len) {
    unsigned long checksum = 0;
    
    /* Execute all loop patterns */
    process_data_downward_for(arr, len);
    process_data_downward_neq(arr, len);
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
        /* Make a copy to work on */
        int *work_data = (int *)malloc(data_size * sizeof(int));
        if (!work_data) continue;
        
        for (int i = 0; i < data_size; i++) {
            work_data[i] = data[i] + iter; /* Vary input slightly */
        }
        
        total_checksum += process_all_patterns(work_data, data_size);
        
        free(work_data);
    }
    
    /* Print result to prevent optimization */
    printf("Total checksum: %lu\n", total_checksum);
    
    free(data);
    return 0;
}
