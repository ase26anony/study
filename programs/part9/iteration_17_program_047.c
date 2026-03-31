/* 
 * Complex loop nesting test program targeting hw-doloop.cc uncovered lines
 * Lines 429-436: bitmap intersection logic for loop containment analysis
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define SIZE 1024
#define INNER_SIZE 64
#define DEPTH 8

/* Force inlining to create complex CFG within loops */
__attribute__((always_inline)) 
static inline int process_chunk(int *arr, int start, int end, int mod) {
    int sum = 0;
    /* Loop with multiple exit points */
    for (int i = start; i < end; i += mod) {
        if (i % 3 == 0) {
            sum += arr[i] * 2;
            if (sum > 1000) goto early_exit;
        } else if (i % 7 == 0) {
            sum -= arr[i];
            continue;  /* Skip rest of iteration */
        } else {
            sum += arr[i];
        }
        
        /* Nested switch creating multiple basic blocks */
        switch (i % 5) {
            case 0: sum += 1; break;
            case 1: sum += arr[i] % 2; break;
            case 2: sum *= 2; break;
            case 3: sum /= (arr[i] % 10 + 1); break;
            case 4: sum -= 3; break;
        }
        
        early_exit:;
    }
    return sum;
}

/* Helper with loop that will be inlined */
__attribute__((always_inline))
static inline void matrix_process(int n, int m, int matrix[][INNER_SIZE], int *result) {
    /* Partially overlapping loops - shares some blocks but not all */
    int i = 0, j = 0;
    
    /* First loop with complex condition */
    while (i < n && j < m) {
        if (i % 2 == 0) {
            /* Inner loop with early exit */
            for (int k = 0; k < INNER_SIZE; k++) {
                result[i] += matrix[i][k];
                if (matrix[i][k] < 0) break;
                if (k % 4 == 0) continue;
                result[i] -= matrix[j][k % m];
            }
            i += 2;
        } else {
            /* Different path with do-while */
            do {
                result[j] *= matrix[i % n][j % m];
                j++;
            } while (j < m && result[j] < 10000);
            i++;
        }
    }
}

/* Function with perfectly nested loops */
static int perfect_nesting(int *arr, int n) {
    int total = 0;
    
    /* Outer loop */
    for (int i = 0; i < n; i++) {
        /* Middle loop - proper subset of outer */
        for (int j = 0; j < i; j++) {
            /* Innermost loop - proper subset of middle */
            for (int k = 0; k < j; k++) {
                total += arr[i] * arr[j] + arr[k];
                if (total % 7 == 0) {
                    total /= 2;
                    continue;  /* Skip to next k iteration */
                }
                /* Conditional creating divergent paths */
                if (k % 3 == 0) {
                    total -= arr[i];
                } else if (k % 3 == 1) {
                    total += arr[j];
                } else {
                    total *= 2;
                }
            }
            
            /* Additional block in middle loop only */
            if (j % 2 == 0) {
                total += process_chunk(arr, 0, j, 2);
            }
        }
        
        /* Outer loop unique block */
        total += arr[i] * i;
    }
    
    return total;
}

/* Function with sibling loops (same nesting level) */
static int sibling_loops(int *a, int *b, int n) {
    int sum_a = 0, sum_b = 0;
    
    /* First sibling loop */
    for (int i = 0; i < n; i += 2) {
        sum_a += a[i];
        if (i % 4 == 0) {
            sum_a += b[i % n];
            goto skip_block;  /* Create non-contiguous blocks */
        }
        sum_a *= 2;
        skip_block:;
    }
    
    /* Second sibling loop - shares no blocks with first */
    int j = 1;
    while (j < n) {
        sum_b += b[j];
        if (sum_b > 1000) {
            sum_b /= 2;
            j += 3;
            continue;
        }
        j += 1;
    }
    
    return sum_a + sum_b;
}

