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
        /* Non-trivial but simple operation to keep counter in register */
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
        /* Different operation to avoid CSE */
        arr[i-1] = arr[i-1] * 2 + arr[i % len];
    }
}

__attribute__((hot))
static void process_data_while_decrement(int *arr, int len) {
    /* Pattern 3: While loop that decrements counter */
    int n = len;
    while (n--) {
        /* Another variation of operation */
        arr[n] = (arr[n] << 1) | 1;
    }
}

__attribute__((hot))
static void process_data_nested(int *arr, int len) {
    /* Pattern 4: Nested loop with inner decrementing counter */
    int outer = 10;  /* Small constant to keep loop manageable */
    
    for (int j = 0; j < outer; j++) {
        /* Inner loop with decrementing counter */
        for (int i = len - 1; i >= 0; i--) {
            /* Operation with outer loop dependency */
            arr[i] += j;
        }
    }
}

__attribute__((hot))
static unsigned long process_all_patterns(int *arr, int len) {
    unsigned long checksum = 0;
    
    /* Execute all loop patterns to increase coverage chance */
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

int main(int argc, char *argv[]) {
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
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (int i = 0; i < data_size; i++) {
        data[i] = rand() % 100;
    }
    
    /* Hot loop: call processing function many times */
    for (int iter = 0; iter < HOT_LOOP_COUNT; iter++) {
        /* Make small modifications to prevent complete optimization */
        data[iter % data_size] = iter;
        
        /* Process data with all loop patterns */
        total_checksum += process_all_patterns(data, data_size);
        
        /* Alternate between different sizes to avoid pattern recognition */
        int current_len = data_size - (iter % 10);
        if (current_len < 10) current_len = data_size;
        
        /* Call individual patterns directly sometimes */
        if (iter % 3 == 0) {
            process_data_downward_for(data, current_len);
        }
        if (iter % 5 == 0) {
            process_data_while_decrement(data, current_len);
        }
    }
    
    printf("Final checksum: %lu\n", total_checksum);
    
    /* Use result to prevent optimization */
    if (total_checksum > 0) {
        printf("Processing completed successfully\n");
    }
    
    free(data);
    return 0;
}
