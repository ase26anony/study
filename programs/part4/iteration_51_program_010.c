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
        arr[i - 1] = arr[i - 1] * 2 + arr[i - 1] / 2;
    }
}

__attribute__((hot))
static void process_data_while_decrement(int *arr, int len) {
    /* Pattern 3: While loop with decrementing counter */
    unsigned int n = len;
    while (n--) {
        /* Another distinct computation */
        arr[n] = arr[n] + (arr[n] >> 1) - 3;
    }
}

__attribute__((hot))
static void process_data_nested(int *arr, int len) {
    /* Pattern 4: Nested loops with inner decrementing counter */
    int chunks = 4;
    int chunk_size = len / chunks;
    
    for (int chunk = 0; chunk < chunks; chunk++) {
        int start = chunk * chunk_size;
        int end = start + chunk_size;
        
        /* Inner loop with decrementing counter */
        for (int i = end - 1; i >= start; i--) {
            arr[i] = arr[i] ^ 0x55AA;  /* Simple bitwise operation */
        }
    }
    
    /* Handle remainder */
    for (int i = chunks * chunk_size; i < len; i++) {
        arr[i] = arr[i] ^ 0x55AA;
    }
}

__attribute__((hot))
static unsigned long long process_all_patterns(int *arr, int len) {
    unsigned long long checksum = 0;
    
    /* Call all loop patterns to increase coverage chance */
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
    unsigned long long total_checksum = 0;
    
    /* Use command line argument for size if provided */
    if (argc > 1) {
        data_size = atoi(argv[1]);
        if (data_size <= 0) data_size = ARRAY_SIZE;
    }
    
    printf("Testing doloop optimization with array size: %d\n", data_size);
    
    /* Allocate and initialize array with runtime values */
    data = (int *)malloc(data_size * sizeof(int));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random but deterministic values */
    srand(42);
    for (int i = 0; i < data_size; i++) {
        data[i] = rand() % 1000;
    }
    
    clock_t start = clock();
    
    /* Hot loop: call processing function many times */
    for (int iter = 0; iter < HOT_LOOP_COUNT; iter++) {
        /* Slightly modify data each iteration to prevent optimization */
        data[iter % data_size] = iter;
        
        total_checksum += process_all_patterns(data, data_size);
        
        /* Prevent compiler from moving computations out of loop */
        asm volatile("" : : "r"(data) : "memory");
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Total checksum: %llu\n", total_checksum);
    printf("Elapsed time: %.3f seconds\n", elapsed);
    printf("Iterations: %d\n", HOT_LOOP_COUNT);
    
    free(data);
    return 0;
}
