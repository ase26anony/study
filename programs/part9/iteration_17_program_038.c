/* 
 * Complex loop nesting test to exercise hardware do-loop optimization logic
 * Specifically targets bitmap intersection analysis in GCC's hw-dolopp.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define SIZE 1024
#define ITERATIONS 100

/* Force inlining to create complex control flow within loops */
__attribute__((always_inline)) 
static inline int process_chunk(int *restrict dst, const int *restrict src, 
                                int start, int end, int threshold) {
    int sum = 0;
    
    /* Loop with multiple exit points and conditional blocks */
    for (int i = start; i < end; i += (i % 3) + 1) {  /* Variable stride */
        if (src[i] > threshold) {
            dst[i] = src[i] * 2;
            sum += dst[i];
            
            /* Early exit creates additional basic block */
            if (dst[i] > 1000) {
                break;
            }
        } else if (src[i] < -threshold) {
            dst[i] = src[i] / 2;
            sum += dst[i];
            
            /* Continue to different point */
            if (i % 2 == 0) {
                continue;
            }
        } else {
            dst[i] = src[i];
            sum += src[i];
        }
        
        /* Additional block only executed sometimes */
        if (i % 7 == 0) {
            dst[i] += i;
        }
    }
    
    return sum;
}

/* Function with nested loops that will be inlined */
__attribute__((always_inline))
static inline void nested_matrix_process(int matrix[SIZE][SIZE], int factor) {
    /* Outer loop with complex condition */
    for (int i = 0; i < SIZE && factor > 0; i += factor) {
        int j = 0;
        
        /* Inner while loop with multiple conditions */
        while (j < SIZE && matrix[i][j] != 0) {
            /* Switch statement inside loop creates multiple blocks */
            switch (matrix[i][j] % 4) {
                case 0:
                    matrix[i][j] += factor;
                    /* Fall through to create shared block */
                case 1:
                    matrix[i][j] *= 2;
                    break;
                case 2:
                    matrix[i][j] -= factor;
                    /* goto creates non-contiguous block ranges */
                    if (matrix[i][j] < 0) goto negative_handler;
                    break;
                case 3:
                    matrix[i][j] /= (factor > 0 ? factor : 1);
                    break;
            }
            
            /* Label for goto target */
            negative_handler:
            if (matrix[i][j] < -100) {
                matrix[i][j] = -100;
            }
            
            /* Loop-carried dependency with continue */
            if (j % 3 == 0) {
                j += 2;
                continue;
            }
            
            j += 1;
        }
        
        /* Another inner loop at same level - sibling to while loop */
        for (int k = 0; k < i; ++k) {
            /* Partially overlapping blocks with outer loop */
            if (k % 2 == 0) {
                matrix[i][k] += matrix[k][i];
            } else {
                matrix[i][k] -= matrix[k][i];
            }
            
            /* Infinite loop with conditional break */
            for (;;) {
                if (matrix[i][k] > 10000) {
                    matrix[i][k] = 10000;
                    break;
                }
                if (matrix[i][k] < -10000) {
                    matrix[i][k] = -10000;
                    break;
                }
                /* Small computation to prevent infinite optimization */
                matrix[i][k] += (matrix[i][k] > 0) ? -1 : 1;
            }
        }
    }
}

/* Recursive function creating loop-like structure */
static int recursive_loop(int *arr, int start, int end, int depth) {
    if (start >= end || depth <= 0) {
        return 0;
    }
    
    int mid = (start + end) / 2;
    int sum = 0;
    
    /* Do-while with nested if for early exit */
    do {
        sum += arr[mid];
        
        if (arr[mid] % 2 == 0) {
            /* Process left half */
            sum += recursive_loop(arr, start, mid, depth - 1);
        } else {
            /* Process right half */
            sum += recursive_loop(arr, mid + 1, end, depth - 1);
        }
        
        mid += (depth % 3) - 1;  /* Variable modification */
        
        /* Multiple condition checks */
    } while (mid > start && mid < end && sum < 1000000 && depth-- > 0);
    
    return sum;
}

/* Test 1: Perfectly nested loops with subset blocks */
static int test_perfect_nesting(void) {
    int data[SIZE];
    int result[SIZE];
    int total = 0;
    
    /* Initialize data */
    for (int i = 0; i < SIZE; ++i) {
        data[i] = i - SIZE/2;
    }
    
    /* Outer loop */
    for (int outer = 0; outer < SIZE/16; ++outer) {
        /* Middle loop - perfectly nested */
        for (int middle = outer; middle < SIZE/8; ++middle) {
            /* Inner loop - proper subset of middle loop blocks */
            #pragma GCC unroll 2
            for (int inner = middle; inner < SIZE/4; ++inner) {
                /* Complex expression with builtin expect */
                if (__builtin_expect(inner % 16 == 0, 0)) {
                    result[inner] = data[inner] * 3;
                } else {
                    result[inner] = data[inner] + outer + middle;
                }
                
                total += result[inner];
                
                /* Conditional continue creates additional block */
                if (inner % 7 == 0) {
                    continue;
                }
                
                /* Additional computation in some iterations */
                result[inner] += (outer * middle) % 17;
            }
            
            /* Block only in middle loop, not in inner loop */
            if (middle % 5 == 0) {
                total -= data[middle];
            }
        }
        
        /* Block only in outer loop */
        total += outer * 2;
    }
    
    return total;
}

