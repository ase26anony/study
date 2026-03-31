/* test_doloop.c - Program to trigger doloop optimization validation logic */

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
        /* Non-trivial operation to prevent removal */
        arr[i] = arr[i] * 3 + 7;
    }
    /* Handle element 0 separately to avoid i >= 0 comparison */
    if (len > 0) {
        arr[0] = arr[0] * 3 + 7;
    }
}

__attribute__((hot))
void process_data_downward_neq(int *arr, int len) {
    /* Pattern 2: Decrementing for loop with != condition */
    for (int i = len; i != 0; i--) {
        /* Different operation to avoid CSE */
        arr[i - 1] = arr[i - 1] * 2 + 5;
    }
}

__attribute__((hot))
void process_data_while_decrement(int *arr, int len) {
    /* Pattern 3: While loop that decrements counter */
    int n = len;
    while (n--) {
        /* Another distinct operation */
        arr[n] = arr[n] + arr[n] / 2;
    }
}

__attribute__((hot))
void process_data_nested(int *arr, int len) {
    /* Pattern 4: Nested loops with inner decrementing counter */
    int outer = 10;
    while (outer--) {
        for (int i = len - 1; i > 0; i--) {
            /* Complex enough to prevent unrolling */
            arr[i] = (arr[i] << 1) | (arr[i] >> 31);
        }
        if (len > 0) {
            arr[0] = (arr[0] << 1) | (arr[0] >> 31);
        }
    }
}

__attribute__((hot))
void process_data_mixed(unsigned *arr, unsigned len) {
    /* Pattern 5: Using unsigned counter (common in doloop) */
    for (unsigned i = len; i > 0; i--) {
        arr[i - 1] = arr[i - 1] ^ 0xAAAAAAAA;
    }
}

int main(int argc, char *argv[]) {
    int *data1 = malloc(ARRAY_SIZE * sizeof(int));
    int *data2 = malloc(ARRAY_SIZE * sizeof(int));
    unsigned *data3 = malloc(ARRAY_SIZE * sizeof(unsigned));
    
    if (!data1 || !data2 || !data3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data1[i] = rand() % 1000;
        data2[i] = rand() % 1000;
        data3[i] = rand() % 1000;
    }
    
    /* Execute hot loops many times to encourage optimization */
    clock_t start = clock();
    
    for (int iter = 0; iter < HOT_LOOP_COUNT; iter++) {
        /* Vary the length slightly to prevent constant propagation */
        int len = ARRAY_SIZE - (iter % 10);
        
        /* Call different loop patterns */
        process_data_downward_for(data1, len);
        process_data_downward_neq(data2, len);
        process_data_while_decrement(data1, len);
        process_data_nested(data2, len);
        process_data_mixed(data3, len);
    }
    
    clock_t end = clock();
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum1 = 0, checksum2 = 0, checksum3 = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum1 += data1[i];
        checksum2 += data2[i];
        checksum3 += data3[i];
    }
    
    printf("Execution time: %.2f seconds\n", 
           (double)(end - start) / CLOCKS_PER_SEC);
    printf("Checksums: %lld %lld %lld\n", 
           checksum1, checksum2, checksum3);
    
    free(data1);
    free(data2);
    free(data3);
    
    return 0;
}
