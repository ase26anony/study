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
    /* Handle i=0 case separately to maintain > condition */
    if (len > 0) {
        arr[0] = arr[0] * 3 + 7;
    }
}

__attribute__((hot))
static void process_data_downward_for_ne(int *arr, int len) {
    /* Pattern 2: Decrementing for loop with != condition */
    for (int i = len; i != 0; i--) {
        /* Different computation to avoid CSE */
        arr[i - 1] = arr[i - 1] * 2 + 5;
    }
}

__attribute__((hot))
static void process_data_while_decrement(int *arr, int len) {
    /* Pattern 3: While loop that decrements counter */
    int n = len;
    while (n--) {
        /* Another distinct computation */
        arr[n] = arr[n] + arr[n] * 4;
    }
}

__attribute__((hot))
static void process_data_nested_loops(int *arr, int len) {
    /* Pattern 4: Nested loops with inner decrementing counter */
    int outer_lim = len / 10;
    if (outer_lim < 1) outer_lim = 1;
    
    for (int j = 0; j < outer_lim; j++) {
        /* Inner loop with decrementing counter */
        for (int i = len - 1; i >= 0; i--) {
            /* Mix index with outer counter to prevent hoisting */
            arr[i] = arr[i] + j;
        }
    }
}

__attribute__((hot))
static void process_data_mixed_patterns(int *arr, int len) {
    /* Use all patterns in sequence */
    process_data_downward_for(arr, len);
    process_data_downward_for_ne(arr, len);
    process_data_while_decrement(arr, len);
    process_data_nested_loops(arr, len);
}

/* Compute checksum to prevent dead code elimination */
static int compute_checksum(int *arr, int len) {
    int sum = 0;
    for (int i = 0; i < len; i++) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char **argv) {
    int *data;
    int size = ARRAY_SIZE;
    int checksum = 0;
    clock_t start, end;
    
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
    
    /* Initialize with non-zero values */
    for (int i = 0; i < size; i++) {
        data[i] = (i * 3 + 7) % 100;
    }
    
    start = clock();
    
    /* Hot loop to make the functions "hot" for the optimizer */
    for (int iter = 0; iter < HOT_LOOP_COUNT; iter++) {
        /* Alternate between different loop patterns */
        if (iter % 4 == 0) {
            process_data_downward_for(data, size);
        } else if (iter % 4 == 1) {
            process_data_downward_for_ne(data, size);
        } else if (iter % 4 == 2) {
            process_data_while_decrement(data, size);
        } else {
            process_data_mixed_patterns(data, size);
        }
        
        /* Occasionally recompute to prevent monotonic overflow */
        if (iter % 100 == 0) {
            for (int i = 0; i < size; i++) {
                data[i] = data[i] % 1000;
            }
        }
    }
    
    end = clock();
    
    checksum = compute_checksum(data, size);
    
    printf("Checksum: %d\n", checksum);
    printf("Time elapsed: %.2f seconds\n", 
           (double)(end - start) / CLOCKS_PER_SEC);
    
    free(data);
    return 0;
}
