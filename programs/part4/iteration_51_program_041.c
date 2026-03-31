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
        arr[i] = arr[i] * 3 + arr[i - 1];
    }
    /* Handle last element separately to avoid out-of-bounds */
    if (len > 0) {
        arr[0] = arr[0] * 3;
    }
}

__attribute__((hot))
void process_data_downward_for_ne(int *arr, int len) {
    /* Pattern 2: Decrementing for loop with != condition */
    for (int i = len; i != 0; i--) {
        arr[i - 1] = arr[i - 1] * 2 + 7;
    }
}

__attribute__((hot))
void process_data_while_decrement(int *arr, int len) {
    /* Pattern 3: While loop that decrements counter */
    int n = len;
    while (n--) {
        arr[n] = (arr[n] << 1) | 1;
    }
}

__attribute__((hot))
void process_data_nested_loops(int *arr, int len) {
    /* Pattern 4: Nested loops with inner decrementing counter */
    int outer = len / 100;
    if (outer < 1) outer = 1;
    
    for (int j = 0; j < outer; j++) {
        int inner_len = len / outer;
        /* Inner loop with decrementing counter */
        for (int i = inner_len - 1; i >= 0; i--) {
            int idx = j * inner_len + i;
            if (idx < len) {
                arr[idx] = arr[idx] + j - i;
            }
        }
    }
}

__attribute__((hot))
void process_data_mixed_patterns(int *arr, int len) {
    /* Combine multiple patterns in one function */
    unsigned int count = len;
    
    /* First pattern */
    while (count > 0) {
        arr[count - 1] += count;
        count--;
    }
    
    /* Second pattern */
    for (unsigned int i = len; i != 0; i--) {
        arr[i - 1] ^= 0x55AA55AA;
    }
}

int main(int argc, char *argv[]) {
    int *data;
    int size = ARRAY_SIZE;
    long long checksum = 0;
    
    /* Use command line argument for size if provided */
    if (argc > 1) {
        size = atoi(argv[1]);
        if (size <= 0) size = ARRAY_SIZE;
    }
    
    /* Allocate and initialize array */
    data = (int *)malloc(size * sizeof(int));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        data[i] = rand() % 1000;
    }
    
    printf("Processing %d elements %d times...\n", size, HOT_LOOP_COUNT);
    
    /* Execute hot loops multiple times to encourage optimization */
    for (int iter = 0; iter < HOT_LOOP_COUNT; iter++) {
        /* Call different loop patterns */
        process_data_downward_for(data, size);
        process_data_downward_for_ne(data, size);
        process_data_while_decrement(data, size);
        process_data_nested_loops(data, size);
        process_data_mixed_patterns(data, size);
        
        /* Add some variation to prevent complete optimization */
        data[iter % size] += iter;
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < size; i++) {
        checksum += data[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    free(data);
    return 0;
}
