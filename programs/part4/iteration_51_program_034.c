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
        arr[i] = arr[i] * 3 + arr[i - 1];
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
        arr[i - 1] = (arr[i - 1] << 1) | 0x1;
    }
}

__attribute__((hot))
void process_data_while_decrement(int *arr, int len) {
    /* Pattern 3: While loop that decrements counter */
    int n = len;
    while (n--) {
        /* Another computation variant */
        arr[n] += n * 7;
    }
}

__attribute__((hot))
void process_data_nested(int *arr, int len) {
    /* Pattern 4: Nested loops with inner decrementing counter */
    int outer;
    int inner_len = len / 4;
    
    for (outer = 0; outer < 4; outer++) {
        int start = outer * inner_len;
        int end = start + inner_len;
        
        /* Inner loop with decrementing counter */
        for (int i = end - 1; i >= start; i--) {
            /* Mix of operations to prevent other optimizations */
            arr[i] = (arr[i] ^ 0x55) + outer;
        }
    }
    
    /* Handle remainder if any */
    for (int i = 4 * inner_len; i < len; i++) {
        arr[i] = (arr[i] ^ 0xAA);
    }
}

__attribute__((hot))
void process_data_mixed(unsigned *arr, unsigned len) {
    /* Pattern 5: Using unsigned counter (common in doloop) */
    for (unsigned i = len; i > 0; i--) {
        /* Bit manipulation operations */
        arr[i - 1] = (arr[i - 1] * 0x9E3779B9) ^ (i - 1);
    }
}

/* Main driver that calls hot functions repeatedly */
int main(int argc, char *argv[]) {
    int *array1;
    int *array2;
    unsigned *array3;
    int array_size = ARRAY_SIZE;
    int i, j;
    long long checksum = 0;
    
    /* Use command line argument for size if provided */
    if (argc > 1) {
        array_size = atoi(argv[1]);
        if (array_size <= 0) array_size = ARRAY_SIZE;
    }
    
    printf("Testing doloop optimization with array size: %d\n", array_size);
    
    /* Allocate arrays */
    array1 = (int*)malloc(array_size * sizeof(int));
    array2 = (int*)malloc(array_size * sizeof(int));
    array3 = (unsigned*)malloc(array_size * sizeof(unsigned));
    
    if (!array1 || !array2 || !array3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pseudo-random data */
    srand(time(NULL));
    for (i = 0; i < array_size; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
        array3[i] = rand() % 1000;
    }
    
    /* Repeatedly call hot functions to make loops "hot" */
    for (j = 0; j < HOT_LOOP_COUNT; j++) {
        /* Call different loop patterns */
        process_data_downward_for(array1, array_size);
        process_data_downward_neq(array2, array_size);
        process_data_while_decrement(array1, array_size);
        process_data_nested(array2, array_size);
        process_data_mixed(array3, array_size);
        
        /* Alternate patterns to keep optimizer interested */
        if (j % 2 == 0) {
            process_data_downward_for(array2, array_size);
            process_data_while_decrement(array3, array_size);
        } else {
            process_data_downward_neq(array1, array_size);
            process_data_nested(array3, array_size);
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (i = 0; i < array_size; i++) {
        checksum += array1[i] + array2[i] + array3[i];
    }
    
    printf("Final checksum: %lld\n", checksum);
    
    /* Clean up */
    free(array1);
    free(array2);
    free(array3);
    
    return 0;
}
