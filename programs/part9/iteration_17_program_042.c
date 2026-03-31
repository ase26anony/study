/* 
 * Complex loop nesting test program targeting hw-doloop.cc coverage
 * Specifically designed to exercise bitmap intersection logic for loop nesting analysis
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define SIZE 1024
#define ITERATIONS 100

/* Force inlining to create complex CFG within loops */
__attribute__((always_inline)) 
static inline int process_chunk(int *restrict dst, const int *restrict src, int start, int end) {
    int sum = 0;
    /* Loop with multiple exit points */
    for (int i = start; i < end; i += (i % 3) + 1) {
        if (src[i] > 0) {
            dst[i] = src[i] * 2;
            sum += dst[i];
            if (dst[i] > 1000) break; /* Early exit creates separate block */
        } else {
            dst[i] = -src[i];
            sum -= dst[i];
            /* Continue jumps to different points based on condition */
            if (i % 5 == 0) continue;
        }
        /* Additional block that may or may not be executed */
        dst[i] += i;
    }
    return sum;
}

/* Function with nested loops that will be inlined */
__attribute__((always_inline))
static inline void nested_matrix_process(int A[SIZE][SIZE], int B[SIZE][SIZE], int C[SIZE][SIZE]) {
    /* Outer loop with complex increment */
    for (int i = 0; i < SIZE; i += (i % 4) + 1) {
        /* First inner loop - perfectly nested */
        for (int j = 0; j < SIZE; j++) {
            C[i][j] = 0;
            /* Innermost loop with conditional continue */
            for (int k = 0; k < SIZE; k++) {
                if (k % 2 == 0) {
                    C[i][j] += A[i][k] * B[k][j];
                    if (C[i][j] < 0) continue; /* Skip to next iteration */
                } else {
                    C[i][j] -= A[i][k] * B[k][j];
                }
                /* Additional computation block */
                C[i][j] = C[i][j] % 10007;
            }
        }
        
        /* Second inner loop at same level but with different structure */
        int j = 0;
        while (j < SIZE) {
            /* Switch statement creating multiple basic blocks */
            switch (i % 4) {
                case 0:
                    A[i][j] += B[i][j];
                    j += 2;
                    break;
                case 1:
                    A[i][j] -= B[i][j];
                    j += 3;
                    break;
                case 2:
                    A[i][j] *= B[i][j];
                    j += 1;
                    /* Fall through creates shared block */
                default:
                    A[i][j] /= 2;
                    j += (j % 7) + 1;
            }
            if (A[i][j] > 1000) goto cleanup; /* Non-local exit */
        }
        cleanup:
        /* Empty cleanup block - creates separate basic block */
        ;
    }
}

/* Function with partially overlapping loops */
static void overlapping_loops(int arr[SIZE], int pattern[SIZE]) {
    int i = 0, j = 0;
    
    /* First loop with complex condition */
    while (i < SIZE && j < SIZE) {
        /* if-else chain creating divergent paths */
        if (arr[i] == pattern[j]) {
            arr[i] = pattern[j] * 3;
            i++; j++;
        } else if (arr[i] < pattern[j]) {
            arr[i] += pattern[j];
            i++;
            /* Nested do-while with early exit */
            do {
                if (i >= SIZE) break;
                arr[i] = arr[i-1] + 1;
                i++;
            } while (i % 10 != 0);
        } else {
            pattern[j] = arr[i] / 2;
            j++;
        }
        
        /* Second loop starting in middle of first loop's body */
        for (int k = 0; k < (i % 20); k++) {
            arr[k] ^= pattern[k]; /* XOR operation */
            if (k % 3 == 0) continue;
            arr[k] |= 0xFF; /* Bitwise OR */
        }
    }
    
    /* Infinite loop with conditional breaks at different depths */
    for (;;) {
        int idx = 0;
        while (idx < SIZE) {
            arr[idx] = (arr[idx] * 1103515245 + 12345) & 0x7FFFFFFF;
            if (arr[idx] % 100 == 0) {
                /* Break from inner loop only */
                break;
            }
            idx += (arr[idx] % 5) + 1;
            if (idx > SIZE/2) {
                /* Break from both loops */
                goto exit_infinite;
            }
        }
        /* This block is sometimes skipped */
        arr[0] = (arr[0] + arr[SIZE-1]) / 2;
    }
    exit_infinite: ;
}

