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
    /* Handle i=0 case separately to avoid zero comparison in loop */
    if (len > 0) {
        arr[0] = arr[0] * 3 + 7;
    }
}

__attribute__((hot))
static void process_data_downward_neq(int *arr, int len) {
    /* Pattern 2: Decrementing for loop with != condition */
    for (int i = len; i != 0; i--) {
        /* Different operation to prevent merging with other loops */
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
    /* Pattern 4: Nested loop with inner decrementing counter */
    int outer = 10;
    while (outer--) {
        int inner = len;
        while (inner > 0) {
            arr[inner-1] = arr[inner-1] ^ outer;
            inner--;
        }
    }
}

__attribute__((hot))
static void process_data_mixed(unsigned *arr, unsigned len) {
    /* Pattern 5: Using unsigned counter (common in doloop) */
    for (unsigned i = len; i > 0; i--) {
        arr[i-1] = (arr[i-1] << 1) | 1;
    }
}

int main(int argc, char *argv[]) {
    int *array1 = malloc(ARRAY_SIZE * sizeof(int));
    int *array2 = malloc(ARRAY_SIZE * sizeof(int));
    unsigned *array3 = malloc(ARRAY_SIZE * sizeof(unsigned));
    
    if (!array1 || !array2 || !array3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pseudo-random data */
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
        array3[i] = rand() % 1000;
    }
    
    /* Execute hot loops many times to encourage optimization */
    for (int iter = 0; iter < HOT_LOOP_COUNT; iter++) {
        process_data_downward_for(array1, ARRAY_SIZE);
        process_data_downward_neq(array2, ARRAY_SIZE);
        process_data_while_decrement(array1, ARRAY_SIZE / 2);
        process_data_nested(array2, ARRAY_SIZE / 4);
        process_data_mixed(array3, ARRAY_SIZE);
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += array1[i] + array2[i] + array3[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    free(array1);
    free(array2);
    free(array3);
    
    return 0;
}
