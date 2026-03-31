/* test_doloop.c
 * Designed to trigger GCC's doloop optimization validation logic
 * Compile with: gcc -O2 -fdoloop -funroll-loops -march=native -c test_doloop.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define HOT_LOOP_COUNT 10000
#define ARRAY_SIZE 1024

/* Mark function as hot to encourage loop optimization */
__attribute__((hot))
static void process_data(int *arr, int len) {
    int i;
    
    /* Pattern 1: Basic decrementing for loop with > condition */
    for (i = len - 1; i > 0; i--) {
        arr[i] = arr[i] * 3 + 7;
    }
    
    /* Pattern 2: Decrementing for loop with != condition */
    for (i = len - 1; i != 0; i--) {
        arr[i] = arr[i] + arr[i-1];
    }
    
    /* Pattern 3: While loop that decrements counter */
    while (len--) {
        arr[len] = (arr[len] << 1) | 1;
    }
}

/* Another hot function with nested loops */
__attribute__((hot))
static void process_matrix(int *matrix, int rows, int cols) {
    int i, j;
    
    /* Nested loop with inner decrementing counter */
    for (i = 0; i < rows; i++) {
        for (j = cols - 1; j >= 0; j--) {
            int idx = i * cols + j;
            matrix[idx] = matrix[idx] * 2 - 1;
        }
    }
}

/* Function with multiple loop patterns to increase coverage chance */
__attribute__((hot))
static void mixed_loops(unsigned int *data, unsigned int size) {
    unsigned int i;
    
    /* Pattern 4: Unsigned counter with != 0 condition */
    for (i = size; i != 0; i--) {
        data[i-1] = (data[i-1] * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Pattern 5: Separate counter variable */
    unsigned int count = size;
    while (count > 0) {
        data[count-1] ^= 0xAAAAAAAA;
        count--;
    }
}

int main(int argc, char **argv) {
    int *array1 = malloc(ARRAY_SIZE * sizeof(int));
    int *matrix = malloc(ARRAY_SIZE * sizeof(int));
    unsigned int *array2 = malloc(ARRAY_SIZE * sizeof(unsigned int));
    
    if (!array1 || !matrix || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 1000;
        matrix[i] = rand() % 1000;
        array2[i] = rand() % 1000;
    }
    
    /* Execute hot loops many times to encourage optimization */
    for (int iter = 0; iter < HOT_LOOP_COUNT; iter++) {
        process_data(array1, ARRAY_SIZE);
        process_matrix(matrix, 32, 32);  /* 32x32 matrix */
        mixed_loops(array2, ARRAY_SIZE);
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += array1[i];
        checksum += matrix[i];
        checksum += array2[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    free(array1);
    free(matrix);
    free(array2);
    
    return 0;
}
