/* 
 * Complex loop nesting test program designed to exercise
 * hardware do-loop optimization logic in GCC, specifically
 * targeting bitmap intersection analysis for loop containment.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define SIZE 1024
#define ITERATIONS 100

/* Force inlining to create complex control flow within loops */
__attribute__((always_inline)) 
static inline int process_chunk(int *arr, int start, int end, int mod) {
    int sum = 0;
    
    /* Loop with multiple exit points and conditional blocks */
    for (int i = start; i < end; i += (mod > 0 ? mod : 1)) {
        if (arr[i] % 2 == 0) {
            sum += arr[i];
            /* Early continue creates divergent control flow */
            if (arr[i] > 1000) continue;
        } else {
            sum -= arr[i];
            /* Conditional break at different depth */
            if (arr[i] < -1000) break;
        }
        
        /* Nested switch inside loop */
        switch (arr[i] % 4) {
            case 0:
                arr[i] >>= 1;
                break;
            case 1:
                arr[i] <<= 1;
                /* Fall through to create shared basic blocks */
            case 2:
                arr[i] ^= 0xFF;
                break;
            default:
                arr[i] = ~arr[i];
                /* goto creates non-contiguous block ranges */
                if (arr[i] == 0) goto skip_processing;
        }
        
        skip_processing:
        /* Empty target for goto */
        if (i % 7 == 0) {
            /* Another conditional continue */
            continue;
        }
    }
    return sum;
}

/* Helper function with loop that will be inlined */
__attribute__((always_inline))
static inline void transform_matrix(int mat[SIZE][SIZE], int n, int *result) {
    int temp = 0;
    
    /* Perfectly nested loops - inner blocks are subset of outer */
    for (int i = 0; i < n; ++i) {
        /* Complex increment expression */
        for (int j = 0; j < n; j += (i % 3) + 1) {
            mat[i][j] = (mat[i][j] * 3) / 2;
            
            /* Conditional with multiple basic blocks */
            if (mat[i][j] > 100 && i < j) {
                mat[i][j] -= 50;
                /* Nested if creates more blocks */
                if (mat[i][j] % 11 == 0) {
                    mat[i][j] >>= 2;
                }
            } else if (mat[i][j] < -100) {
                mat[i][j] += 75;
            }
            
            temp += mat[i][j];
        }
        
        /* Sibling loop at same nesting level */
        int k = 0;
        while (k < n && temp > 0) {
            mat[i][k] += temp % 10;
            k += 2;  /* Skip pattern */
            /* Loop-carried dependency */
            if (k > n/2) {
                break;
            }
        }
    }
    
    *result = temp;
}

/* Function with partially overlapping loops */
static void overlapping_loops(int *arr1, int *arr2, int len, int *out1, int *out2) {
    int sum1 = 0, sum2 = 0;
    int i = 0, j = 0;
    
    /* First loop with complex condition */
    while (i < len && j < len/2) {
        /* Shared basic block between loops */
        int val = arr1[i] + arr2[j];
        
        if (val > 0) {
            sum1 += val;
            i += 2;
        } else {
            sum2 -= val;
            j += 1;
        }
        
        /* Conditional goto creates non-contiguous blocks */
        if (val == 0) goto shared_exit_point;
        
        /* Infinite loop with conditional break */
        for (;;) {
            if (i >= len || j >= len/2) break;
            if (arr1[i] == arr2[j]) {
                sum1++;
                break;  /* Break from infinite loop */
            }
            i++; j++;
        }
    }
    
    shared_exit_point:
    
    /* Second loop that partially overlaps with first */
    do {
        sum2 += arr1[i % len] * arr2[j % len];
        
        /* Complex increment with side effect */
        i += (sum2 % 5) + 1;
        j += (sum1 % 3) + 1;
        
        /* Multiple condition checks */
    } while (i < len * 2 && j < len * 2 && (sum1 + sum2) < 10000);
    
    *out1 = sum1;
    *out2 = sum2;
}

