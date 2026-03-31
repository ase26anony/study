/* 
 * Complex loop nesting test program to exercise hardware do-loop optimization logic
 * Specifically targets bitmap intersection analysis in GCC's hw-doloop.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define SIZE 1024
#define ITERATIONS 100

/* Force inlining to merge control flow graphs */
__attribute__((always_inline)) 
static inline int process_inner_loop(int *__restrict arr, int start, int end, int threshold) {
    int sum = 0;
    /* Complex inner loop with multiple exit points */
    for (int i = start; i < end; i += (i % 3) + 1) {  /* Variable stride */
        if (arr[i] > threshold) {
            sum += arr[i];
            if (sum > 1000) {
                /* Early exit creates new basic block */
                goto early_exit;
            }
        } else if (arr[i] < -threshold) {
            sum -= arr[i];
            /* Continue skips to different point */
            continue;
        } else {
            /* Switch creates multiple control flow paths */
            switch (arr[i] % 4) {
                case 0:
                    sum += 1;
                    break;
                case 1:
                    sum += 2;
                    /* Fall through to create shared blocks */
                case 2:
                    sum += 3;
                    break;
                default:
                    /* goto creates non-contiguous blocks */
                    if (arr[i] == 0) goto zero_handler;
                    sum += 4;
            }
        }
        
        /* Additional conditional creates divergent paths */
        if (i % 7 == 0) {
            sum *= 2;
        }
        
        zero_handler:
        /* Empty label for goto target */
        ;
    }
    
    early_exit:
    return sum;
}

/* Helper function with loop that will be inlined */
__attribute__((always_inline))
static void nested_helper(int *__restrict a, int *__restrict b, int n) {
    /* do-while with complex condition */
    int i = 0;
    do {
        if (i % 2 == 0) {
            /* Nested for loop inside do-while */
            for (int j = 0; j < 5 && j < n; ++j) {
                a[i] += b[j];
                /* Conditional continue */
                if (a[i] > 100) continue;
                a[i] -= 1;
            }
        } else {
            /* Infinite loop with conditional break */
            for (;;) {
                a[i] *= 2;
                if (a[i] > 1000 || i % 11 == 0) {
                    break;
                }
                /* Multiple break points */
                if (a[i] < 0) {
                    break;
                }
            }
        }
        
        /* Complex increment */
        i += (a[i] % 5) + 1;
    } while (i < n && i < SIZE);
}

/* Function with partially overlapping loops */
static void test_partial_overlap(int *__restrict data) {
    int i = 0;
    int j = SIZE / 2;
    
    /* First loop - processes first half */
    while (i < SIZE / 2) {
        data[i] = i * 2;
        
        /* Conditional that might jump into second loop's blocks */
        if (data[i] % 17 == 0) {
            /* This creates shared basic blocks */
            data[j] = data[i];
            j++;
        }
        
        i++;
    }
    
    /* Second loop - processes second half but shares some blocks */
    j = SIZE / 2;
    while (j < SIZE) {
        data[j] = data[j - SIZE/2] + 1;
        
        /* Shared block with first loop */
        if (data[j] % 17 == 0) {
            data[0] = j;
        }
        
        j++;
    }
}

/* Function with sibling loops (same nesting level) */
static void test_sibling_loops(int *__restrict arr1, int *__restrict arr2) {
    /* First sibling loop */
    for (int i = 0; i < SIZE; i += 2) {
        if (__builtin_expect(i % 8 == 0, 0)) {
            arr1[i] = i * 3;
        } else {
            arr1[i] = i * 2;
        }
        
        /* Complex condition with short-circuit */
        if (i > 0 && arr1[i] > arr1[i-1] || i == 0) {
            arr1[i] += 1;
        }
    }
    
    /* Second sibling loop - no block sharing with first */
    for (int i = 1; i < SIZE; i += 2) {
        switch (i % 6) {
            case 0:
                arr2[i] = 0;
                break;
            case 1:
            case 2:
                arr2[i] = 1;
                /* Fall through creates shared blocks within this loop */
            case 3:
                arr2[i] += 2;
                break;
            default:
                arr2[i] = 3;
        }
    }
}

