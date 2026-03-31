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
        /* Non-trivial but simple operation to prevent removal */
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
static void process_data_nested_loops(int *arr, int len) {
    /* Pattern 4: Nested loops with inner decrementing counter */
    int outer_iters = 10;
    
    for (int outer = 0; outer < outer_iters; outer++) {
        /* Inner loop with decrementing counter */
        for (int i = len - 1; i >= 0; i--) {
            arr[i] = (arr[i] + outer) & 0xFF;
        }
    }
}

__attribute__((hot))
static void process_data_mixed_patterns(int *arr, int len) {
    /* Use different patterns based on array size */
    if (len > 100) {
        process_data_downward_for(arr, len);
    } else {
        process_data_downward_for_ne(arr, len);
    }
}

static unsigned int compute_checksum(int *arr, int len) {
    unsigned int sum = 0;
    for (int i = 0; i < len; i++) {
        sum += (unsigned int)arr[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    int *data;
    int data_size = ARRAY_SIZE;
    unsigned int checksum = 0;
    clock_t start, end;
    
    /* Use command line argument for size if provided */
    if (argc > 1) {
        data_size = atoi(argv[1]);
        if (data_size <= 0) data_size = ARRAY_SIZE;
    }
    
    printf("Testing doloop optimization with array size: %d\n", data_size);
    
    /* Allocate and initialize array */
    data = (int *)malloc(data_size * sizeof(int));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    srand(42);
    for (int i = 0; i < data_size; i++) {
        data[i] = rand() % 100;
    }
    
    start = clock();
    
    /* Call hot functions many times to encourage optimization */
    for (int iter = 0; iter < HOT_LOOP_COUNT; iter++) {
        /* Alternate between different loop patterns */
        switch (iter % 4) {
            case 0:
                process_data_downward_for(data, data_size);
                break;
            case 1:
                process_data_downward_for_ne(data, data_size);
                break;
            case 2:
                process_data_while_decrement(data, data_size);
                break;
            case 3:
                process_data_nested_loops(data, data_size);
                break;
        }
        
        /* Occasionally use mixed patterns */
        if (iter % 100 == 0) {
            process_data_mixed_patterns(data, data_size);
        }
    }
    
    end = clock();
    
    checksum = compute_checksum(data, data_size);
    
    printf("Checksum: %u\n", checksum);
    printf("Time: %.2f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    
    free(data);
    return 0;
}
