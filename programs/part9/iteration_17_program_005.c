/* 
 * Complex loop nesting test program to trigger uncovered bitmap intersection logic
 * in GCC's hardware do-loop optimization (hw-doloop.cc lines 429-436)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define SIZE 1024
#define NEST_DEPTH 5

/* Force inlining to create complex control flow within loops */
__attribute__((always_inline)) 
static inline int process_chunk(int *arr, int start, int end, int mod) {
    int sum = 0;
    
    /* Loop with multiple exit points and complex control flow */
    for (int i = start; i < end; i += (i % 3) + 1) {
        if (arr[i] % mod == 0) {
            sum += arr[i];
            if (sum > 1000) goto early_exit;  /* Non-contiguous block creation */
        } else if (arr[i] % (mod + 1) == 0) {
            sum -= arr[i] / 2;
            continue;  /* Skip to different point */
        } else {
            /* Switch inside loop creates multiple basic blocks */
            switch (arr[i] % 4) {
                case 0:
                    sum += i;
                    break;
                case 1:
                    sum += arr[i] * 2;
                    /* Fall through to create shared block */
                case 2:
                    sum += 1;
                    break;
                default:
                    /* Nested while inside for loop */
                    int j = 0;
                    while (j < 3 && sum < 500) {
                        sum += j;
                        j++;
                    }
                    break;
            }
        }
        
        /* Another conditional creating divergent paths */
        if (i % 7 == 0) {
            sum *= 2;
        } else if (i % 5 == 0) {
            sum /= 2;
        }
        
        early_exit: ;  /* Label for goto */
    }
    
    return sum;
}

/* Function with perfectly nested loops - inner blocks are subset of outer */
__attribute__((always_inline))
static inline void perfect_nesting(int matrix[SIZE][SIZE]) {
    int total = 0;
    
    /* Outer loop */
    for (int i = 0; i < SIZE; i++) {
        /* Middle loop - perfectly nested */
        for (int j = 0; j < SIZE; j += 2) {
            /* Innermost loop - all blocks are subset of outer loops */
            for (int k = 0; k < NEST_DEPTH; k++) {
                matrix[i][j] += i * j + k;
                
                /* Conditional break creates multiple exit points */
                if (matrix[i][j] > 10000) {
                    break;
                }
                
                /* Continue with condition */
                if (k % 2 == 0) {
                    continue;
                }
                
                matrix[i][j] -= k;
            }
            
            /* Partial overlap: this block belongs to middle but not inner loop */
            if (j % 3 == 0) {
                matrix[i][j] *= 2;
            }
        }
        
        /* Block only in outer loop */
        total += i;
    }
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(total) : "memory");
}

/* Function with partially overlapping loops */
static void partial_overlap(int *arr1, int *arr2, int len) {
    int i = 0, j = 0;
    
    /* First loop with complex increment */
    while (i < len) {
        arr1[i] = i * 2;
        
        /* Second loop that starts inside first loop */
        if (i > len / 2) {
            do {
                arr2[j] = arr1[i] + j;
                j++;
                
                /* Conditional continue to different points */
                if (j % 4 == 0) continue;
                if (j % 5 == 0) continue;
                
                arr2[j] -= 1;
            } while (j < i && j < len);
        }
        
        /* Complex increment with variable step */
        i += (i % 7) + 1;
        
        /* Early exit from different points */
        if (arr1[i % len] > 5000) {
            break;
        }
    }
    
    /* Sibling loop to the while loop above */
    for (int k = 0; k < len; k++) {
        /* Shared block with previous loop's body */
        if (k < i) {
            arr1[k] += arr2[k % j];
        } else {
            /* Unique block to this loop */
            arr1[k] = -1;
        }
    }
}

