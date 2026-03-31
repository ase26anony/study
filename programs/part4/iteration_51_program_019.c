/* test_doloop.c - Program to trigger doloop optimization validation */
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
        arr[i] = arr[i] * 3 + 7;
    }
    /* Handle element 0 separately to avoid i >= 0 */
    arr[0] = arr[0] * 3 + 7;
}

__attribute__((hot))
void process_data_downward_for_ne(int *arr, int len) {
    /* Pattern 2: Decrementing for loop with != condition */
    for (int i = len; i != 0; i--) {
        arr[i - 1] = arr[i - 1] * 2 + 1;
    }
}

__attribute__((hot))
void process_data_while_decrement(int *arr, int len) {
    /* Pattern 3: While loop that decrements counter */
    int n = len;
    while (n--) {
        arr[n] = arr[n] * 5 - 3;
    }
}

__attribute__((hot))
void process_data_nested_loops(int *arr, int len) {
    /* Pattern 4: Nested loops with inner decrementing counter */
    int outer = 10;
    while (outer--) {
        for (int i = len - 1; i > 0; i--) {
            arr[i] += outer;
        }
        arr[0] += outer;
    }
}

__attribute__((hot))
void process_data_mixed_patterns(int *arr, int len) {
    /* Use multiple patterns in one function */
    int temp = len;
    
    /* First pattern */
    while (temp > 0) {
        arr[temp - 1] = arr[temp - 1] ^ 0xFF;
        temp--;
    }
    
    /* Reset and use second pattern */
    temp = len;
    do {
        arr[len - temp] = arr[len - temp] | 0xAA;
    } while (--temp);
}

int main(int argc, char *argv[]) {
    int *array;
    int size = ARRAY_SIZE;
    int i, j;
    unsigned long long checksum = 0;
    
    /* Use command line argument for size if provided */
    if (argc > 1) {
        size = atoi(argv[1]);
        if (size <= 0) size = ARRAY_SIZE;
    }
    
    /* Allocate and initialize array */
    array = (int *)malloc(size * sizeof(int));
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (i = 0; i < size; i++) {
        array[i] = rand() % 1000;
    }
    
    printf("Processing %d elements %d times...\n", size, HOT_LOOP_COUNT);
    
    /* Repeatedly call hot functions to make loops "hot" */
    for (j = 0; j < HOT_LOOP_COUNT; j++) {
        process_data_downward_for(array, size);
        process_data_downward_for_ne(array, size);
        process_data_while_decrement(array, size);
        process_data_nested_loops(array, size);
        process_data_mixed_patterns(array, size);
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (i = 0; i < size; i++) {
        checksum += array[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    free(array);
    return 0;
}
