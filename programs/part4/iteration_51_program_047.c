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
        /* Non-trivial but simple operation to prevent removal */
        arr[i] = arr[i] * 3 + 7;
    }
    
    /* Handle i = 0 case separately to avoid zero comparison in loop */
    if (len > 0) {
        arr[0] = arr[0] * 3 + 7;
    }
}

__attribute__((hot))
static void process_data_downward_neq(int *arr, int len) {
    /* Pattern 2: Decrementing for loop with != condition */
    for (int i = len; i != 0; i--) {
        /* Different operation to avoid merging with other loops */
        arr[i-1] = arr[i-1] * 2 + 1;
    }
}

__attribute__((hot))
static void process_data_while_decrement(int *arr, int len) {
    /* Pattern 3: While loop that decrements counter */
    int n = len;
    while (n--) {
        /* Another distinct operation */
        arr[n] = arr[n] + n;
    }
}

__attribute__((hot))
static void process_data_nested(int *arr, int len) {
    /* Pattern 4: Nested loops with inner decrementing counter */
    int outer = 10;
    while (outer--) {
        for (int i = len - 1; i >= 0; i--) {
            /* Complex enough to prevent unrolling */
            arr[i] = (arr[i] * arr[(i + 1) % len]) / 2;
        }
    }
}

__attribute__((hot))
static void process_data_mixed(int *arr, int len) {
    /* Use all patterns in sequence */
    process_data_downward_for(arr, len);
    process_data_downward_neq(arr, len);
    process_data_while_decrement(arr, len);
    process_data_nested(arr, len);
}

static int compute_checksum(int *arr, int len) {
    int sum = 0;
    for (int i = 0; i < len; i++) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    int *data;
    int size = ARRAY_SIZE;
    int i, j;
    
    /* Use command line argument for size if provided */
    if (argc > 1) {
        size = atoi(argv[1]);
        if (size <= 0) size = ARRAY_SIZE;
    }
    
    printf("Testing doloop optimization with array size %d\n", size);
    
    /* Allocate and initialize array */
    data = (int *)malloc(size * sizeof(int));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (i = 0; i < size; i++) {
        data[i] = rand() % 100;
    }
    
    /* Run hot loops many times to encourage optimization */
    clock_t start = clock();
    
    for (j = 0; j < HOT_LOOP_COUNT; j++) {
        /* Call the mixed processing function */
        process_data_mixed(data, size);
        
        /* Alternate between different loop patterns */
        if (j % 3 == 0) {
            process_data_downward_for(data, size);
        } else if (j % 3 == 1) {
            process_data_downward_neq(data, size);
        } else {
            process_data_while_decrement(data, size);
        }
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    /* Compute and print checksum to prevent dead code elimination */
    int checksum = compute_checksum(data, size);
    printf("Checksum: %d\n", checksum);
    printf("Elapsed time: %.3f seconds\n", elapsed);
    
    free(data);
    return 0;
}