/* Function with partially overlapping loops */
static int overlapping_loops(int *arr, int n) {
    int result = 0;
    int i = 0;
    
    /* Loop A */
    while (i < n / 2) {
        result += arr[i];
        if (arr[i] < 0) {
            result -= arr[i] * 2;
            i += 2;
            continue;
        }
        i++;
    }
    
    /* Loop B - overlaps with A in some blocks but not all */
    for (int j = n / 4; j < 3 * n / 4; j++) {
        result += arr[j] * 3;
        /* Shared block pattern with loop A */
        if (arr[j] < 0) {
            result -= arr[j] * 2;
            continue;
        }
        /* Unique block to loop B */
        result += j;
    }
    
    return result;
}

/* Infinite loop with conditional breaks */
static int infinite_loop_test(int *arr, int n) {
    int count = 0;
    int value = 0;
    
    for (;;) {
        value += arr[count % n];
        
        /* Multiple break points at different depths */
        if (value > 1000) {
            break;
        }
        
        /* Nested infinite loop */
        for (int i = 0; ; i++) {
            value -= i;
            if (i > 10 || value < -500) {
                if (value < -500) break;
                if (i > 10) continue;
            }
            
            /* Deeply nested conditional */
            if (i % 2 == 0) {
                value += process_chunk(arr, 0, i % n, 1);
                if (value > 2000) goto outer_break;
            }
        }
        
        count++;
        if (count > 100) break;
    }
    outer_break:
    
    return value;
}

/* Recursive function creating loop-like structure */
static int recursive_loop(int *arr, int depth, int idx) {
    if (depth <= 0) return arr[idx % SIZE];
    
    int sum = 0;
    /* Loop within recursion */
    for (int i = 0; i < depth; i++) {
        sum += recursive_loop(arr, depth - 1, idx + i);
        if (sum % 5 == 0) {
            sum += i;
            continue;
        }
        sum *= 2;
    }
    
    return sum;
}

/* Main test driver */
int main(void) {
    /* Initialize test data */
    int *data = malloc(SIZE * sizeof(int));
    int matrix[DEPTH][INNER_SIZE];
    int results[10] = {0};
    
    /* Fill with pseudo-random but deterministic values */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (i * 13 + 7) % 100;
    }
    
    for (int i = 0; i < DEPTH; i++) {
        for (int j = 0; j < INNER_SIZE; j++) {
            matrix[i][j] = (i * 17 + j * 23) % 50;
        }
    }
    
    printf("Starting complex loop nesting tests...\n");
    
    /* Test 1: Perfect nesting (bitmap_intersect_compl_p both ways false) */
    results[0] = perfect_nesting(data, SIZE);
    printf("Perfect nesting result: %d\n", results[0]);
    
    /* Test 2: Sibling loops (bitmap_intersect_p false) */
    results[1] = sibling_loops(data, data + SIZE/2, SIZE/2);
    printf("Sibling loops result: %d\n", results[1]);
    
    /* Test 3: Partially overlapping loops 
       (bitmap_intersect_compl_p true for one direction) */
    results[2] = overlapping_loops(data, SIZE);
    printf("Overlapping loops result: %d\n", results[2]);
    
    /* Test 4: Infinite loops with breaks */
    results[3] = infinite_loop_test(data, SIZE);
    printf("Infinite loop result: %d\n", results[3]);
    
    /* Test 5: Matrix processing with inlined loops */
    matrix_process(DEPTH, INNER_SIZE, matrix, results + 4);
    printf("Matrix process result: %d\n", results[4]);
    
    /* Test 6: Recursive loop-like structure */
    results[5] = recursive_loop(data, 5, 0);
    printf("Recursive loop result: %d\n", results[5]);
    
    /* Test 7: Mixed all patterns */
    int final = 0;
    for (int i = 0; i < 6; i++) {
        final += results[i];
        /* Loop with hardware optimization hints */
        #pragma GCC unroll 4
        for (int j = 0; j < i; j++) {
            final += (results[j] % 2 == 0) ? results[j] : -results[j];
        }
    }
    
    printf("Final accumulated result: %d\n", final);
    
    free(data);
    return final != 0 ? 0 : 1;
}
