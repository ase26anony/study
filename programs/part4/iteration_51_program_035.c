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
        arr[i] = arr[i] * 3 + 7;
    }
    /* Ensure last element processed */
    arr[0] = arr[0] * 3 + 7;
}

__attribute__((hot))
static void process_data_downward_for_ne(int *arr, int len) {
    /* Pattern 2: Decrementing for loop with != condition */
    for (int i = len - 1; i != 0; i--) {
        arr[i] = arr[i] * 2 + 1;
    }
    arr[0] = arr[0] * 2 + 1;
}

__attribute__((hot))
static void process_data_while_decrement(int *arr, int len) {
    /* Pattern 3: While loop with decrement */
    int n = len;
    while (n--) {
        arr[n] = arr[n] * 5 - 3;
    }
}

__attribute__((hot))
static void process_data_nested_loops(int *arr, int len) {
    /* Pattern 4: Nested loops with inner decrementing counter */
    int outer_iters = 10;
    
    for (int outer = 0; outer < outer_iters; outer++) {
        /* Inner loop with decrementing counter */
        for (int i = len - 1; i >= 0; i--) {
            arr[i] += outer;
        }
    }
}

__attribute__((hot))
static void process_data_mixed_patterns(int *arr, int len) {
    /* Use multiple patterns in sequence */
    process_data_downward_for(arr, len);
    process_data_downward_for_ne(arr, len);
    process_data_while_decrement(arr, len);
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
    int array_size = ARRAY_SIZE;
    int checksum = 0;
    
    /* Use runtime-determined size to prevent compile-time unrolling */
    if (argc > 1) {
        array_size = atoi(argv[1]);
        if (array_size < 100) array_size = 100;
        if (array_size > 100000) array_size = 100000;
    }
    
    data = (int *)malloc(array_size * sizeof(int));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize array with pseudo-random values */
    srand(time(NULL));
    for (int i = 0; i < array_size; i++) {
        data[i] = rand() % 100;
    }
    
    /* Hot loop to make the functions "hot" for the optimizer */
    for (int iter = 0; iter < HOT_LOOP_COUNT; iter++) {
        /* Call different loop patterns to increase coverage chance */
        if (iter % 4 == 0) {
            process_data_downward_for(data, array_size);
        } else if (iter % 4 == 1) {
            process_data_downward_for_ne(data, array_size);
        } else if (iter % 4 == 2) {
            process_data_while_decrement(data, array_size);
        } else {
            process_data_nested_loops(data, array_size);
        }
        
        /* Also call the mixed patterns function occasionally */
        if (iter % 10 == 0) {
            process_data_mixed_patterns(data, array_size);
        }
    }
    
    checksum = compute_checksum(data, array_size);
    printf("Final checksum: %d\n", checksum);
    
    free(data);
    return 0;
}
