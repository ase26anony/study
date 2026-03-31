/* test_doloop.c - Program to trigger doloop optimization validation logic */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define HOT_LOOP_COUNT 10000
#define ARRAY_SIZE 1024

/* Mark function as hot to encourage loop optimization */
__attribute__((hot))
static void process_data_downward_for(int *arr, int len) {
    /* Pattern 1: Basic decrementing for loop with > condition */
    for (int i = len - 1; i > 0; i--) {
        /* Non-trivial computation to prevent loop removal */
        arr[i] = arr[i] * 3 + arr[i - 1];
    }
}

__attribute__((hot))
static void process_data_downward_neq(int *arr, int len) {
    /* Pattern 2: Decrementing for loop with != condition */
    for (int i = len - 1; i != 0; i--) {
        /* Different computation pattern */
        arr[i] = (arr[i] << 1) | (arr[i] >> 31);
    }
}

__attribute__((hot))
static void process_data_while_decrement(int *arr, int len) {
    /* Pattern 3: While loop with decrement */
    int n = len;
    while (n--) {
        /* Another computation variant */
        arr[n] = arr[n] + n * 7;
    }
}

__attribute__((hot))
static void process_data_nested(int *arr, int len) {
    /* Pattern 4: Nested loops with inner decrementing counter */
    for (int outer = 0; outer < 10; outer++) {
        for (int inner = len - 1; inner > 0; inner--) {
            /* Complex enough to avoid simplification */
            arr[inner] = (arr[inner] ^ outer) + inner;
        }
    }
}

__attribute__((hot))
static void process_data_mixed(int *arr, int len) {
    /* Pattern 5: Mixed decrement patterns in same function */
    unsigned int count = len;
    
    /* First loop: unsigned counter with != 0 */
    for (unsigned int i = count; i != 0; i--) {
        arr[i - 1] = arr[i - 1] * 2 + 1;
    }
    
    /* Second loop: signed counter with > 0 */
    int j = len / 2;
    while (j > 0) {
        arr[j] = arr[j] - arr[j - 1];
        j--;
    }
}

static void initialize_array(int *arr, int len) {
    for (int i = 0; i < len; i++) {
        arr[i] = (i * 17 + 13) % 256;
    }
}

static int compute_checksum(int *arr, int len) {
    int sum = 0;
    for (int i = 0; i < len; i++) {
        sum = (sum * 31 + arr[i]) & 0xFFFF;
    }
    return sum;
}

int main(int argc, char *argv[]) {
    int array_size = ARRAY_SIZE;
    
    /* Use command line argument for size if provided */
    if (argc > 1) {
        array_size = atoi(argv[1]);
        if (array_size < 10) array_size = ARRAY_SIZE;
    }
    
    int *data = (int *)malloc(array_size * sizeof(int));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    initialize_array(data, array_size);
    
    printf("Processing %d elements for %d iterations...\n", 
           array_size, HOT_LOOP_COUNT);
    
    clock_t start = clock();
    
    /* Hot loop to make the functions "hot" for the optimizer */
    for (int iter = 0; iter < HOT_LOOP_COUNT; iter++) {
        /* Call different loop patterns to increase coverage chance */
        process_data_downward_for(data, array_size);
        process_data_downward_neq(data, array_size);
        process_data_while_decrement(data, array_size);
        process_data_nested(data, array_size);
        process_data_mixed(data, array_size);
        
        /* Occasionally reinitialize to prevent overflow */
        if (iter % 1000 == 0) {
            initialize_array(data, array_size);
        }
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    int checksum = compute_checksum(data, array_size);
    
    printf("Checksum: 0x%04X\n", checksum);
    printf("Elapsed time: %.2f seconds\n", elapsed);
    printf("Processed %d total iterations\n", 
           HOT_LOOP_COUNT * 5); /* 5 different loop patterns */
    
    free(data);
    return 0;
}
