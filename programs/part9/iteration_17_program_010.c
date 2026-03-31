/* 
 * Test program to exercise hardware do-loop optimization logic in GCC
 * Specifically targets bitmap intersection analysis for loop nesting relationships
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define SIZE 1024
#define ITERATIONS 100
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

/* Force inlining to create complex control flow within loops */
__attribute__((always_inline)) 
static inline int process_chunk(int *__restrict dest, const int *__restrict src, 
                                int start, int end, int threshold) {
    int sum = 0;
    
    /* Loop with multiple exit points and complex condition */
    for (int i = start; i < end; i += (i % 3) + 1) {
        if (UNLIKELY(src[i] > threshold)) {
            dest[i] = src[i] * 2;
            if (dest[i] > 1000) goto early_exit;
        } else if (src[i] < -threshold) {
            dest[i] = src[i] / 2;
            continue;  /* Skip accumulation */
        } else {
            dest[i] = src[i];
        }
        sum += dest[i];
        
        /* Nested switch creating divergent control flow */
        switch (i % 4) {
            case 0: sum += 1; break;
            case 1: sum -= 1; break;
            case 2: 
                if (sum > 500) continue;  /* Jump to loop start */
                break;
            case 3:
                sum *= 2;
                /* Fall through to default */
            default:
                if (i % 7 == 0) break;
        }
    }
    
early_exit:
    return sum;
}

/* Function with partially overlapping loops */
static void test_partial_overlap(int *arr1, int *arr2, int n) {
    int i = 0, j = 0;
    
    /* First loop with complex increment */
    while (i < n && j < n) {
        arr1[i] = i * j;
        
        /* Conditional creating divergent paths */
        if (i % 2 == 0) {
            /* Inner loop that shares some blocks */
            for (int k = 0; k < (i % 5); ++k) {
                arr2[j] += arr1[i] * k;
                if (k == 2) goto skip_point;  /* Multiple entry points */
                arr2[j] -= 1;
skip_point:
                if (arr2[j] < 0) break;
            }
            j += 2;
        } else {
            /* Different path with its own loop */
            do {
                arr2[j] = arr1[i] - j;
                j++;
            } while (j < n && arr2[j-1] > 0);
        }
        
        i += (i % 3) + 1;
        
        /* Switch with goto labels creating non-contiguous blocks */
        switch (i % 3) {
            case 0: goto update_a;
            case 1: goto update_b;
            case 2: goto update_c;
        }
        
update_a:
        arr1[i % n] += 1;
        continue;
update_b:
        arr2[j % n] += 2;
        continue;
update_c:
        arr1[i % n] += arr2[j % n];
    }
}

/* Perfectly nested loops */
static int test_perfect_nesting(int *matrix, int rows, int cols) {
    int total = 0;
    
    #pragma GCC unroll 2
    for (int i = 0; i < rows; ++i) {
        /* Outer loop blocks completely contain inner loop blocks */
        for (int j = 0; j < cols; ++j) {
            int idx = i * cols + j;
            
            /* Multiple condition checks */
            while (matrix[idx] > 0 && j < cols - 1) {
                matrix[idx] -= (i + j);
                if (matrix[idx] < 0) {
                    matrix[idx] = 0;
                    break;  /* Early exit from while */
                }
                j++;
                idx = i * cols + j;
            }
            
            /* do-while with nested if */
            do {
                if (matrix[idx] % 2 == 0) {
                    matrix[idx] /= 2;
                    continue;  /* Skip to while check */
                } else {
                    matrix[idx] *= 3;
                }
                total += matrix[idx];
            } while (matrix[idx] > 10 && ++j < cols);
        }
    }
    
    return total;
}

