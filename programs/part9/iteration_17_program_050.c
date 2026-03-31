/* 
 * Complex loop nesting test program designed to exercise 
 * hardware-assisted do-loop optimization logic in GCC.
 * Specifically targets bitmap intersection analysis for 
 * loop containment relationships.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define SIZE 1024
#define ITERATIONS 100

/* Force inlining to merge control flow graphs */
__attribute__((always_inline)) 
static inline void process_inner(int *restrict dst, const int *restrict src, int n) {
    /* Helper function with loops that will be inlined */
    for (int i = 0; i < n; i += 2) {
        if (i % 4 == 0) {
            dst[i] = src[i] * 2;
        } else {
            dst[i] = src[i] + 1;
            /* Early continue creates divergent control flow */
            if (dst[i] > 1000) continue;
            dst[i] *= 3;
        }
    }
}

/* Recursive function creating loop-like structure */
__attribute__((noinline))
static int recursive_loop(int *arr, int start, int end, int depth) {
    if (start >= end || depth <= 0) return 0;
    
    /* Partial overlap with caller's loops */
    int sum = 0;
    for (int i = start; i < end && i < start + 5; i++) {
        sum += arr[i];
        if (i % 3 == 0) {
            /* Jump to different point in loop */
            goto skip_point;
        }
        arr[i] = sum;
        skip_point:
        if (sum > 100) break;
    }
    
    /* Tail recursion creates overlapping block structure */
    return sum + recursive_loop(arr, start + 1, end - 1, depth - 1);
}

/* Test 1: Perfectly nested loops with subset relationship */
static int test_perfect_nesting(int *data) {
    int result = 0;
    
    /* Outer loop - larger block set */
    for (int i = 0; i < SIZE; i++) {
        data[i] = i;
        
        /* Middle loop - proper subset of outer */
        for (int j = 0; j < i && j < 10; j++) {
            if (j % 2 == 0) {
                /* Inner loop - proper subset of middle */
                for (int k = 0; k < j; k++) {
                    data[i] += data[k];
                    /* Complex increment */
                    k += (k % 3 == 0) ? 1 : 0;
                }
            } else {
                /* Alternative path still within middle loop */
                data[i] *= 2;
            }
            
            /* Conditional continue to different blocks */
            if (data[i] > 1000) continue;
            data[i] -= j;
        }
        
        result += data[i];
    }
    
    return result;
}

/* Test 2: Partially overlapping loops with shared and unique blocks */
static int test_partial_overlap(int *data, int *buffer) {
    int result = 0;
    int i = 0;
    
    /* First loop with some unique blocks */
    while (i < SIZE / 2) {
        buffer[i] = data[i] * 3;
        
        /* Switch statement creating multiple entry points */
        switch (i % 4) {
            case 0:
                buffer[i] += 1;
                /* Fall through */
            case 1:
                buffer[i] *= 2;
                break;
            case 2:
                /* goto creating non-contiguous blocks */
                if (buffer[i] > 500) goto special_case;
                buffer[i] /= 2;
                break;
            special_case:
            case 3:
                buffer[i] = 0;
                break;
        }
        
        i += (i % 5 == 0) ? 2 : 1;
    }
    
    /* Second loop overlapping with first but with unique blocks */
    for (int j = SIZE / 4; j < 3 * SIZE / 4; j++) {
        /* Shared computation with first loop */
        data[j] = buffer[j % (SIZE / 2)] + j;
        
        /* Unique computation path */
        if (j % 7 == 0) {
            /* Infinite loop with conditional break */
            for (;;) {
                data[j] >>= 1;
                if (data[j] < 10) break;
                if (data[j] % 11 == 0) break;
            }
        } else {
            /* Do-while with early exit */
            int k = 0;
            do {
                if (k > 5) break;
                data[j] += k * k;
                k++;
            } while (k < 8);
        }
        
        result ^= data[j];
    }
    
    return result;
}

/* Test 3: Sibling loops with no direct block sharing */
static int test_sibling_loops(int *data, int *aux) {
    int sum1 = 0, sum2 = 0;
    
    /* First sibling loop */
    #pragma GCC unroll 4
    for (int i = 0; i < SIZE; i += 4) {
        /* Loop with known bounds for optimization */
        if (__builtin_expect(i < SIZE - 4, 1)) {
            /* SIMD-friendly pattern */
            aux[i] = data[i] + data[i + 1];
            aux[i + 1] = data[i + 1] * data[i + 2];
            sum1 += aux[i] + aux[i + 1];
        } else {
            /* Epilogue handling */
            for (int j = i; j < SIZE; j++) {
                aux[j] = data[j];
                sum1 += aux[j];
            }
        }
    }
    
    /* Second sibling loop (no block overlap with first) */
    int idx = 0;
    while (idx < SIZE) {
        /* Different control flow structure */
        if (idx % 3 == 0 && idx % 5 == 0) {
            data[idx] = sum1;
        } else if (idx % 3 == 0 || idx % 5 == 0) {
            data[idx] = sum1 / 2;
        } else {
            data[idx] = 0;
        }
        
        /* Complex condition with short-circuit */
        if (idx < SIZE / 2 && data[idx] > 100) {
            sum2 += data[idx];
        } else if (idx >= SIZE / 2 || data[idx] <= 100) {
            sum2 -= data[idx];
        }
        
        idx += (sum2 % 7) + 1;
    }
    
    return sum1 + sum2;
}

