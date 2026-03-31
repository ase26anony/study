/* test_doloop.c - Program to trigger doloop optimization validation */
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
    /* Handle i=0 case separately to maintain > 0 condition */
    if (len > 0) {
        arr[0] = arr[0] * 3 + 7;
    }
}

__attribute__((hot))
static void process_data_downward_for_ne(int *arr, int len) {
    /* Pattern 2: Decrementing for loop with != condition */
    for (int i = len; i != 0; i--) {
        /* Different operation to avoid CSE between loops */
        arr[i - 1] = arr[i - 1] * 2 + 5;
    }
}

__attribute__((hot))
static void process_data_while_decrement(int *arr, int len) {
    /* Pattern 3: While loop with decrement */
    int n = len;
    while (n--) {
        /* Another distinct operation */
        arr[n] = arr[n] + arr[n] / 2;
    }
}

__attribute__((hot))
static void process_data_nested(int *arr, int len) {
    /* Pattern 4: Nested loops with inner decrementing counter */
    int outer = 10;
    while (outer--) {
        for (int i = len - 1; i >= 0; i--) {
            /* Complex enough to prevent unrolling */
            arr[i] = (arr[i] << 1) | (arr[i] >> 31);
        }
    }
}

__attribute__((hot))
static unsigned long process_all_patterns(int *arr, int len) {
    unsigned long checksum = 0;
    
    /* Call all loop patterns to increase coverage chance */
    process_data_downward_for(arr, len);
    process_data_downward_for_ne(arr, len);
    process_data_while_decrement(arr, len);
    process_data_nested(arr, len);
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < len; i++) {
        checksum += arr[i];
    }
    
    return checksum;
}

int main(int argc, char **argv) {
    int *data;
    int data_size = ARRAY_SIZE;
    unsigned long total_checksum = 0;
    
    /* Use command line argument for size if provided */
    if (argc > 1) {
        data_size = atoi(argv[1]);
        if (data_size <= 0) data_size = ARRAY_SIZE;
    }
    
    /* Allocate and initialize array */
    data = (int *)malloc(data_size * sizeof(int));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random but deterministic values */
    srand(42);
    for (int i = 0; i < data_size; i++) {
        data[i] = rand() % 100;
    }
    
    /* Time measurement for profiling */
    clock_t start = clock();
    
    /* Hot loop calling the processing function many times */
    for (int iter = 0; iter < HOT_LOOP_COUNT; iter++) {
        total_checksum += process_all_patterns(data, data_size);
        
        /* Occasionally modify data to prevent complete optimization */
        if (iter % 100 == 0) {
            data[iter % data_size] = iter;
        }
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    /* Print results to prevent optimization removal */
    printf("Checksum: %lu\n", total_checksum);
    printf("Time: %.3f seconds\n", elapsed);
    printf("Data[0]=%d, Data[%d]=%d\n", 
           data[0], data_size-1, data[data_size-1]);
    
    free(data);
    return 0;
}
