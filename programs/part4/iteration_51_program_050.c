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
        /* Non-trivial but simple operation to keep counter in register */
        arr[i] = arr[i] * 3 + 7;
    }
    /* Handle element 0 separately to avoid i >= 0 condition */
    if (len > 0) {
        arr[0] = arr[0] * 3 + 7;
    }
}

__attribute__((hot))
void process_data_downward_neq(int *arr, int len) {
    /* Pattern 2: Decrementing for loop with != condition */
    for (int i = len; i != 0; i--) {
        /* Different operation to avoid identical loop merging */
        arr[i - 1] = arr[i - 1] * 2 + 5;
    }
}

__attribute__((hot))
void process_data_while_decrement(int *arr, int len) {
    /* Pattern 3: While loop that decrements counter */
    int n = len;
    while (n--) {
        /* Another variation of operation */
        arr[n] = arr[n] + arr[n] * 4;
    }
}

__attribute__((hot))
void process_data_nested(int *arr, int len) {
    /* Pattern 4: Nested loop with inner decrementing counter */
    int outer_iters = 10;
    
    for (int outer = 0; outer < outer_iters; outer++) {
        /* Inner loop with decrementing counter */
        for (int i = len - 1; i > 0; i--) {
            /* Operation with outer loop dependency */
            arr[i] = arr[i] * (outer + 1) + i;
        }
        if (len > 0) {
            arr[0] = arr[0] * (outer + 1);
        }
    }
}

__attribute__((hot))
void process_data_mixed(unsigned int *arr, unsigned int len) {
    /* Pattern 5: Using unsigned counter (common in doloop) */
    for (unsigned int i = len; i > 0; i--) {
        arr[i - 1] = (arr[i - 1] << 1) | 1;
    }
}

/* Main driver that calls hot functions repeatedly */
int main(int argc, char *argv[]) {
    int *array1 = (int *)malloc(ARRAY_SIZE * sizeof(int));
    int *array2 = (int *)malloc(ARRAY_SIZE * sizeof(int));
    unsigned int *array3 = (unsigned int *)malloc(ARRAY_SIZE * sizeof(unsigned int));
    
    if (!array1 || !array2 || !array3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pseudo-random but deterministic values */
    srand(42);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
        array3[i] = rand() % 1000;
    }
    
    int len = ARRAY_SIZE;
    if (argc > 1) {
        len = atoi(argv[1]);
        if (len <= 0 || len > ARRAY_SIZE) {
            len = ARRAY_SIZE;
        }
    }
    
    clock_t start = clock();
    
    /* Repeatedly call hot functions to make loops "hot" */
    for (int iter = 0; iter < HOT_LOOP_COUNT; iter++) {
        /* Alternate between different loop patterns */
        switch (iter % 5) {
            case 0:
                process_data_downward_for(array1, len);
                break;
            case 1:
                process_data_downward_neq(array2, len);
                break;
            case 2:
                process_data_while_decrement(array1, len);
                break;
            case 3:
                process_data_nested(array2, len);
                break;
            case 4:
                process_data_mixed(array3, len);
                break;
        }
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum1 = 0, checksum2 = 0, checksum3 = 0;
    for (int i = 0; i < len; i++) {
        checksum1 += array1[i];
        checksum2 += array2[i];
        checksum3 += array3[i];
    }
    
    printf("Processing completed in %.3f seconds\n", elapsed);
    printf("Checksums: %llu, %llu, %llu\n", checksum1, checksum2, checksum3);
    
    free(array1);
    free(array2);
    free(array3);
    
    return 0;
}
