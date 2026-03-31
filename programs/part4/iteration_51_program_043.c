/* test_doloop.c - Program to trigger doloop optimization validation logic */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define HOT_LOOP_COUNT 1000
#define ARRAY_SIZE 10000

/* Mark function as hot to encourage loop optimization */
__attribute__((hot))
void process_data_downward_for(int *arr, int len) {
    /* Pattern 1: Basic decrementing for loop with > condition */
    for (int i = len - 1; i > 0; i--) {
        /* Non-trivial but simple computation to keep counter in register */
        arr[i] = arr[i] * 3 + arr[i-1];
    }
    /* Handle element 0 separately to avoid underflow */
    if (len > 0) {
        arr[0] = arr[0] * 3;
    }
}

__attribute__((hot))
void process_data_downward_neq(int *arr, int len) {
    /* Pattern 2: Decrementing for loop with != condition */
    for (int i = len; i != 0; i--) {
        /* Different computation pattern */
        arr[i-1] = (arr[i-1] << 1) | 1;
    }
}

__attribute__((hot))
void process_data_while_decrement(int *arr, int len) {
    /* Pattern 3: While loop that decrements counter */
    int n = len;
    while (n--) {
        /* Simple array processing */
        arr[n] += n * 2;
    }
}

__attribute__((hot))
void process_data_nested_loops(int *arr, int len) {
    /* Pattern 4: Nested loops with inner decrementing counter */
    int outer_iters = 10;
    
    for (int outer = 0; outer < outer_iters; outer++) {
        /* Inner loop with decrementing counter */
        for (int i = len - 1; i >= 0; i--) {
            /* Mix of operations to prevent other optimizations */
            arr[i] = (arr[i] ^ outer) + i;
        }
    }
}

__attribute__((hot))
void process_data_mixed_patterns(int *arr, int len) {
    /* Combine multiple patterns in one function */
    
    /* First pattern */
    unsigned int count = len;
    while (count-- > 0) {
        arr[count] = arr[count] * 2 + 5;
    }
    
    /* Second pattern */
    for (int i = len - 1; i != 0; i--) {
        arr[i] = arr[i] - arr[0];
    }
}

int main(int argc, char *argv[]) {
    int *data;
    int size = ARRAY_SIZE;
    int i, j;
    long long checksum = 0;
    
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
    for (i = 0; i < size; i++) {
        data[i] = rand() % 100;
    }
    
    /* Run hot loops multiple times to encourage optimization */
    clock_t start = clock();
    
    for (j = 0; j < HOT_LOOP_COUNT; j++) {
        /* Call different loop patterns to increase coverage chance */
        process_data_downward_for(data, size);
        process_data_downward_neq(data, size);
        process_data_while_decrement(data, size);
        process_data_nested_loops(data, size);
        process_data_mixed_patterns(data, size);
        
        /* Occasionally modify array to prevent complete optimization */
        if (j % 100 == 0) {
            data[0] = j;
        }
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    /* Compute checksum to prevent dead code elimination */
    for (i = 0; i < size; i++) {
        checksum += data[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    printf("Elapsed time: %.3f seconds\n", elapsed);
    printf("Loops executed: %d\n", HOT_LOOP_COUNT * 5);
    
    free(data);
    return 0;
}