/* Sibling loops with shared outer loop */
static void test_sibling_loops(int *data, int n) {
    int sum_a = 0, sum_b = 0;
    
    /* Outer loop containing two sibling inner loops */
    for (int outer = 0; outer < ITERATIONS; ++outer) {
        /* First sibling loop */
        int i = outer % n;
        while (i < n) {
            data[i] += outer;
            if (data[i] > 1000) {
                data[i] = 1000;
                goto next_sibling;  /* Jump between sibling loops */
            }
            sum_a += data[i];
            i += (outer % 4) + 1;
        }
        
        /* Second sibling loop (shares no blocks with first) */
        next_sibling:
        for (int j = 0; j < (outer % 8); ++j) {
            int idx = (outer + j) % n;
            data[idx] -= j;
            sum_b += data[idx];
            
            /* Infinite loop with conditional break */
            for (;;) {
                if (data[idx] < -500) break;
                data[idx] -= 1;
                if (data[idx] % 7 == 0) break;
                
                /* Another nested infinite loop */
                while (1) {
                    data[idx] += 1;
                    if (data[idx] >= 0) break;
                }
                break;
            }
        }
    }
    
    /* Prevent dead code elimination */
    data[0] = sum_a + sum_b;
}

/* Recursive function creating loop-like structures */
static int recursive_loop(int *arr, int depth, int idx, int max_depth) {
    if (depth >= max_depth || idx >= SIZE) {
        return arr[idx % SIZE];
    }
    
    int result = 0;
    
    /* Loop inside recursion */
    for (int i = 0; i < depth; ++i) {
        arr[idx] += i;
        
        /* Multiple recursive calls */
        if (i % 2 == 0) {
            result += recursive_loop(arr, depth + 1, idx * 2, max_depth);
        } else {
            result += recursive_loop(arr, depth + 1, idx + 1, max_depth);
        }
        
        /* Complex condition with short-circuit */
        if (i > 2 && arr[idx] < 100 && depth < 5) {
            result *= 2;
        }
    }
    
    return result;
}

/* Main test driver */
int main(void) {
    int *array1 = malloc(SIZE * sizeof(int));
    int *array2 = malloc(SIZE * sizeof(int));
    int *matrix = malloc(SIZE * SIZE * sizeof(int));
    
    if (!array1 || !array2 || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    for (int i = 0; i < SIZE; ++i) {
        array1[i] = i - SIZE/2;
        array2[i] = i * 2;
    }
    
    for (int i = 0; i < SIZE * SIZE; ++i) {
        matrix[i] = i % 100;
    }
    
    int total = 0;
    
    /* Test 1: Partial overlap loops */
    printf("Test 1: Partial overlap loops\n");
    test_partial_overlap(array1, array2, SIZE);
    total += array1[SIZE/2] + array2[SIZE/2];
    
    /* Test 2: Perfect nesting */
    printf("Test 2: Perfect nesting loops\n");
    total += test_perfect_nesting(matrix, 32, 32);
    
    /* Test 3: Sibling loops */
    printf("Test 3: Sibling loops\n");
    test_sibling_loops(array1, SIZE);
    total += array1[0];
    
    /* Test 4: Inlined function with complex loop */
    printf("Test 4: Inlined function processing\n");
    total += process_chunk(array2, array1, 0, SIZE, 100);
    
    /* Test 5: Recursive loop-like structure */
    printf("Test 5: Recursive loops\n");
    total += recursive_loop(array1, 0, 1, 5);
    
    /* Test 6: Mixed loop types in sequence */
    printf("Test 6: Mixed loop sequence\n");
    int k = 0;
    
    /* while with complex condition */
    while (k < SIZE && array1[k] < 1000) {
        /* for with pointer arithmetic */
        for (int *ptr = array2; ptr < array2 + (k % 10); ++ptr) {
            *ptr += k;
        }
        
        /* do-while with early continue */
        do {
            if (k % 3 == 0) continue;
            array1[k] -= array2[k];
            k += (k % 7) + 1;
        } while (k < SIZE / 2);
        
        k++;
    }
    
    /* Final computation to use all results */
    int final_result = 0;
    for (int i = 0; i < SIZE; ++i) {
        final_result += array1[i] + array2[i];
        if (i < 32 * 32) {
            final_result += matrix[i];
        }
    }
    
    final_result += total;
    
    printf("Final result: %d\n", final_result);
    printf("(This value varies based on optimizations)\n");
    
    free(array1);
    free(array2);
    free(matrix);
    
    return 0;
}
