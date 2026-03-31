/* 
 * Complex loop nesting test program targeting hw-doloop.cc uncovered lines
 * Lines 429-436: bitmap intersection logic for loop containment analysis
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define SIZE 1024
#define ITERS 100

/* Force inlining to create complex CFG within loops */
__attribute__((always_inline)) 
static inline int process_chunk(int *restrict dst, const int *restrict src, 
                                int start, int end, int threshold) {
    int sum = 0;
    /* Loop with multiple exits and complex condition */
    for (int i = start; i < end; i += (i % 3) + 1) {
        if (src[i] > threshold) {
            dst[i] = src[i] * 2;
            sum += dst[i];
            if (sum > 10000) break; /* Early exit */
        } else if (src[i] < -threshold) {
            dst[i] = src[i] / 2;
            sum -= dst[i];
            continue; /* Skip rest of iteration */
        } else {
            dst[i] = src[i];
            /* Nested switch creating multiple basic blocks */
            switch (i % 4) {
                case 0: dst[i] += 1; break;
                case 1: dst[i] -= 1; break;
                case 2: dst[i] *= 2; break;
                case 3: dst[i] /= 2; break;
            }
        }
        /* Additional condition splitting the block */
        sum += (dst[i] % 2 == 0) ? dst[i] : -dst[i];
    }
    return sum;
}

/* Helper with tail recursion creating loop-like structure */
__attribute__((noinline))
static int recursive_processor(int *arr, int idx, int depth, int limit) {
    if (idx >= limit || depth <= 0) return 0;
    
    int result = arr[idx];
    /* Partial overlap: shares some blocks with caller's loops */
    if (arr[idx] > 0) {
        result += recursive_processor(arr, idx + 1, depth - 1, limit);
    } else {
        result -= recursive_processor(arr, idx + 2, depth - 1, limit);
    }
    
    /* Small loop inside recursive function */
    for (int i = 0; i < depth && i < 5; ++i) {
        result += arr[idx + i] % 256;
    }
    
    return result;
}

/* Function containing sibling loops (same level, different blocks) */
static void sibling_loops_test(int *a, int *b, int n) {
    int i, j;
    
    /* First sibling loop */
    for (i = 0; i < n; ++i) {
        if (i % 2 == 0) {
            a[i] = b[i] * 2;
            goto skip_odd; /* Creates alternate path */
        }
        a[i] = b[i] + 1;
    skip_odd:
        if (i % 3 == 0) continue;
        a[i] += i;
    }
    
    /* Second sibling loop - shares no blocks with first */
    for (j = n - 1; j >= 0; --j) {
        b[j] = a[j] - j;
        /* Nested infinite loop with conditional break */
        for (;;) {
            if (b[j] < 0) {
                b[j] = -b[j];
                break;
            }
            if (b[j] > 1000) {
                b[j] %= 1000;
                break;
            }
            b[j] >>= 1;
            if (b[j] == 0) break;
        }
    }
}

/* Complex nested loops with partial overlap */
static void overlapping_loops_test(int matrix[SIZE][SIZE]) {
    int i, j, k;
    
    /* Outer loop */
    for (i = 0; i < SIZE - 1; ++i) {
        /* First inner loop - perfectly nested */
        #pragma GCC unroll 2
        for (j = 0; j < SIZE; ++j) {
            matrix[i][j] = i * j;
            if (matrix[i][j] % 7 == 0) {
                matrix[i][j] += 1;
            }
        }
        
        /* Second inner loop - partially overlapping with first */
        for (k = 0; k < SIZE; k += 2) {
            /* Conditional creating divergent paths */
            if (__builtin_expect((i + k) % 11 == 0, 0)) {
                matrix[i][k] *= 2;
                goto matrix_update;
            }
            matrix[i][k] += matrix[i][k + 1];
        matrix_update:
            matrix[i][k] %= 256;
        }
        
        /* Third loop at same level as first two but different structure */
        j = 0;
        while (j < SIZE) {
            matrix[i][j] -= i;
            if (matrix[i][j] < 0 && j % 3 == 0) {
                matrix[i][j] = recursive_processor(matrix[i], j, 3, SIZE);
            }
            j += (i % 4) + 1; /* Variable increment */
        }
    }
}

/* Test function with multiple loop types interacting */
static int mixed_loops_test(int *arr, int n) {
    int total = 0;
    int i = 0;
    
    /* do-while with nested if creating early exits */
    do {
        if (arr[i] == 0) {
            /* Process chunk creates its own loop structure */
            total += process_chunk(arr, arr, i, i + 10, 50);
            i += 10;
            continue;
        }
        
        int j = 0;
        /* while loop with multiple conditions */
        while (j < n && arr[j] != 0 && total < 1000000) {
            total += arr[j];
            /* Nested for with complex increment */
            for (int k = j; k < n && k < j + 5; k += (arr[k] % 2) + 1) {
                arr[k] = (arr[k] * 3) % 997;
                total += arr[k];
            }
            j += (arr[j] > 0) ? 1 : 2;
        }
        
        i++;
    } while (i < n);
    
    return total;
}

/* Main driver creating all required patterns */
int main() {
    int data[SIZE];
    int processed[SIZE];
    int matrix[SIZE][SIZE];
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; ++i) {
        data[i] = (i * 37) % 7919;
        processed[i] = 0;
    }
    
    printf("Starting complex loop nesting tests...\n");
    
    /* Test 1: Mixed loop types */
    int result1 = mixed_loops_test(data, SIZE);
    printf("Mixed loops result: %d\n", result1);
    
    /* Test 2: Overlapping nested loops */
    overlapping_loops_test(matrix);
    
    /* Verify matrix computation */
    int matrix_sum = 0;
    for (int i = 0; i < SIZE/16; ++i) {
        for (int j = 0; j < SIZE/16; ++j) {
            matrix_sum += matrix[i][j];
        }
    }
    printf("Matrix sum sample: %d\n", matrix_sum);
    
    /* Test 3: Sibling loops */
    sibling_loops_test(data, processed, SIZE);
    
    /* Verify sibling loops */
    int sibling_sum = 0;
    for (int i = 0; i < SIZE; ++i) {
        sibling_sum += processed[i];
    }
    printf("Sibling loops sum: %d\n", sibling_sum);
    
    /* Test 4: Perfectly nested loops with function inlining */
    int chunk_result = 0;
    for (int base = 0; base < SIZE; base += 64) {
        chunk_result += process_chunk(
            processed + base,
            data + base,
            0, 64, 100
        );
    }
    printf("Chunk processing result: %d\n", chunk_result);
    
    /* Final validation */
    int final_total = result1 + matrix_sum + sibling_sum + chunk_result;
    printf("Final total: %d\n", final_total);
    
    return (final_total > 0) ? 0 : 1;
}
