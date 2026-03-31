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
        /* Non-trivial but simple computation to keep counter in register */
        arr[i] = arr[i] * 3 + 7;
    }
    /* Handle i=0 case separately to maintain > 0 condition */
    if (len > 0) {
        arr[0] = arr[0] * 3 + 7;
    }
}

__attribute__((hot))
static void process_data_downward_for_ne(int *arr, int len) {
    /* Pattern 2: Decrementing for loop with != condition */
    for (int i = len; i != 0; i--) {
        /* Different computation pattern */
        arr[i-1] = (arr[i-1] << 1) | 1;
    }
}

__attribute__((hot))
static void process_data_while_decrement(int *arr, int len) {
    /* Pattern 3: While loop that decrements counter */
    unsigned int count = len;
    while (count--) {
        /* Another computation variant */
        arr[count] += count * 2;
    }
}

__attribute__((hot))
static void process_data_nested_loops(int *arr, int len) {
    /* Pattern 4: Nested loops with inner decrementing counter */
    int outer_loops = 10;
    
    while (outer_loops--) {
        /* Inner loop with decrementing counter */
        for (int i = len - 1; i >= 0; i--) {
            /* Mix of operations */
            arr[i] = (arr[i] + outer_loops) ^ 0x55;
        }
    }
}

__attribute__((hot))
static void process_data_mixed_patterns(int *arr, int len) {
    /* Combine multiple patterns in one hot function */
    unsigned int temp_len = len;
    
    /* First pattern: for loop with > */
    for (int i = temp_len - 1; i > 0; i--) {
        arr[i] = arr[i] * 2 + 1;
    }
    if (temp_len > 0) {
        arr[0] = arr[0] * 2 + 1;
    }
    
    /* Second pattern: while with post-decrement */
    unsigned int counter = temp_len;
    while (counter) {
        counter--;
        arr[counter] ^= 0xFF;
    }
}

static long compute_checksum(int *arr, int len) {
    long sum = 0;
    for (int i = 0; i < len; i++) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    int *data;
    int size = ARRAY_SIZE;
    long total_checksum = 0;
    
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
        data[i] = rand() % 100;
    }
    
    printf("Processing %d elements for %d iterations...\n", size, HOT_LOOP_COUNT);
    
    /* Hot loop to make the inner loops "hot" for optimization */
    for (int iter = 0; iter < HOT_LOOP_COUNT; iter++) {
        /* Call different pattern functions to increase coverage chance */
        process_data_downward_for(data, size);
        process_data_downward_for_ne(data, size);
        process_data_while_decrement(data, size);
        
        /* Every 10 iterations, use nested pattern */
        if (iter % 10 == 0) {
            process_data_nested_loops(data, size);
        }
        
        /* Every 5 iterations, use mixed patterns */
        if (iter % 5 == 0) {
            process_data_mixed_patterns(data, size);
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    total_checksum = compute_checksum(data, size);
    printf("Final checksum: %ld\n", total_checksum);
    
    free(data);
    return 0;
}