/* Recursive function creating loop-like structure */
static int recursive_loop(int *arr, int start, int end, int depth) {
    if (start >= end || depth <= 0) return 0;
    
    int mid = (start + end) / 2;
    int sum = 0;
    
    /* Process left half */
    for (int i = start; i < mid; i++) {
        arr[i] = (arr[i] * depth) % 1009;
        sum += arr[i];
        if (arr[i] < 0) {
            /* Early return creates separate exit block */
            return sum;
        }
    }
    
    /* Recursive calls create complex nesting */
    sum += recursive_loop(arr, start, mid, depth - 1);
    sum += recursive_loop(arr, mid, end, depth - 1);
    
    /* Process right half with while loop */
    int j = mid;
    while (j < end) {
        arr[j] = (arr[j] + sum) & 0xFFF;
        j += (arr[j] % 7) + 1;
    }
    
    return sum;
}

/* Test function with sibling loops */
static void sibling_loops(int data[SIZE][SIZE]) {
    /* First sibling loop */
    #pragma GCC unroll 4
    for (int i = 0; i < SIZE; i++) {
        if (i % 2 == 0) {
            for (int j = 0; j < SIZE; j += 2) {
                data[i][j] = data[i][j] * 2 + 1;
                /* Complex condition with short-circuit evaluation */
                if (j > 0 && data[i][j-1] < data[i][j]) {
                    data[i][j-1] = data[i][j];
                }
            }
        }
    }
    
    /* Second sibling loop at same level, sharing no blocks with first */
    for (int i = 0; i < SIZE; i++) {
        if (i % 2 == 1) {
            int j = SIZE - 1;
            do {
                data[i][j] = data[i][j] / 3 - 5;
                j -= (data[i][j] % 4) + 1;
            } while (j >= 0);
        }
    }
    
    /* Third loop overlapping with both siblings */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            /* Shared computation block */
            data[i][j] = (data[i][j] + i * j) % 997;
        }
    }
}

/* Main test driver */
int main() {
    /* Initialize data structures */
    int *array1 = (int*)malloc(SIZE * sizeof(int));
    int *array2 = (int*)malloc(SIZE * sizeof(int));
    int (*matrixA)[SIZE] = (int(*)[SIZE])malloc(SIZE * SIZE * sizeof(int));
    int (*matrixB)[SIZE] = (int(*)[SIZE])malloc(SIZE * SIZE * sizeof(int));
    int (*matrixC)[SIZE] = (int(*)[SIZE])malloc(SIZE * SIZE * sizeof(int));
    int (*matrixD)[SIZE] = (int(*)[SIZE])malloc(SIZE * SIZE * sizeof(int));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (i * 17) % 7919;
        array2[i] = (i * 23) % 7919;
        for (int j = 0; j < SIZE; j++) {
            matrixA[i][j] = (i * j * 31) % 7919;
            matrixB[i][j] = (i * j * 37) % 7919;
            matrixD[i][j] = (i * j * 41) % 7919;
        }
    }
    
    int total_sum = 0;
    
    /* Run multiple iterations to ensure loops execute */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Test 1: Inlined function with nested loops */
        total_sum += process_chunk(array1, array2, 0, SIZE);
        
        /* Test 2: Complex matrix processing with perfect nesting */
        nested_matrix_process(matrixA, matrixB, matrixC);
        total_sum += matrixC[SIZE-1][SIZE-1];
        
        /* Test 3: Partially overlapping loops */
        overlapping_loops(array1, array2);
        total_sum += array1[SIZE/2] + array2[SIZE/2];
        
        /* Test 4: Recursive loop-like structure */
        total_sum += recursive_loop(array1, 0, SIZE, 5);
        
        /* Test 5: Sibling loops */
        sibling_loops(matrixD);
        total_sum += matrixD[0][0] + matrixD[SIZE-1][SIZE-1];
        
        /* Use __builtin_expect for loop optimization hints */
        if (__builtin_expect(total_sum > 1000000, 0)) {
            total_sum %= 1000000;
        }
    }
    
    printf("Final checksum: %d\n", total_sum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(matrixA);
    free(matrixB);
    free(matrixC);
    free(matrixD);
    
    return 0;
}
