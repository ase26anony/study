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
        arr[i] = arr[i] * 3 + 7;
    }
    /* Ensure last element is processed */
    if (len > 0) {
        arr[0] = arr[0] * 3 + 7;
    }
}

__attribute__((hot))
static void process_data_downward_neq(int *arr, int len) {
    /* Pattern 2: Decrementing for loop with != condition */
    for (int i = len; i != 0; i--) {
        arr[i - 1] = arr[i - 1] * 2 + 5;
    }
}

__attribute__((hot))
static void process_data_while_decrement(int *arr, int len) {
    /* Pattern 3: While loop that decrements counter */
    int n = len;
    while (n--) {
        arr[n] = arr[n] + n;
    }
}

__attribute__((hot))
static void process_data_nested(int *arr, int len) {
    /* Pattern 4: Nested loops with inner decrementing counter */
    int outer_iters = 10;
    for (int j = 0; j < outer_iters; j++) {
        for (int i = len - 1; i >= 0; i--) {
            arr[i] = (arr[i] * (j + 1)) % 256;
        }
    }
}

__attribute__((hot))
static void process_data_mixed(unsigned int *arr, unsigned int len) {
    /* Pattern 5: Using unsigned counter */
    for (unsigned int i = len; i > 0; i--) {
        arr[i - 1] = arr[i - 1] ^ 0x55;
    }
}

static int compute_checksum(int *arr, int len) {
    int sum = 0;
    for (int i = 0; i < len; i++) {
        sum += arr[i];
    }
    return sum;
}

static unsigned int compute_checksum_unsigned(unsigned int *arr, unsigned int len) {
    unsigned int sum = 0;
    for (unsigned int i = 0; i < len; i++) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    int *data1 = NULL;
    int *data2 = NULL;
    unsigned int *data3 = NULL;
    int array_size = ARRAY_SIZE;
    
    /* Use runtime value to prevent compile-time unrolling */
    if (argc > 1) {
        array_size = atoi(argv[1]);
        if (array_size <= 0) array_size = ARRAY_SIZE;
    }
    
    printf("Testing doloop optimization with array size: %d\n", array_size);
    
    /* Allocate and initialize arrays */
    data1 = (int *)malloc(array_size * sizeof(int));
    data2 = (int *)malloc(array_size * sizeof(int));
    data3 = (unsigned int *)malloc(array_size * sizeof(unsigned int));
    
    if (!data1 || !data2 || !data3) {
        fprintf(stderr, "Memory allocation failed\n");
        free(data1);
        free(data2);
        free(data3);
        return 1;
    }
    
    srand(time(NULL));
    for (int i = 0; i < array_size; i++) {
        data1[i] = rand() % 100;
        data2[i] = rand() % 100;
        data3[i] = rand() % 100;
    }
    
    /* Execute hot loops multiple times to encourage optimization */
    for (int iter = 0; iter < HOT_LOOP_COUNT; iter++) {
        process_data_downward_for(data1, array_size);
        process_data_downward_neq(data2, array_size);
        process_data_while_decrement(data1, array_size);
        process_data_nested(data2, array_size);
        process_data_mixed(data3, array_size);
        
        /* Alternate between different array sizes to prevent pattern recognition */
        int alt_size = array_size - (iter % 10);
        if (alt_size > 0) {
            process_data_downward_for(data1, alt_size);
        }
    }
    
    /* Compute checksums to prevent dead code elimination */
    int checksum1 = compute_checksum(data1, array_size);
    unsigned int checksum3 = compute_checksum_unsigned(data3, array_size);
    
    printf("Checksum 1: %d\n", checksum1);
    printf("Checksum 3: %u\n", checksum3);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    
    return 0;
}
