/* test_doloop.c
 * Designed to trigger doloop optimization validation in loop-doloop.cc
 * Compile with: gcc -O2 -funroll-loops -fdoloop -c test_doloop.c
 * Or for ARM: gcc -O2 -fdoloop -march=armv8-a -c test_doloop.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define HOT_LOOP_COUNT 1000
#define ARRAY_SIZE 10000

/* Mark function as hot to encourage loop optimization */
__attribute__((hot))
static void process_data(int *arr, int len) {
    int i;
    int sum = 0;
    
    /* Pattern 1: Basic decrementing for loop with > condition */
    for (i = len - 1; i > 0; i--) {
        arr[i] = arr[i] * 2 + 1;
        sum += arr[i];
    }
    
    /* Pattern 2: Decrementing for loop with != condition */
    for (i = len - 1; i != 0; i--) {
        arr[i] = arr[i] * 3 - 2;
        sum += arr[i];
    }
    
    /* Pattern 3: While loop that decrements counter */
    while (len--) {
        arr[len] = arr[len] + sum;
        /* Non-trivial but safe operation */
        if (arr[len] > 1000000) {
            arr[len] = 1000000;
        }
    }
    
    /* Prevent dead code elimination */
    arr[0] = sum;
}

/* Another hot function with nested loops */
__attribute__((hot))
static void process_matrix(int *matrix, int rows, int cols) {
    int i, j;
    
    /* Nested loop with inner decrementing counter */
    for (i = 0; i < rows; i++) {
        for (j = cols - 1; j >= 0; j--) {
            int idx = i * cols + j;
            matrix[idx] = (matrix[idx] * 7) >> 2;
            /* Non-trivial operation to prevent optimization */
            matrix[idx] ^= (i * j) & 0xFF;
        }
    }
}

/* Function with unsigned counter (common in doloop patterns) */
__attribute__((hot))
static void process_unsigned(unsigned *arr, unsigned len) {
    unsigned i;
    
    /* Unsigned decrementing loop */
    for (i = len; i > 0; i--) {
        arr[i-1] = arr[i-1] * 3;
        /* Prevent overflow check elimination */
        if (arr[i-1] < 100) {
            arr[i-1] = 100;
        }
    }
}

int main(int argc, char **argv) {
    int *data = malloc(ARRAY_SIZE * sizeof(int));
    int *matrix = malloc(ARRAY_SIZE * sizeof(int));
    unsigned *udata = malloc(ARRAY_SIZE * sizeof(unsigned));
    int i, j;
    clock_t start, end;
    
    if (!data || !matrix || !udata) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-zero values */
    srand(time(NULL));
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = rand() % 1000;
        matrix[i] = rand() % 1000;
        udata[i] = rand() % 1000;
    }
    
    start = clock();
    
    /* Hot loop to make the functions "hot" for the optimizer */
    for (j = 0; j < HOT_LOOP_COUNT; j++) {
        process_data(data, ARRAY_SIZE);
        process_matrix(matrix, 100, 100);  /* 100x100 matrix */
        process_unsigned(udata, ARRAY_SIZE);
        
        /* Vary array sizes slightly to prevent constant propagation */
        int size_mod = ARRAY_SIZE - (j % 10);
        if (size_mod > 10) {
            process_data(data, size_mod);
        }
    }
    
    end = clock();
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum += data[i];
        checksum += matrix[i];
        checksum += udata[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    printf("Time: %.2f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    
    free(data);
    free(matrix);
    free(udata);
    
    return 0;
}
