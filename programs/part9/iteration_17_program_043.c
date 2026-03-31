/* 
 * Test program to exercise hardware do-loop optimization logic in GCC
 * Specifically targets bitmap intersection logic for loop nesting analysis
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define ARRAY_SIZE 1024
#define MATRIX_SIZE 32

/* Helper function that will be inlined */
static inline __attribute__((always_inline)) 
void process_chunk(int *__restrict dest, const int *__restrict src, int start, int end) {
    /* Creates a loop with complex control flow */
    for (int i = start; i < end; i += (i % 3) + 1) {  /* Variable stride */
        if (src[i] & 1) {
            dest[i] = src[i] * 2;
            if (dest[i] > 1000) {
                dest[i] = 1000;  /* Creates additional basic block */
            }
        } else {
            dest[i] = src[i] / 2;
            /* Early continue creates divergent control flow */
            if (dest[i] < 0) continue;
            dest[i] += i;  /* Loop-carried dependency */
        }
        
        /* Switch inside loop creates multiple basic blocks */
        switch (src[i] % 4) {
            case 0:
                dest[i] += 1;
                break;
            case 1:
                dest[i] += 2;
                /* Fall through to create shared block */
            case 2:
                dest[i] *= 2;
                break;
            default:
                dest[i] -= 1;
                /* goto creates non-contiguous blocks */
                if (dest[i] < -100) goto adjust_value;
        }
        
        if (dest[i] == 0) {
            dest[i] = 1;
        }
        
        adjust_value:
        if (dest[i] > 500) {
            dest[i] = 500;
        }
    }
}

/* Function with perfectly nested loops - inner blocks are subset of outer */
void test_perfect_nesting(int *__restrict arr) {
    int sum = 0;
    
    /* Outer loop */
    for (int i = 0; i < MATRIX_SIZE; ++i) {
        /* Middle loop - perfectly nested */
        for (int j = 0; j < MATRIX_SIZE; ++j) {
            /* Innermost loop - all blocks are subset of outer loops */
            for (int k = 0; k < MATRIX_SIZE; ++k) {
                int idx = i * MATRIX_SIZE * MATRIX_SIZE + j * MATRIX_SIZE + k;
                if (__builtin_expect(idx < ARRAY_SIZE, 1)) {
                    arr[idx] = i + j + k;
                    sum += arr[idx];
                    
                    /* Conditional break creates multiple exit points */
                    if (sum > 10000) {
                        break;  /* Only breaks innermost loop */
                    }
                }
            }
            
            /* Different code path creates unique blocks in middle loop */
            if (j % 2 == 0) {
                sum += j * 10;
            } else {
                sum -= j * 5;
                /* continue skips to different point */
                if (sum < 0) continue;
            }
            
            sum += 1;
        }
        
        /* Outer loop has blocks not in inner loops */
        if (i % 3 == 0) {
            sum *= 2;
        } else if (i % 3 == 1) {
            sum /= 2;
        } else {
            sum += 100;
        }
    }
    
    printf("Perfect nesting sum: %d\n", sum);
}

/* Function with partially overlapping loops */
void test_partial_overlap(int *__restrict arr) {
    int result = 0;
    int i = 0;
    
    /* First loop with complex condition */
    while (i < ARRAY_SIZE / 2 && result < 5000) {
        arr[i] = i * 2;
        result += arr[i];
        
        /* Nested loop that shares some blocks but has unique ones */
        if (i % 5 == 0) {
            for (int j = 0; j < 10; ++j) {
                arr[i + j] += j;
                result += j;
                
                /* Early exit creates unique block */
                if (result > 3000) {
                    break;
                }
            }
            
            /* This block is only in outer loop */
            result += 100;
        } else {
            /* Alternative path creates divergence */
            result -= 50;
        }
        
        i += (i % 7) + 1;  /* Variable increment */
    }
    
    /* Second loop that overlaps with first loop's blocks */
    int k = ARRAY_SIZE / 4;
    do {
        arr[k] = arr[k] * 3;
        result += arr[k];
        
        /* Shared structure with first loop */
        if (k % 3 == 0) {
            for (int j = 0; j < 5; ++j) {
                arr[k + j] -= j;
                result -= j;
            }
        }
        
        k--;
    } while (k > 0 && result < 10000);
    
    printf("Partial overlap result: %d\n", result);
}

