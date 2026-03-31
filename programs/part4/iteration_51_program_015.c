/* test_doloop.c
 * Designed to trigger GCC's doloop optimization validation logic
 * Compile with: gcc -O2 -fdoloop -funroll-loops -c test_doloop.c
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
        arr[i] = arr[i] * 3 + 7;
        sum += arr[i];  /* Prevent dead code elimination */
    }
    
    /* Pattern 2: Decrementing for loop with != condition */
    for (i = len - 2; i != 0; i--) {
        arr[i] = arr[i] / 2 + arr[i + 1];
        sum += arr[i];
    }
    
    /* Pattern 3: While loop that decrements counter */
    i = len - 3;
    while (i) {
        arr[i] = arr[i] ^ 0x55;  /* Simple non-trivial operation */
        sum += arr[i];
        i--;
    }
    
    /* Pattern 4: Another variant with unsigned counter */
    unsigned int u;
    for (u = len - 4; u > 0; u--) {
        arr[u] = arr[u] + sum;
    }
    
    /* Use sum to prevent optimization */
    arr[0] = sum;
}

/* Secondary hot function with nested loops */
__attribute__((hot))
static void process_matrix(int *matrix, int rows, int cols) {
    int i, j;
    
    /* Nested loop with inner decrementing counter */
    for (i = 0; i < rows; i++) {
        for (j = cols - 1; j > 0; j--) {
            int idx = i * cols + j;
            matrix[idx] = matrix[idx] * 2 - matrix[idx - 1];
        }
    }
}

int main(int argc, char *argv[]) {
    int *data;
    int *matrix;
    int i, result = 0;
    int data_size = ARRAY_SIZE;
    int matrix_rows = 100;
    int matrix_cols = 100;
    
    /* Use command line argument for variable size if provided */
    if (argc > 1) {
        data_size = atoi(argv[1]);
        if (data_size < 100) data_size = 100;
    }
    
    /* Allocate and initialize data */
    data = (int *)malloc(data_size * sizeof(int));
    matrix = (int *)malloc(matrix_rows * matrix_cols * sizeof(int));
    
    if (!data || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random but deterministic values */
    srand(42);
    for (i = 0; i < data_size; i++) {
        data[i] = rand() % 100;
    }
    for (i = 0; i < matrix_rows * matrix_cols; i++) {
        matrix[i] = rand() % 100;
    }
    
    /* Hot loop: call processing functions many times */
    for (i = 0; i < HOT_LOOP_COUNT; i++) {
        process_data(data, data_size);
        process_matrix(matrix, matrix_rows, matrix_cols);
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (i = 0; i < data_size; i++) {
        result ^= data[i];
    }
    for (i = 0; i < matrix_rows * matrix_cols; i++) {
        result ^= matrix[i];
    }
    
    printf("Result checksum: %d\n", result);
    
    free(data);
    free(matrix);
    return 0;
}