/* Test 2: Partially overlapping loops */
static int test_partial_overlap(void) {
    int matrix[64][64];
    int total = 0;
    
    /* Initialize */
    for (int i = 0; i < 64; ++i) {
        for (int j = 0; j < 64; ++j) {
            matrix[i][j] = (i * 64 + j) % 100;
        }
    }
    
    /* First loop covering rows 0-31 */
    for (int i = 0; i < 32; ++i) {
        for (int j = 0; j < 64; ++j) {
            matrix[i][j] += i;
            total += matrix[i][j];
        }
    }
    
    /* Second loop covering rows 16-47 - overlaps with first */
    for (int i = 16; i < 48; ++i) {
        for (int j = 16; j < 48; ++j) {
            matrix[i][j] *= 2;
            total -= matrix[i][j];
            
            /* if-else creating divergent paths */
            if (matrix[i][j] > 100) {
                matrix[i][j] = 100;
            } else if (matrix[i][j] < -100) {
                matrix[i][j] = -100;
            } else {
                matrix[i][j] += j;
            }
        }
    }
    
    /* Third loop covering columns 0-31 only */
    for (int j = 0; j < 32; ++j) {
        for (int i = 0; i < 64; ++i) {
            matrix[i][j] += matrix[j][i];
            total += matrix[i][j] % 19;
        }
    }
    
    return total;
}

/* Test 3: Sibling loops with shared outer loop */
static int test_sibling_loops(void) {
    int data[3][SIZE];
    int total = 0;
    
    /* Initialize 2D array */
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            data[i][j] = (i * SIZE + j) % 255;
        }
    }
    
    /* Outer loop containing sibling inner loops */
    for (int row = 0; row < 3; ++row) {
        /* First sibling loop - processes even indices */
        for (int col = 0; col < SIZE; col += 2) {
            data[row][col] = data[row][col] * 3 + 1;
            total += data[row][col];
            
            /* Nested if with goto */
            if (data[row][col] > 1000) {
                goto reduce_value;
            }
            continue;
            
            reduce_value:
            data[row][col] /= 2;
        }
        
        /* Second sibling loop - processes odd indices */
        for (int col = 1; col < SIZE; col += 2) {
            data[row][col] = data[row][col] / 3 - 1;
            total += data[row][col];
            
            /* Switch with multiple cases */
            switch (data[row][col] % 5) {
                case 0:
                case 1:
                    data[row][col] += row;
                    break;
                case 2:
                    data[row][col] -= col;
                    /* Fall through */
                case 3:
                    data[row][col] *= 2;
                    break;
                default:
                    data[row][col] = 0;
            }
        }
        
        /* Third sibling loop - processes all with stride */
        for (int col = 0; col < SIZE; col += 3) {
            data[row][col] += data[(row + 1) % 3][col];
            total += data[row][col] % 37;
        }
    }
    
    return total;
}

/* Test 4: Complex control flow with function inlining */
static int test_inlined_loops(void) {
    int src[SIZE];
    int dst[SIZE];
    int total = 0;
    
    /* Initialize source array */
    for (int i = 0; i < SIZE; ++i) {
        src[i] = (i * 7) % 100 - 50;
    }
    
    /* Call inline function multiple times with different parameters */
    total += process_chunk(dst, src, 0, SIZE/4, 10);
    total += process_chunk(dst, src, SIZE/4, SIZE/2, 20);
    total += process_chunk(dst, src, SIZE/2, 3*SIZE/4, 30);
    total += process_chunk(dst, src, 3*SIZE/4, SIZE, 40);
    
    /* Process results with another loop */
    for (int i = 0; i < SIZE; i += 4) {
        /* Loop with multiple induction variables */
        for (int j = i, k = 0; j < i + 4 && k < 4; ++j, ++k) {
            dst[j] += total % 100;
            
            /* Conditional break at different depths */
            if (dst[j] > 10000) {
                break;
            }
        }
        
        total += dst[i] + dst[i+1] + dst[i+2] + dst[i+3];
    }
    
    return total;
}

/* Test 5: Mixed loop types with recursion */
static int test_mixed_recursive(void) {
    int data[SIZE];
    int total = 0;
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; ++i) {
        data[i] = (i * 13) % 100;
    }
    
    /* While loop with complex condition */
    int idx = 0;
    while (idx < SIZE && total < 1000000) {
        /* Do-while inside while */
        int count = 0;
        do {
            data[idx] += count;
            total += data[idx];
            count++;
            
            if (data[idx] % 11 == 0) {
                /* Recursive call */
                total += recursive_loop(data, idx, 
                                       (idx + 16) < SIZE ? idx + 16 : SIZE, 
                                       3);
            }
        } while (count < 3 && idx < SIZE);
        
        idx += (data[idx] % 7) + 1;
    }
    
    /* Final processing loop */
    #pragma GCC unroll 1
    for (int i = 0; i < SIZE; ++i) {
        /* Multiple conditions joined */
        if (i % 2 == 0 || i % 3 == 0 || data[i] > 50) {
            data[i] = (data[i] * 2) % 1000;
            total += data[i];
        }
    }
    
    return total;
}

int main(void) {
    int total = 0;
    
    printf("Starting complex loop nesting tests...\n");
    
    /* Run all tests multiple times to ensure execution */
    for (int iter = 0; iter < ITERATIONS; ++iter) {
        total += test_perfect_nesting();
        total += test_partial_overlap();
        total += test_sibling_loops();
        total += test_inlined_loops();
        total += test_mixed_recursive();
        
        /* Additional top-level loop with matrix processing */
        int matrix[SIZE][SIZE];
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                matrix[i][j] = (i * 8 + j + iter) % 100;
            }
        }
        nested_matrix_process(matrix, (iter % 5) + 1);
        
        /* Accumulate some results */
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                total += matrix[i][j];
            }
        }
    }
    
    printf("Total result: %d\n", total);
    printf("(This value is non-deterministic due to complex loop interactions)\n");
    
    return total != 0 ? 0 : 1;
}
