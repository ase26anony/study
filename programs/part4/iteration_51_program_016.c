/* test_doloop.c
 * Designed to trigger GCC's doloop optimization validation logic
 * Specifically targets lines 136-150 in loop-doloop.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define HOT_LOOP_COUNT 1000
#define ARRAY_SIZE 10000

/* Mark function as hot to encourage loop optimization */
__attribute__((hot))
void process_data(int *arr, int len) {
    int i;
    int sum = 0;
    
    /* Pattern 1: Basic decrementing for loop with > condition */
    for (i = len - 1; i > 0; i--) {
        /* Non-trivial operation to prevent removal */
        arr[i] = arr[i] * 3 + 7;
        sum += arr[i];
    }
    
    /* Pattern 2: Decrementing for loop with != condition */
    for (i = len - 1; i != 0; i--) {
        arr[i] = arr[i] / 2 + 1;
        sum += arr[i];
    }
    
    /* Pattern 3: While loop with decrement */
    i = len;
    while (i--) {
        arr[i] = arr[i] + i;
        sum += arr[i];
    }
    
    /* Use sum to prevent dead code elimination */
    arr[0] = sum % 1000;
}

/* Another hot function with nested loops */
__attribute__((hot))
void process_matrix(int *matrix, int rows, int cols) {
    int i, j;
    
    /* Pattern 4: Nested loop with decrementing inner counter */
    for (i = rows - 1; i >= 0; i--) {
        for (j = cols - 1; j > 0; j--) {
            int idx = i * cols + j;
            matrix[idx] = matrix[idx] * 2 - 1;
        }
    }
    
    /* Pattern 5: Mixed decrement patterns */
    i = rows;
    while (i--) {
        j = cols;
        while (j--) {
            int idx = i * cols + j;
            matrix[idx] += (i + j);
        }
    }
}

int main(int argc, char *argv[]) {
    int *data = NULL;
    int *matrix = NULL;
    int data_size = ARRAY_SIZE;
    int matrix_rows = 100;
    int matrix_cols = 100;
    int i;
    clock_t start, end;
    double cpu_time_used;
    
    /* Use command line argument for variable size if provided */
    if (argc > 1) {
        data_size = atoi(argv[1]);
        if (data_size <= 0) data_size = ARRAY_SIZE;
    }
    
    printf("Testing doloop optimization with array size: %d\n", data_size);
    
    /* Allocate and initialize data */
    data = (int *)malloc(data_size * sizeof(int));
    if (!data) {
        perror("malloc failed for data");
        return 1;
    }
    
    matrix = (int *)malloc(matrix_rows * matrix_cols * sizeof(int));
    if (!matrix) {
        perror("malloc failed for matrix");
        free(data);
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (i = 0; i < data_size; i++) {
        data[i] = rand() % 100;
    }
    for (i = 0; i < matrix_rows * matrix_cols; i++) {
        matrix[i] = rand() % 100;
    }
    
    start = clock();
    
    /* Execute hot loops many times to encourage optimization */
    for (i = 0; i < HOT_LOOP_COUNT; i++) {
        process_data(data, data_size);
        process_matrix(matrix, matrix_rows, matrix_cols);
    }
    
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (i = 0; i < data_size; i++) {
        checksum = (checksum + data[i]) % 1000000;
    }
    for (i = 0; i < matrix_rows * matrix_cols; i++) {
        checksum = (checksum + matrix[i]) % 1000000;
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Execution time: %f seconds\n", cpu_time_used);
    
    free(data);
    free(matrix);
    
    return 0;
}
