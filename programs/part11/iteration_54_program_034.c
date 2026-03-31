/* test_hw_doloop.c - Test program to cover GCC hw-doloop pass bitmap intersection logic */
/* Compile with: gcc -O2 -march=armv7-a -fdump-rtl-all -fprofile-arcs -ftest-coverage test_hw_doloop.c -o test_hw_doloop */

#include <stdint.h>
#include <stdlib.h>

/* Prevent optimization of empty loops */
static volatile int sink;

/* Pattern A: Perfectly nested loops - inner loop is subset of outer loop */
__attribute__((noinline, target("thumb")))
void perfect_nesting(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Outer loop */
    for (i = 0; i < n; i++) {
        /* Create additional basic block in outer loop */
        if (__builtin_expect(i & 1, 0)) {
            sink = i;
        }
        
        /* Inner loop - perfectly nested */
        for (j = 0; j < m; j++) {
            sum += i * j;
            /* Split inner loop block */
            if (__builtin_expect(j & 1, 0)) {
                sink = j;
            }
        }
        
        /* Another block in outer loop after inner loop */
        if (__builtin_expect(i % 3 == 0, 0)) {
            sink = sum;
        }
    }
    
    sink = sum;
}

/* Pattern B: Partially overlapping loops - share some blocks but not subsets */
__attribute__((noinline, target("thumb")))
void partial_overlap(int n, int m) {
    int i = 0, j = 0;
    int sum = 0;
    
    /* Shared setup block */
    int shared = n * m;
    sink = shared;
    
    /* First loop */
    i = 0;
    while (i < n) {
        sum += i;
        /* Shared computation block */
        if (shared > 0) {
            sink = shared--;
        }
        
        /* Conditional second loop inside first */
        if (i % 2 == 0) {
            /* Second loop that shares the 'if (shared > 0)' block */
            for (j = 0; j < m; j++) {
                sum -= j;
                /* This block is only in second loop */
                if (j % 3 == 0) {
                    sink = j;
                }
            }
        }
        i++;
    }
    
    /* Additional loop that overlaps with first through shared variable */
    do {
        sum += shared;
        shared++;
    } while (shared < 10);
    
    sink = sum;
}

/* Pattern C: Sequential loops with shared preheader/setup */
__attribute__((noinline, target("thumb")))
void sequential_loops(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Shared setup block - may be included in both loop bitmaps */
    int setup = n + m;
    sink = setup;
    
    /* First loop */
    for (i = 0; i < n; i++) {
        sum += i * setup;
        /* Early exit creates additional blocks */
        if (i == n/2) {
            break;
        }
    }
    
    /* Shared intermediate computation */
    setup = sum % 100;
    
    /* Second loop - shares setup block through control flow */
    j = 0;
    while (j < m) {
        sum -= j * setup;
        j++;
        
        /* Nested mini-loop */
        int k = 0;
        do {
            sum += k;
            k++;
        } while (k < 3);
    }
    
    sink = sum;
}

/* Pattern D: Complex nested loops with multiple exits */
__attribute__((noinline, target("thumb")))
void complex_nesting(int n, int m) {
    int i, j, k;
    int sum = 0;
    
    /* Triple nested loop */
    for (i = 0; i < n; i++) {
        /* Middle loop with early exit */
        for (j = 0; j < m; j++) {
            if (j == m/2) {
                /* Early exit from middle loop */
                break;
            }
            
            /* Innermost loop */
            for (k = 0; k < 5; k++) {
                sum += i * j * k;
                /* Conditional continue */
                if (k % 2 == 0) {
                    continue;
                }
                sum -= 1;
            }
            
            /* Another block in middle loop */
            if (j % 3 == 0) {
                sink = j;
            }
        }
        
        /* Outer loop continues */
        if (i % 4 == 0) {
            sink = i;
        }
    }
    
    /* Additional disjoint loop */
    for (i = 0; i < 10; i++) {
        sum += i;
    }
    
    sink = sum;
}

/* Pattern E: Loops with goto creating irregular control flow */
__attribute__((noinline, target("thumb")))
void irregular_loops(int n) {
    int i = 0;
    int sum = 0;
    
    /* Loop with goto creating overlapping blocks */
    start_loop:
    if (i >= n) goto end_loop;
    
    sum += i;
    i++;
    
    /* Conditional jump to shared block */
    if (i % 3 == 0) {
        goto shared_block;
    }
    
    /* Normal loop continuation */
    goto start_loop;
    
    shared_block:
    sink = i;
    goto start_loop;
    
    end_loop:
    
    /* Another loop that shares the shared_block */
    for (i = 0; i < 5; i++) {
        sum *= 2;
        if (i == 2) {
            goto shared_block2;
        }
    }
    
    shared_block2:
    sink = sum;
    
    /* Loop that includes shared_block2 */
    while (sum > 0) {
        sum--;
        if (sum % 10 == 0) {
            goto shared_block2;
        }
    }
}

/* Main function to ensure all patterns are executed */
int main(void) {
    int iterations = 100;
    int inner_iterations = 50;
    
    /* Execute each pattern multiple times with different parameters */
    for (int run = 0; run < 10; run++) {
        perfect_nesting(iterations + run, inner_iterations + run);
        partial_overlap(iterations + run, inner_iterations + run);
        sequential_loops(iterations + run, inner_iterations + run);
        complex_nesting(iterations + run, inner_iterations + run);
        irregular_loops(iterations + run);
    }
    
    /* Compile-time check to ensure optimization */
    static_assert(sizeof(int) == 4, "int must be 4 bytes for consistent behavior");
    
    return 0;
}
