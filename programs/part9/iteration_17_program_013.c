/* 
 * Complex loop nesting test program targeting hw-doloop.cc coverage
 * Specifically designed to exercise bitmap intersection logic for loop containment
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define SIZE 1024
#define ITERATIONS 100

/* Force inlining to create complex CFG within loops */
__attribute__((always_inline)) 
static inline int process_block(int *data, int idx, int mod) {
    if (idx % mod == 0) {
        data[idx] = data[idx] * 2 + 1;
        return 1;
    } else if (idx % (mod + 1) == 0) {
        data[idx] = data[idx] / 2 - 1;
        return 2;
    } else {
        data[idx] = data[idx] + idx;
        return 3;
    }
}

/* Helper function with loops that will be inlined */
__attribute__((always_inline))
static inline void nested_helper(int *__restrict src, int *__restrict dst, int start, int end) {
    /* Creates partially overlapping block structure */
    int i = start;
    while (i < end) {
        /* Multiple condition checks create separate basic blocks */
        if (i > start + (end - start)/3 && i < start + 2*(end - start)/3) {
            /* Inner loop-like structure through goto */
            int j = i;
process_inner:
            dst[j] = src[j] * 3;
            j++;
            if (j < i + 5 && j < end) {
                goto process_inner;  /* Creates non-contiguous blocks */
            }
            i = j;
        } else {
            dst[i] = src[i] + process_block(src, i, 7);
            i += (i % 3) + 1;  /* Variable increment */
        }
        
        /* Early exit point */
        if (dst[i-1] > 10000) {
            break;
        }
    }
}

/* Test 1: Perfectly nested loops with subset relationship */
static int test_perfect_nesting(int *arr) {
    int sum = 0;
    
    /* Outer loop - larger block bitmap */
    for (int i = 0; i < SIZE; i += 2) {
        /* Multiple basic blocks in outer loop */
        if (i % 4 == 0) {
            arr[i] = i * 2;
        } else {
            arr[i] = i * 3;
        }
        
        /* Perfectly nested inner loop - proper subset */
        for (int j = 0; j < 10; j++) {
            /* Inner loop blocks are all within outer loop */
            switch (j % 3) {
                case 0: arr[i] += j; break;
                case 1: arr[i] -= j; break;
                case 2: arr[i] *= j; break;
            }
            
            /* Continue to different points */
            if (j % 2 == 0) continue;
            
            arr[i] += process_block(arr, i, 5);
        }
        
        sum += arr[i];
    }
    
    return sum;
}

/* Test 2: Partially overlapping loops */
static int test_partial_overlap(int *arr1, int *arr2) {
    int result = 0;
    int i = 0;
    
    /* First loop with complex structure */
    do {
        /* Multiple entry points via labels */
        if (i % 5 == 0) {
            goto special_case;
        }
        
        arr1[i] = i * i;
        
        /* Nested conditional loop */
        int k = 0;
        while (k < 8) {
            arr1[i] += k;
            k += (k % 2) + 1;  /* Variable increment */
            
            /* Early exit creates separate block */
            if (arr1[i] > 1000) {
                break;
            }
        }
        
        result += arr1[i];
        i++;
        continue;
        
special_case:
        arr1[i] = -i;
        i++;
    } while (i < SIZE/2);
    
    /* Second loop overlapping with first */
    for (int j = SIZE/4; j < 3*SIZE/4; j++) {
        /* Shares some blocks with first loop, has unique blocks */
        if (j < SIZE/2) {
            /* This block exists in both loops */
            arr2[j] = arr1[j] * 2;
        } else {
            /* Unique to this loop */
            arr2[j] = j * j;
            
            /* Infinite loop with conditional break */
            for (;;) {
                arr2[j] /= 2;
                if (arr2[j] < 100) break;
                
                /* Another level of nesting */
                int m = 0;
                while (m < 3) {
                    arr2[j] += m;
                    m++;
                }
            }
        }
        
        result += arr2[j];
    }
    
    return result;
}

/* Test 3: Sibling loops with no direct nesting */
static int test_sibling_loops(int *matrix) {
    int total = 0;
    
    /* Common outer loop */
    for (int row = 0; row < 16; row++) {
        /* First sibling loop - processes first half */
        #pragma GCC unroll 2
        for (int col = 0; col < 8; col++) {
            int idx = row * 16 + col;
            matrix[idx] = row * col;
            
            /* Complex conditional structure */
            if (col % 3 == 0) {
                matrix[idx] += process_block(matrix, idx, 4);
            } else if (col % 3 == 1) {
                for (int k = 0; k < 2; k++) {
                    matrix[idx] -= k;
                }
            }
            
            total += matrix[idx];
        }
        
        /* Second sibling loop - processes second half, no block overlap */
        int col = 8;
        while (col < 16) {
            int idx = row * 16 + col;
            matrix[idx] = row + col;
            
            /* Different control flow pattern */
            switch (row % 4) {
                case 0: matrix[idx] *= 2; break;
                case 1: matrix[idx] /= 2; break;
                case 2: 
                    /* Nested for with goto */
                    for (int x = 0; x < 2; x++) {
                        matrix[idx] += x;
                        if (x == 1) goto skip_point;
                    }
                    break;
                case 3: matrix[idx] = matrix[idx]; break;
            }
            
skip_point:
            total += matrix[idx];
            col += (row % 2) + 1;  /* Variable increment */
        }
    }
    
    return total;
}

