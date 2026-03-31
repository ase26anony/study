/* test_hw_doloop.c - Test program for GCC hardware loop optimization pass */
/* Compile with: gcc -O2 -march=armv7-a -fdump-rtl-all -fprofile-arcs -ftest-coverage test_hw_doloop.c -o test_hw_doloop */

#include <stdint.h>
#include <stdlib.h>

/* Prevent optimization of empty loops */
static volatile int sink;

/* Pattern A: Perfectly nested loops - inner loop is subset of outer loop */
__attribute__((noinline))
void perfect_nesting(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Outer loop */
    for (i = 0; i < n; i++) {
        /* Add some basic blocks inside outer but outside inner */
        if (__builtin_expect(i & 1, 0)) {
            sum += i;
        }
        
        /* Inner loop - perfectly nested */
        for (j = 0; j < m; j++) {
            /* Split inner loop into multiple basic blocks */
            if (__builtin_expect(j & 1, 1)) {
                sum += i * j;
            } else {
                sum += i + j;
            }
        }
        
        /* More code in outer loop after inner */
        if (__builtin_expect(i % 3 == 0, 0)) {
            sum -= 1;
        }
    }
    
    sink = sum; /* Prevent dead code elimination */
}

/* Pattern B: Partially overlapping loops with shared blocks but not perfect nesting */
__attribute__((noinline))
void partial_overlap(int n, int m) {
    int i = 0, j = 0;
    int sum = 0;
    int shared_counter = 0;
    
    /* Shared setup block */
    if (n > 0 && m > 0) {
        shared_counter = 1;
    }
    
    /* First loop */
    while (i < n) {
        /* Shared block between loops */
        if (shared_counter) {
            sum += i;
        }
        
        /* Unique to first loop */
        if (__builtin_expect(i % 2 == 0, 1)) {
            sum *= 2;
        }
        
        i++;
        
        /* Conditional second loop inside first - creates partial overlap */
        if (i == n/2) {
            /* Second loop that shares some blocks with first */
            for (j = 0; j < m; j++) {
                /* Reuse the shared block */
                if (shared_counter) {
                    sum += j;
                }
                
                /* Unique to second loop */
                if (__builtin_expect(j % 3 == 0, 0)) {
                    sum /= 2;
                }
            }
        }
    }
    
    sink = sum;
}

/* Pattern C: Sequential loops with shared preheader/setup */
__attribute__((noinline))
void sequential_loops(int n, int m) {
    int i, j;
    int sum = 0;
    volatile int* arr = (volatile int*)malloc(n * sizeof(int));
    
    if (!arr) return;
    
    /* Shared setup block */
    for (i = 0; i < n; i++) {
        arr[i] = i * 2;
    }
    
    /* First loop */
    i = 0;
    do {
        sum += arr[i];
        
        /* Split block with always-true condition */
        if (__builtin_expect(1, 1)) {
            sum += i;
        }
        
        i++;
    } while (i < n);
    
    /* Second loop - sequential but shares setup block */
    for (j = 0; j < m; j++) {
        /* Different computation to avoid merging */
        sum -= j;
        
        /* Early exit creates different block structure */
        if (j > m/2) {
            break;
        }
        
        /* Another split */
        if (__builtin_expect(j & 1, 0)) {
            sum += arr[j % n];
        }
    }
    
    free((void*)arr);
    sink = sum;
}

/* Pattern D: Complex nested loops with early exits */
__attribute__((noinline))
void complex_nesting(int n, int m, int k) {
    int i, j, l;
    int sum = 0;
    
    /* Outer loop */
    for (i = 0; i < n; i++) {
        /* Middle loop with early exit */
        j = 0;
        while (j < m) {
            /* Innermost loop */
            for (l = 0; l < k; l++) {
                sum += i * j * l;
                
                /* Early exit from innermost */
                if (l > k/2) {
                    sum += 1000;
                    break;
                }
            }
            
            j++;
            
            /* Early exit from middle */
            if (j > m/3) {
                sum -= 500;
                break;
            }
        }
        
        /* Another inner loop at same level */
        for (j = 0; j < i; j++) {
            sum += j;
        }
    }
    
    sink = sum;
}

/* Pattern E: Loops with irregular control flow using goto */
__attribute__((noinline))
void irregular_loops(int n) {
    int i = 0, j = 0;
    int sum = 0;
    
    /* Loop with goto creating overlapping blocks */
    for (i = 0; i < n; i++) {
        if (i % 2 == 0) {
            sum += i;
            goto shared_block;
        }
        
        sum *= 2;
        
    shared_block:
        /* This block is shared between paths */
        sum += 1;
        
        /* Nested loop entered conditionally */
        if (i % 3 == 0) {
            for (j = 0; j < i; j++) {
                sum += j;
                if (j > 5) goto outer_continue;
            }
        }
        
    outer_continue:
        continue;
    }
    
    sink = sum;
}

/* Pattern F: Multiple functions with different loop types to increase coverage */
__attribute__((noinline, target("thumb")))
void thumb_specific_loops(int n) {
    int i;
    int sum = 0;
    
    /* Simple loop that should be optimized to hardware loop */
    for (i = 0; i < n; i++) {
        sum += i * 2;
    }
    
    /* Another loop */
    i = n;
    while (i-- > 0) {
        sum -= i;
    }
    
    sink = sum;
}

/* Main function that calls all patterns */
int main(void) {
    int iterations = 100;
    
    /* Call each function with different parameters */
    perfect_nesting(iterations, 50);
    partial_overlap(iterations, 30);
    sequential_loops(iterations, 40);
    complex_nesting(20, 15, 10);
    irregular_loops(iterations);
    thumb_specific_loops(iterations);
    
    /* Compile-time check to ensure optimization */
    static_assert(sizeof(int) == 4, "int must be 4 bytes");
    
    return sink != 0 ? 0 : 1;
}