/* Function with infinite loops and conditional breaks */
static int infinite_loops_complex(int *data, int size) {
    int result = 0;
    int attempts = 0;
    
    /* Infinite outer loop */
    for (;;) {
        attempts++;
        
        /* First inner infinite loop */
        while (1) {
            result += data[attempts % size];
            
            /* Multiple break conditions at different depths */
            if (result > 10000) goto outer_break;
            if (attempts > 50) break;
            
            /* Nested finite loop inside infinite loop */
            for (int i = 0; i < 3; i++) {
                result -= i;
                if (result < 0) {
                    /* Break to outer infinite loop */
                    goto continue_outer;
                }
            }
            
            /* Switch with goto labels */
            switch (attempts % 3) {
                case 0: goto case0_label;
                case 1: continue;
                case 2: break;
            }
            
            case0_label:
            result *= 2;
        }
        
        /* Different inner loop structure */
        int counter = 0;
        do {
            result += counter;
            counter++;
            
            /* Conditional with early continue */
            if (counter % 2 == 0) {
                continue;
            }
            
            /* Another nested loop */
            for (int x = 0; x < 2; x++) {
                result -= x;
            }
        } while (counter < 5 && attempts < 100);
        
        continue_outer: ;
        
        if (attempts > 100) break;
    }
    outer_break:
    
    return result;
}

/* Recursive function creating loop-like structures */
__attribute__((noinline))
static int recursive_loop_like(int *arr, int depth, int idx) {
    if (depth <= 0 || idx >= SIZE) return 0;
    
    int sum = 0;
    
    /* Loop inside recursion */
    for (int i = 0; i < depth; i++) {
        sum += arr[idx + i];
        
        /* Recursive call in loop - creates complex CFG */
        if (i % 2 == 0) {
            sum += recursive_loop_like(arr, depth - 1, idx + i);
        }
        
        /* Conditional break */
        if (sum > 1000) {
            break;
        }
    }
    
    /* Tail recursion */
    if (depth > 1) {
        sum += recursive_loop_like(arr, depth - 1, idx + 1);
    }
    
    return sum;
}

/* Main test driver with hardware loop optimization hints */
int main(void) {
    /* Initialize data with restrict qualifiers for better loop analysis */
    int *__restrict data1 = (int*)malloc(SIZE * sizeof(int));
    int *__restrict data2 = (int*)malloc(SIZE * sizeof(int));
    int (*__restrict matrix)[SIZE] = (int(*)[SIZE])malloc(SIZE * SIZE * sizeof(int));
    
    if (!data1 || !data2 || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        data1[i] = i;
        data2[i] = SIZE - i;
    }
    
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = i * j;
        }
    }
    
    int total = 0;
    
    printf("Starting complex loop nesting tests...\n");
    
    /* Test 1: Perfect nesting with hardware loop hints */
    #pragma GCC unroll 2
    for (int iter = 0; iter < 10; iter++) {
        perfect_nesting(matrix);
        total += matrix[iter % SIZE][0];
    }
    
    /* Test 2: Partial overlap loops */
    partial_overlap(data1, data2, SIZE);
    
    /* Test 3: Process chunks with complex control flow */
    for (int chunk = 0; chunk < SIZE; chunk += 64) {
        /* Use __builtin_expect for loop bound hints */
        if (__builtin_expect(chunk + 64 <= SIZE, 1)) {
            total += process_chunk(data1, chunk, chunk + 64, 7);
        }
    }
    
    /* Test 4: Infinite loops with breaks */
    total += infinite_loops_complex(data2, SIZE);
    
    /* Test 5: Recursive loop-like structures */
    total += recursive_loop_like(data1, NEST_DEPTH, 0);
    
    /* Additional stress: Mixed loop types in sequence */
    int i = 0;
    while (i < SIZE) {
        /* do-while inside while */
        int j = 0;
        do {
            data1[i] += data2[j];
            j++;
            
            /* for loop inside do-while */
            for (int k = 0; k < 4; k++) {
                if (k % 2 == 0) {
                    data1[i] -= k;
                } else {
                    data1[i] += k * 2;
                }
            }
        } while (j < 5 && i + j < SIZE);
        
        i += (i % 11) + 1;
    }
    
    /* Final computation to use all results */
    int final_result = 0;
    for (int idx = 0; idx < SIZE; idx++) {
        final_result += data1[idx] + data2[idx];
        if (idx % 100 == 0) {
            final_result += matrix[idx % SIZE][idx % SIZE];
        }
    }
    
    final_result += total;
    
    printf("Final result: %d\n", final_result);
    printf("Loop nesting tests completed.\n");
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(matrix);
    
    return 0;
}
