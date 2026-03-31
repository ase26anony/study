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
        /* Non-trivial but simple operation to prevent removal */
        arr[i] = arr[i] * 3 + 7;
    }
    /* Handle element 0 separately to match i > 0 condition */
    if (len > 0) {
        arr[0] = arr[0] * 3 + 7;
    }
}

__attribute__((hot))
void process_data_downward_for_ne(int *arr, int len) {
    /* Pattern 2: Decrementing for loop with != condition */
    for (int i = len; i != 0; i--) {
        /* Different operation to avoid CSE between loops */
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
void process_data_nested_loops(int *arr, int len) {
    /* Pattern 4: Nested loop with decrementing inner counter */
    int outer = 10;
    while (outer--) {
        int inner = len;
        while (inner > 0) {
            arr[inner - 1] ^= 0x55;  /* Simple bit operation */
            inner--;
        }
    }
}

__attribute__((hot))
void process_data_mixed(unsigned *arr, unsigned len) {
    /* Pattern 5: Using unsigned counter (common in doloop) */
    for (unsigned i = len; i > 0; i--) {
        arr[i - 1] = (arr[i - 1] << 1) | (arr[i - 1] >> 31);
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
    
    /* Initialize with pseudo-random but deterministic values */
    srand(42);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data1[i] = rand() % 1000;
        data2[i] = rand() % 1000;
        data3[i] = rand() % 1000;
    }
    
    /* Execute hot loops many times to encourage optimization */
    clock_t start = clock();
    
    for (int iter = 0; iter < HOT_LOOP_COUNT; iter++) {
        /* Call different loop patterns to increase coverage chance */
        process_data_downward_for(data1, ARRAY_SIZE);
        process_data_downward_for_ne(data2, ARRAY_SIZE);
        process_data_while_decrement(data1, ARRAY_SIZE);
        process_data_nested_loops(data2, ARRAY_SIZE);
        process_data_mixed(data3, ARRAY_SIZE);
        
        /* Alternate array sizes slightly to prevent constant propagation */
        int alt_size = ARRAY_SIZE - (iter % 3);
        process_data_downward_for(data1, alt_size);
    }
    
    clock_t end = clock();
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum1 = 0, checksum2 = 0, checksum3 = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum1 += data1[i];
        checksum2 += data2[i];
        checksum3 += data3[i];
    }
    
    printf("Processing time: %.2f seconds\n", 
           (double)(end - start) / CLOCKS_PER_SEC);
    printf("Checksums: %lld, %lld, %lld\n", 
           checksum1, checksum2, checksum3);
    
    free(data1);
    free(data2);
    free(data3);
    
    return 0;
}