/* Function with sibling loops (same nesting level) */
void test_sibling_loops(int *__restrict arr1, int *__restrict arr2) {
    int total = 0;
    
    /* Outer loop containing sibling inner loops */
    for (int outer = 0; outer < 10; ++outer) {
        /* First inner loop - sibling to second */
        #pragma GCC unroll 2
        for (int i = 0; i < 8; ++i) {
            int idx = outer * 8 + i;
            if (idx < ARRAY_SIZE) {
                arr1[idx] = idx * outer;
                total += arr1[idx];
                
                /* Complex condition with short-circuit */
                if (idx % 2 == 0 || (outer > 5 && i < 4)) {
                    total += 1;
                    goto sibling_common;  /* goto creates shared block */
                }
                
                total -= 1;
            }
            
            sibling_common:
            if (total < 0) total = 0;
        }
        
        /* Second inner loop - sibling to first, shares no blocks */
        int j = 7;
        while (j >= 0) {
            int idx = outer * 8 + j;
            if (idx < ARRAY_SIZE) {
                arr2[idx] = idx / (outer + 1);
                total += arr2[idx];
                
                /* Different control structure */
                switch (j % 3) {
                    case 0: total += 10; break;
                    case 1: total += 20; break;
                    case 2: total += 30; break;
                }
            }
            j--;
        }
        
        /* Infinite loop with conditional break */
        for (;;) {
            total += outer;
            if (total > 5000) break;
            
            /* Nested infinite loop */
            int counter = 0;
            while (1) {
                counter++;
                total += counter;
                if (counter > 3 || total > 6000) break;
            }
            
            if (outer % 2 == 0) break;
        }
    }
    
    printf("Sibling loops total: %d\n", total);
}

/* Recursive function creating loop-like structure */
int __attribute__((noinline)) recursive_loop(int *arr, int depth, int idx) {
    if (depth <= 0 || idx >= ARRAY_SIZE) return 0;
    
    int sum = arr[idx];
    
    /* Tail recursion creates loop-like flow */
    if (depth % 2 == 0) {
        for (int i = 0; i < 3; ++i) {
            sum += recursive_loop(arr, depth - 1, idx + i);
        }
    } else {
        int j = 0;
        while (j < 2) {
            sum += recursive_loop(arr, depth - 1, idx + j * 2);
            j++;
        }
    }
    
    return sum;
}

/* Main test driver */
int main() {
    /* Initialize data arrays */
    int *array1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *array2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        array1[i] = i % 100;
        array2[i] = (i * 3) % 100;
    }
    
    printf("Testing hardware do-loop optimization patterns...\n");
    
    /* Test 1: Perfectly nested loops */
    test_perfect_nesting(array1);
    
    /* Test 2: Partially overlapping loops */
    test_partial_overlap(array2);
    
    /* Test 3: Sibling loops */
    test_sibling_loops(array1, array2);
    
    /* Test 4: Inlined function with complex loop */
    int chunk_result = 0;
    for (int chunk = 0; chunk < ARRAY_SIZE; chunk += 64) {
        process_chunk(array1, array2, chunk, chunk + 64);
        chunk_result += array1[chunk];
    }
    printf("Inlined chunk result: %d\n", chunk_result);
    
    /* Test 5: Recursive loop-like structure */
    int recursive_sum = recursive_loop(array1, 5, 0);
    printf("Recursive sum: %d\n", recursive_sum);
    
    /* Final computation to prevent optimization */
    int final_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        final_sum += array1[i] + array2[i];
    }
    printf("Final sum: %d\n", final_sum);
    
    free(array1);
    free(array2);
    
    return 0;
}