/* Test 4: Complex mixed nesting with function inlining */
static int test_mixed_nesting_inline(int *data, int *temp) {
    int total = 0;
    
    /* Outer loop calling inline function */
    for (int outer = 0; outer < ITERATIONS; outer++) {
        /* Inlined function creates inner loop structure */
        process_inner(temp, data, SIZE);
        
        /* Complex middle loop with multiple exits */
        int mid = 0;
        while (mid < SIZE) {
            /* Nested switch with goto labels */
            switch (data[mid] % 6) {
                case 0:
                    temp[mid] += outer;
                    /* goto creating cross-loop block sharing */
                    if (temp[mid] > 1000) goto update_total;
                    break;
                case 1:
                case 2:
                    temp[mid] *= outer;
                    break;
                case 3:
                    /* Another loop inside */
                    for (int inner = 0; inner < 3; inner++) {
                        temp[mid] += inner * outer;
                        if (temp[mid] % 13 == 0) goto end_inner;
                    }
                    end_inner:
                    break;
                default:
                    temp[mid] = outer;
            }
            
            update_total:
            total += temp[mid];
            
            /* Multiple condition checks */
            if (mid > SIZE / 2 && total < 0 || outer % 11 == 0) {
                break;
            }
            
            mid += (outer % 4) + 1;
        }
        
        /* Call recursive function */
        total += recursive_loop(data, outer, SIZE - outer, 3);
    }
    
    return total;
}

/* Test 5: Loop distribution with hardware optimization hints */
static int test_hardware_hints(int *restrict a, int *restrict b, 
                               int *restrict c, int *restrict d) {
    int result = 0;
    
    /* Loop with stride pattern for prefetch analysis */
    for (int i = 0; i < SIZE; i++) {
        a[i] = i * 2;
        b[i] = i * 3;
    }
    
    /* Distributed loops that might be analyzed together */
    #pragma GCC unroll 2
    for (int i = 0; i < SIZE; i += 2) {
        c[i] = a[i] + b[i];
        /* Dependency chain */
        if (i > 0) {
            c[i] += c[i - 1];
        }
    }
    
    /* Another loop with different block structure */
    for (int i = 1; i < SIZE; i += 2) {
        d[i] = a[i] * b[i];
        /* Complex condition with function call */
        d[i] += (d[i] % 17 == 0) ? recursive_loop(c, 0, i, 2) : 0;
        result += d[i];
    }
    
    /* Final reduction loop */
    for (int i = 0; i < SIZE; i++) {
        result += c[i] - d[i];
    }
    
    return result;
}

int main(void) {
    /* Allocate and initialize data */
    int *data = malloc(SIZE * sizeof(int));
    int *buffer = malloc(SIZE * sizeof(int));
    int *temp = malloc(SIZE * sizeof(int));
    int *a = malloc(SIZE * sizeof(int));
    int *b = malloc(SIZE * sizeof(int));
    int *c = malloc(SIZE * sizeof(int));
    int *d = malloc(SIZE * sizeof(int));
    
    if (!data || !buffer || !temp || !a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; i++) {
        data[i] = i % 97;
        a[i] = i % 53;
        b[i] = i % 71;
    }
    
    printf("Testing complex loop nesting patterns for hardware do-loop analysis\n");
    
    /* Execute tests to trigger bitmap intersection logic */
    int result1 = test_perfect_nesting(data);
    printf("Test 1 (perfect nesting): %d\n", result1);
    
    int result2 = test_partial_overlap(data, buffer);
    printf("Test 2 (partial overlap): %d\n", result2);
    
    int result3 = test_sibling_loops(data, buffer);
    printf("Test 3 (sibling loops): %d\n", result3);
    
    int result4 = test_mixed_nesting_inline(data, temp);
    printf("Test 4 (mixed with inlining): %d\n", result4);
    
    int result5 = test_hardware_hints(a, b, c, d);
    printf("Test 5 (hardware hints): %d\n", result5);
    
    /* Final validation to prevent optimization */
    int final = result1 + result2 + result3 + result4 + result5;
    printf("Final result: %d\n", final);
    
    /* Cleanup */
    free(data);
    free(buffer);
    free(temp);
    free(a);
    free(b);
    free(c);
    free(d);
    
    return (final != 0) ? 0 : 1;
}
