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
        /* Non-trivial but simple operation to keep counter in register */
        arr[i] = arr[i] * 3 + arr[i - 1];
    }
    /* Handle i=0 case separately to avoid negative index */
    if (len > 0) {
        arr[0] = arr[0] * 3;
    }
}

__attribute__((hot))
static void process_data_downward_neq(int *arr, int len) {
    /* Pattern 2: Decrementing for loop with != condition */
    for (int i = len; i != 0; i--) {
        arr[i - 1] = arr[i - 1] * 2 + 1;
    }
}

__attribute__((hot))
static void process_data_while_decrement(int *arr, int len) {
    /* Pattern 3: While loop that decrements counter */
    int n = len;
    while (n--) {
        arr[n] = arr[n] + n;  /* Use counter value in computation */
    }
}

__attribute__((hot))
static void process_data_nested_loops(int *arr, int len) {
    /* Pattern 4: Nested loop with decrementing inner counter */
    int outer_lim = len / 10;
    if (outer_lim < 1) outer_lim = 1;
    
    for (int j = 0; j < outer_lim; j++) {
        /* Inner loop with decrementing counter */
        for (int i = len - 1; i >= 0; i--) {
            arr[i] = arr[i] ^ (i + j);  /* Non-trivial operation */
        }
    }
}

__attribute__((hot))
static void process_data_mixed_patterns(int *arr, int len) {
    /* Use all patterns in one function to increase coverage chance */
    process_data_downward_for(arr, len);
    process_data_downward_neq(arr, len);
    process_data_while_decrement(arr, len);
    process_data_nested_loops(arr, len);
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
    int size = ARRAY_SIZE;
    unsigned int checksum = 0;
    
    /* Use command line argument for size if provided */
    if (argc > 1) {
        size = atoi(argv[1]);
        if (size <= 0) size = ARRAY_SIZE;
    }
    
    printf("Testing doloop optimization with array size: %d\n", size);
    
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
    
    /* Hot loop: call processing function many times */
    for (int iter = 0; iter < HOT_LOOP_COUNT; iter++) {
        /* Alternate between different patterns */
        if (iter % 4 == 0) {
            process_data_downward_for(data, size);
        } else if (iter % 4 == 1) {
            process_data_downward_neq(data, size);
        } else if (iter % 4 == 2) {
            process_data_while_decrement(data, size);
        } else {
            process_data_nested_loops(data, size);
        }
        
        /* Also call the mixed patterns function occasionally */
        if (iter % 10 == 0) {
            process_data_mixed_patterns(data, size);
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    checksum = compute_checksum(data, size);
    printf("Final checksum: %u\n", checksum);
    
    free(data);
    return 0;
}