/* Recursive function creating loop-like structure */
static int recursive_loop(int *arr, int start, int end, int depth) {
    if (start >= end || depth <= 0) return 0;
    
    int mid = (start + end) / 2;
    int sum = 0;
    
    /* Tail recursion creates loop-like control flow */
    if (arr[mid] % 2 == 0) {
        sum = arr[mid] + recursive_loop(arr, start, mid, depth - 1);
    } else {
        sum = arr[mid] - recursive_loop(arr, mid + 1, end, depth - 1);
    }
    
    /* Additional processing */
    for (int i = start; i < mid; i++) {
        arr[i] = (arr[i] + sum) % 100;
    }
    
    return sum;
}

/* Test function with mixed loop types and complex nesting */
static void test_complex_nesting(void) {
    int data[SIZE];
    int matrix[SIZE/32][SIZE/32];
    int result1, result2, result3;
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (i * 17) % 7919;  /* Prime modulus for variety */
    }
    
    for (int i = 0; i < SIZE/32; i++) {
        for (int j = 0; j < SIZE/32; j++) {
            matrix[i][j] = (i * j) % 97;
        }
    }
    
    /* Test 1: Perfect nesting with inlined function */
    transform_matrix(matrix, SIZE/32, &result1);
    
    /* Test 2: Partially overlapping loops */
    int out1, out2;
    overlapping_loops(data, data + SIZE/2, SIZE/2, &out1, &out2);
    
    /* Test 3: Process chunks with different mod values */
    int chunk_sum = 0;
    for (int mod = 1; mod <= 5; mod++) {
        chunk_sum += process_chunk(data, 0, SIZE, mod);
        
        /* Nested while with complex condition */
        int k = 0;
        while (k < SIZE && (data[k] != 0 || chunk_sum > 0)) {
            data[k] = (data[k] + mod) % 256;
            k += mod * 2;
            
            /* Switch with fall-through */
            switch (data[k] % 3) {
                case 0:
                    data[k] += 10;
                    /* Fall through */
                case 1:
                    data[k] *= 2;
                    break;
                case 2:
                    data[k] /= 2;
                    break;
            }
        }
    }
    
    /* Test 4: Recursive loop-like structure */
    result3 = recursive_loop(data, 0, SIZE, 10);
    
    /* Test 5: Sibling loops (same nesting level) */
    int sibling_sum1 = 0, sibling_sum2 = 0;
    
    /* First sibling loop */
    for (int i = 0; i < SIZE; i += 3) {
        sibling_sum1 += data[i];
        if (data[i] > 100) {
            /* goto to different label creates unique blocks */
            goto process_odd;
        }
        data[i] >>= 1;
    }
    
    process_odd:
    
    /* Second sibling loop - shares no blocks with first */
    for (int i = 1; i < SIZE; i += 3) {
        sibling_sum2 += data[i];
        data[i] <<= 1;
        
        /* Nested do-while with early exit */
        int j = 0;
        do {
            if (j > 5) break;
            data[i] += j;
            j++;
        } while (data[i] < 1000);
    }
    
    /* Use results to prevent optimization */
    printf("Results: %d %d %d %d %d %d %d\n", 
           result1, out1, out2, chunk_sum, result3, sibling_sum1, sibling_sum2);
}

/* Main driver with multiple test iterations */
int main(void) {
    int total = 0;
    
    /* Use __builtin_expect for loop bound hints */
    for (int iter = 0; __builtin_expect(iter < ITERATIONS, 1); ++iter) {
        test_complex_nesting();
        total += iter;
    }
    
    /* Additional stress test with restrict pointers */
    {
        int * __restrict a = malloc(SIZE * sizeof(int));
        int * __restrict b = malloc(SIZE * sizeof(int));
        
        if (a && b) {
            /* Hardware-friendly loop with restrict */
            #pragma GCC unroll 4
            for (int i = 0; i < SIZE; i++) {
                a[i] = i;
                b[i] = SIZE - i;
            }
            
            /* Complex nested loop with restrict */
            for (int i = 0; i < SIZE; i++) {
                for (int j = 0; j < SIZE; j += 16) {
                    a[i] += b[j] * (i + j);
                }
            }
            
            free(a);
            free(b);
        }
    }
    
    printf("Total iterations: %d\n", total);
    return 0;
}
