/* 
 * Complex loop nesting test program targeting hw-doloop.cc uncovered lines
 * Lines 429-436: bitmap intersection logic for loop containment analysis
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define SIZE 1024
#define ITERATIONS 100

/* Force inlining to create complex CFG within loops */
__attribute__((always_inline)) 
static inline int process_chunk(int *restrict dst, const int *restrict src, int start, int end) {
    int sum = 0;
    
    /* Loop with multiple exit points and complex control flow */
    for (int i = start; i < end; i += (i % 3) + 1) {
        if (src[i] > 0) {
            dst[i] = src[i] * 2;
            sum += dst[i];
            
            /* Early exit creates divergent basic blocks */
            if (sum > 10000) {
                goto early_exit;
            }
        } else if (src[i] < -100) {
            /* Another exit path */
            break;
        } else {
            dst[i] = src[i] / 2;
            sum += dst[i];
            
            /* Continue with skip creates disjoint blocks */
            if (i % 7 == 0) {
                continue;
            }
            
            /* Nested if creates more blocks */
            if (dst[i] % 2 == 0) {
                dst[i] += 1;
            }
        }
        
        /* Switch inside loop creates multiple case blocks */
        switch (i % 4) {
            case 0:
                dst[i] += i;
                break;
            case 1:
                dst[i] -= i;
                /* Fall through creates shared block */
            case 2:
                dst[i] *= 2;
                break;
            case 3:
                dst[i] /= 2;
                /* break omitted to create different flow */
            default:
                dst[i] += 3;
        }
        
        early_exit:
        /* Empty label creates basic block */
    }
    
    return sum;
}

/* Function with perfectly nested loops - inner blocks are subset of outer */
__attribute__((always_inline))
static inline void perfect_nesting(int *arr, int n) {
    int total = 0;
    
    /* Outer loop */
    for (int i = 0; i < n; ++i) {
        __builtin_expect(i < n - 1, 1); /* Hint for hardware loop */
        
        /* Middle loop - perfectly nested */
        for (int j = 0; j < i; ++j) {
            /* Innermost loop - perfectly nested */
            for (int k = 0; k < j; ++k) {
                arr[k] = arr[i] + arr[j] - arr[k];
                total += arr[k];
                
                /* Conditional continue creates unique block */
                if (arr[k] % 2 == 0) {
                    continue;
                }
                
                /* Complex expression with hardware-friendly stride */
                arr[k] += (k * 7) & 0xFF;
            }
            
            /* Partial block only in middle loop */
            if (j % 3 == 0) {
                arr[j] ^= 0xAAAA;
            }
        }
        
        /* Block only in outer loop */
        arr[i] = total % 256;
    }
    
    /* Prevent optimization */
    asm volatile("" : : "r"(total) : "memory");
}

/* Function with partially overlapping loops */
static void partial_overlap(int *a, int *b, int n) {
    int i = 0, j = 0;
    
    /* First loop with complex entry */
    while (i < n && j < n) {
        a[i] = b[j] * 2;
        
        /* Nested loop that shares some blocks */
        do {
            if (i + j < n) {
                b[j] += a[i];
                
                /* Inner loop with early exit */
                for (int k = 0; k < 5; ++k) {
                    if (a[i] + k > 100) {
                        goto inner_exit;
                    }
                    b[j] -= k;
                }
                inner_exit: ;
            }
            
            j += (i % 2) + 1;
        } while (j < n && b[j] > 0);
        
        i += (j % 3) + 1;
        
        /* Sibling loop structure at same level */
        if (i % 5 == 0) {
            int temp = 0;
            while (temp < 10) {
                a[i % n] += temp;
                temp += (i % 3);
                
                /* Infinite loop with conditional break */
                for (;;) {
                    if (temp > 100) break;
                    if (temp < 0) break;
                    temp += 2;
                }
            }
        }
    }
}

/* Function with sibling loops (same nesting level) */
static void sibling_loops(int *matrix, int rows, int cols) {
    int sum1 = 0, sum2 = 0;
    
    /* First sibling loop */
    #pragma GCC unroll 2
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; c += 2) {
            int idx = r * cols + c;
            matrix[idx] = (matrix[idx] * 3) / 2;
            sum1 += matrix[idx];
            
            /* Complex condition with short-circuit */
            if (c > 0 && (matrix[idx] < matrix[idx-1] || r == 0)) {
                matrix[idx] += sum1;
            }
        }
    }
    
    /* Second sibling loop - shares no blocks with first */
    for (int r = rows - 1; r >= 0; --r) {
        int c = cols - 1;
        while (c >= 0) {
            int idx = r * cols + c;
            matrix[idx] = (matrix[idx] + 7) * 11;
            sum2 += matrix[idx];
            
            /* Multiple decrement paths */
            if (matrix[idx] % 2 == 0) {
                c -= 2;
            } else if (matrix[idx] % 3 == 0) {
                c -= 3;
            } else {
                c -= 1;
            }
        }
    }
    
    /* Use results */
    matrix[0] = sum1 + sum2;
}

/* Recursive function creating loop-like structures */
static int recursive_loop(int *arr, int depth, int max) {
    if (depth >= max) return arr[0];
    
    int result = 0;
    
    /* Loop inside recursion */
    for (int i = 0; i < depth; ++i) {
        arr[i] += recursive_loop(arr + i, depth + 1, max);
        result += arr[i];
        
        /* Tail recursion-like structure */
        if (i % 2 == 0) {
            int temp = recursive_loop(arr, depth + 1, max - 1);
            result += temp;
        }
    }
    
    return result;
}

/* Main test driver */
int main(void) {
    int *data1 = malloc(SIZE * sizeof(int));
    int *data2 = malloc(SIZE * sizeof(int));
    int *matrix = malloc(SIZE * SIZE * sizeof(int));
    
    if (!data1 || !data2 || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; ++i) {
        data1[i] = i * 3 - SIZE/2;
        data2[i] = i * 7 % 256;
    }
    
    for (int i = 0; i < SIZE * SIZE; ++i) {
        matrix[i] = i % 100;
    }
    
    int total = 0;
    
    /* Run tests multiple times to ensure execution */
    for (int iter = 0; iter < ITERATIONS; ++iter) {
        /* Test 1: Perfect nesting with complex control flow */
        perfect_nesting(data1, SIZE);
        total += data1[SIZE/2];
        
        /* Test 2: Partial overlap loops */
        partial_overlap(data1, data2, SIZE);
        total += data2[SIZE/3];
        
        /* Test 3: Sibling loops */
        sibling_loops(matrix, 32, 32);
        total += matrix[0];
        
        /* Test 4: Process chunk with multiple exits */
        int chunk_sum = process_chunk(data1, data2, 0, SIZE/2);
        total += chunk_sum;
        
        /* Test 5: Recursive loop structures */
        int rec_result = recursive_loop(data1, 1, 5);
        total += rec_result;
        
        /* Modify data to prevent same pattern each iteration */
        data1[iter % SIZE] = total % 1000;
        data2[iter % SIZE] = iter * 13;
    }
    
    /* Print results to prevent optimization */
    printf("Total: %d\n", total);
    printf("Checksum: data1[0]=%d, data2[0]=%d, matrix[0]=%d\n", 
           data1[0], data2[0], matrix[0]);
    
    free(data1);
    free(data2);
    free(matrix);
    
    return 0;
}
