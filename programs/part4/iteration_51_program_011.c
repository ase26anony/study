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
        /* Non-trivial computation to prevent removal */
        arr[i] = arr[i] * 3 + arr[i - 1];
    }
    /* Handle last element separately to avoid out-of-bounds */
    if (len > 0) {
        arr[0] = arr[0] * 3 + 1;
    }
}

__attribute__((hot))
static void process_data_downward_neq(int *arr, int len) {
    /* Pattern 2: Decrementing for loop with != condition */
    for (int i = len; i != 0; i--) {
        /* Different computation pattern */
        int idx = i - 1;
        arr[idx] = (arr[idx] << 1) | (arr[idx] >> 31);
    }
}

__attribute__((hot))
static void process_data_while_decrement(int *arr, int len) {
    /* Pattern 3: While loop with decrement */
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
        for (int i = len - 1; i >= 0; i--) {
            /* Computation with outer loop dependency */
            arr[i] = arr[i] + outer;
        }
    }
}

__attribute__((hot))
static void process_data_mixed(unsigned *arr, int len) {
    /* Pattern 5: Mixed unsigned counter */
    unsigned count = (unsigned)len;
    
    while (count > 0) {
        unsigned idx = count - 1;
        arr[idx] = arr[idx] ^ 0x5A5A5A5A;
        count--;
    }
}

static int compute_checksum(int *arr, int len) {
    int sum = 0;
    for (int i = 0; i < len; i++) {
        sum += arr[i];
    }
    return sum;
}

static unsigned compute_checksum_unsigned(unsigned *arr, int len) {
    unsigned sum = 0;
    for (int i = 0; i < len; i++) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    int *data1 = NULL;
    int *data2 = NULL;
    unsigned *data3 = NULL;
    int array_size = ARRAY_SIZE;
    
    /* Allow runtime size specification */
    if (argc > 1) {
        array_size = atoi(argv[1]);
        if (array_size <= 0) array_size = ARRAY_SIZE;
    }
    
    printf("Testing doloop optimization with array size: %d\n", array_size);
    
    /* Allocate and initialize arrays */
    data1 = (int*)malloc(array_size * sizeof(int));
    data2 = (int*)malloc(array_size * sizeof(int));
    data3 = (unsigned*)malloc(array_size * sizeof(unsigned));
    
    if (!data1 || !data2 || !data3) {
        fprintf(stderr, "Memory allocation failed\n");
        free(data1);
        free(data2);
        free(data3);
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (int i = 0; i < array_size; i++) {
        data1[i] = rand() % 100;
        data2[i] = rand() % 100;
        data3[i] = rand() % 100;
    }
    
    /* Execute hot loops multiple times to encourage optimization */
    clock_t start = clock();
    
    for (int iter = 0; iter < HOT_LOOP_COUNT; iter++) {
        /* Alternate between different loop patterns */
        switch (iter % 5) {
            case 0:
                process_data_downward_for(data1, array_size);
                break;
            case 1:
                process_data_downward_neq(data2, array_size);
                break;
            case 2:
                process_data_while_decrement(data1, array_size);
                break;
            case 3:
                process_data_nested(data2, array_size);
                break;
            case 4:
                process_data_mixed(data3, array_size);
                break;
        }
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    /* Compute checksums to prevent dead code elimination */
    int checksum1 = compute_checksum(data1, array_size);
    int checksum2 = compute_checksum(data2, array_size);
    unsigned checksum3 = compute_checksum_unsigned(data3, array_size);
    
    printf("Checksums: %d, %d, %u\n", checksum1, checksum2, checksum3);
    printf("Execution time: %.3f seconds\n", elapsed);
    printf("Loops executed: %d\n", HOT_LOOP_COUNT);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    
    return 0;
}
