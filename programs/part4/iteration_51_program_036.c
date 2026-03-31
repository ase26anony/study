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
        /* Non-trivial but simple operation to keep counter in register */
        arr[i] = arr[i] * 3 + 7;
    }
    /* Handle i=0 case separately to avoid zero comparison in loop */
    if (len > 0) {
        arr[0] = arr[0] * 3 + 7;
    }
}

__attribute__((hot))
static void process_data_downward_for_ne(int *arr, int len) {
    /* Pattern 2: Decrementing for loop with != condition */
    for (int i = len; i != 0; i--) {
        /* Different operation to avoid identical loop fusion */
        arr[i - 1] = arr[i - 1] * 2 + 1;
    }
}

__attribute__((hot))
static void process_data_while_decrement(int *arr, int len) {
    /* Pattern 3: While loop with decrementing counter */
    int n = len;
    while (n--) {
        /* Another variation of operation */
        arr[n] = arr[n] + (arr[n] >> 2);
    }
}

__attribute__((hot))
static void process_data_nested_loops(int *arr, int len) {
    /* Pattern 4: Nested loops with inner decrementing counter */
    int outer_iters = 10;
    
    for (int outer = 0; outer < outer_iters; outer++) {
        /* Inner loop with decrementing counter */
        for (int i = len - 1; i >= 0; i--) {
            /* Accumulate with outer loop index */
            arr[i] = arr[i] + outer;
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
        data[i] = rand() % 100;
    }
    
    /* Initial checksum */
    total_checksum = compute_checksum(data, size);
    printf("Initial checksum: %lu\n", total_checksum);
    
    /* Hot loop to make the functions "hot" for the optimizer */
    clock_t start = clock();
    for (int iter = 0; iter < HOT_LOOP_COUNT; iter++) {
        /* Call all pattern functions to increase coverage chance */
        process_data_mixed_patterns(data, size);
        
        /* Occasionally compute checksum to prevent over-optimization */
        if (iter % 100 == 0) {
            total_checksum += compute_checksum(data, size);
        }
    }
    clock_t end = clock();
    
    /* Final checksum */
    total_checksum += compute_checksum(data, size);
    
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Final checksum: %lu\n", total_checksum);
    printf("Execution time: %.3f seconds\n", elapsed);
    printf("Loops processed: %d\n", HOT_LOOP_COUNT);
    
    free(data);
    return 0;
}