/* Recursive function creating loop-like structure */
static int recursive_loop(int *arr, int depth, int idx) {
    if (depth <= 0 || idx >= SIZE) {
        return 0;
    }
    
    int sum = 0;
    
    /* Tail recursion with loop */
    for (int i = 0; i < 3 && idx + i < SIZE; ++i) {
        arr[idx + i] += depth;
        sum += arr[idx + i];
        
        /* Conditional recursion creates complex CFG */
        if (arr[idx + i] % 7 == 0) {
            sum += recursive_loop(arr, depth - 1, idx + i + 1);
        }
    }
    
    return sum + recursive_loop(arr, depth - 1, idx + 3);
}

/* Main test function with multiple nested loops */
static int test_complex_nesting(void) {
    int data[SIZE];
    int temp[SIZE];
    int result = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; ++i) {
        data[i] = i;
        temp[i] = SIZE - i;
    }
    
    /* Test 1: Perfectly nested loops */
    #pragma GCC unroll 2
    for (int outer = 0; outer < 10; ++outer) {
        int inner_sum = 0;
        
        /* Perfect nesting - inner blocks are subset of outer */
        for (int inner = outer; inner < SIZE; inner += outer + 1) {
            data[inner] += outer;
            
            /* Innermost loop */
            for (int k = 0; k < 5; ++k) {
                data[inner] += k;
                if (data[inner] > 1000) {
                    data[inner] = 1000;
                }
            }
            
            inner_sum += data[inner];
        }
        
        result += inner_sum;
    }
    
    /* Test 2: Partially overlapping loops */
    test_partial_overlap(data);
    
    /* Test 3: Sibling loops */
    test_sibling_loops(data, temp);
    
    /* Test 4: Inlined helper with complex loops */
    nested_helper(data, temp, SIZE / 2);
    
    /* Test 5: Process with inner loop function */
    for (int chunk = 0; chunk < SIZE; chunk += 64) {
        result += process_inner_loop(data, chunk, chunk + 64, 50);
    }
    
    /* Test 6: Recursive loop-like structure */
    result += recursive_loop(data, 5, 0);
    
    /* Test 7: Loop with multiple entry points via goto */
    int i = 0;
    entry_point_1:
    while (i < SIZE / 3) {
        data[i] += result;
        i++;
    }
    
    if (result % 2 == 0) {
        goto entry_point_2;
    }
    
    i = SIZE / 3;
    while (i < 2 * SIZE / 3) {
        data[i] -= result;
        i++;
    }
    
    entry_point_2:
    i = 2 * SIZE / 3;
    do {
        data[i] *= 2;
        if (data[i] > 10000) {
            /* Jump back to create interesting CFG */
            goto entry_point_1;
        }
        i++;
    } while (i < SIZE);
    
    /* Final aggregation to prevent optimization */
    int final_sum = 0;
    for (int idx = 0; idx < SIZE; ++idx) {
        final_sum += data[idx];
        /* Use builtin for hardware loop hint */
        if (__builtin_expect(idx % 128 == 0, 0)) {
            final_sum += temp[idx % 128];
        }
    }
    
    return result + final_sum;
}

int main(void) {
    int total_result = 0;
    
    /* Run multiple iterations to ensure loops execute */
    for (int iter = 0; iter < ITERATIONS; ++iter) {
        total_result += test_complex_nesting();
        
        /* Vary loop behavior slightly each iteration */
        if (iter % 10 == 0) {
            /* Trigger different optimization paths */
            printf("Iteration %d: %d\n", iter, total_result);
        }
    }
    
    printf("Final result: %d\n", total_result);
    return total_result != 0 ? 0 : 1;
}