/* Test 4: Recursive function creating loop-like structures */
static int recursive_loop(int *data, int depth, int idx, int max) {
    if (depth >= 5 || idx >= max) {
        return 0;
    }
    
    int sum = 0;
    
    /* Loop-like recursion */
    for (int i = 0; i < 3; i++) {
        int new_idx = idx + i;
        if (new_idx >= max) break;
        
        data[new_idx] = depth * 100 + i;
        
        /* Multiple recursive paths */
        if (i % 2 == 0) {
            sum += data[new_idx] + recursive_loop(data, depth + 1, new_idx * 2, max);
        } else {
            sum += data[new_idx] - recursive_loop(data, depth + 1, new_idx + 5, max);
        }
        
        /* Conditional continue */
        if (data[new_idx] > 500) {
            continue;
        }
        
        data[new_idx] += process_block(data, new_idx, 3);
    }
    
    return sum;
}

/* Test 5: Complex mixed loops with function inlining */
static int test_mixed_loops(int *arr1, int *arr2, int *arr3) {
    int result = 0;
    
    /* Outer infinite loop with multiple exits */
    for (int outer = 0; ; outer++) {
        if (outer >= ITERATIONS) break;
        
        /* First inner loop - while with multiple conditions */
        int i = 0;
        while (i < SIZE && arr1[i] < 10000 && i % 7 != 0) {
            /* Call to inline function creates overlapping blocks */
            nested_helper(arr1, arr2, i, i + 10);
            
            /* Loop-carried dependency */
            arr3[i] = arr2[i] + (i > 0 ? arr3[i-1] : 0);
            
            /* Multiple continue points */
            if (i % 11 == 0) {
                i += 3;
                continue;
            }
            
            result += arr3[i];
            i += __builtin_expect((i % 13 == 0), 0) ? 2 : 1;
        }
        
        /* Second loop overlapping with parts of first */
        for (int j = SIZE/3; j < 2*SIZE/3; j += 2) {
            /* Shared blocks with first loop's helper */
            if (j % 4 == 0) {
                arr1[j] = arr2[j] * arr3[j];
            } else {
                /* Unique block structure */
                int k = j;
                do {
                    arr1[k] += k * k;
                    k--;
                } while (k > j - 5 && k >= 0);
            }
            
            result += arr1[j];
            
            /* Hardware loop hint */
            if (__builtin_expect(j % 128 == 0, 0)) {
                /* Potential hardware do-loop candidate */
                for (int p = 0; p < 8; p++) {
                    arr2[j + p] = arr1[j] >> p;
                }
            }
        }
        
        /* Early break from outer loop */
        if (result > 1000000) {
            break;
        }
    }
    
    return result;
}

int main() {
    int *array1 = (int*)malloc(SIZE * sizeof(int));
    int *array2 = (int*)malloc(SIZE * sizeof(int));
    int *array3 = (int*)malloc(SIZE * sizeof(int));
    int *matrix = (int*)malloc(256 * sizeof(int));  /* 16x16 */
    
    if (!array1 || !array2 || !array3 || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = i % 100;
        array2[i] = (i * 2) % 100;
        array3[i] = (i * 3) % 100;
    }
    
    for (int i = 0; i < 256; i++) {
        matrix[i] = i;
    }
    
    int total = 0;
    
    /* Execute all tests to cover different bitmap intersection scenarios */
    printf("Running loop nesting tests...\n");
    
    total += test_perfect_nesting(array1);
    printf("Test 1 completed: %d\n", total);
    
    total += test_partial_overlap(array1, array2);
    printf("Test 2 completed: %d\n", total);
    
    total += test_sibling_loops(matrix);
    printf("Test 3 completed: %d\n", total);
    
    total += recursive_loop(array3, 0, 0, SIZE/4);
    printf("Test 4 completed: %d\n", total);
    
    total += test_mixed_loops(array1, array2, array3);
    printf("Test 5 completed: %d\n", total);
    
    /* Final computation to prevent optimization */
    int final_check = 0;
    for (int i = 0; i < SIZE; i++) {
        final_check += array1[i] + array2[i] + array3[i];
    }
    for (int i = 0; i < 256; i++) {
        final_check += matrix[i];
    }
    
    printf("Final result: %d (checksum: %d)\n", total, final_check);
    
    free(array1);
    free(array2);
    free(array3);
    free(matrix);
    
    return 0;
}
