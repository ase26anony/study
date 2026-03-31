/* test_hw_doloop.c
 * Test program to cover uncovered lines in GCC's hw-doloop pass
 * Lines 429-436 in hw-doloop.cc involve bitmap intersection checks
 * for loop hierarchy construction
 */

#include <stdint.h>

/* Prevent optimization of empty loops */
static volatile int sink;

/* Pattern A: Perfectly nested loops - inner loop is subset of outer */
__attribute__((target("arch=armv7-a")))
void perfect_nesting(int n, int m) {
    int sum = 0;
    /* Outer loop */
    for (int i = 0; i < n; i++) {
        /* Split block to create more complex bitmap */
        if (__builtin_expect(i & 1, 0)) {
            sink = i;
        }
        
        /* Inner loop - perfectly nested */
        for (int j = 0; j < m; j++) {
            sum += i * j;
            /* Early exit affects block structure */
            if (j == m/2) {
                break;
            }
        }
        
        /* Another statement in outer loop */
        if (__builtin_expect(sum > 1000, 0)) {
            sink = sum;
        }
    }
    
    sink = sum;
}

/* Pattern B: Partially overlapping loops with shared blocks */
__attribute__((target("arch=armv7-a")))
void partial_overlap(int n, int m) {
    int i = 0, j = 0;
    int sum = 0;
    
    /* Shared setup block */
    int shared = n * 2;
    
    /* First loop */
    do {
        sum += i;
        i++;
        
        /* Conditional entry to second loop's blocks */
        if (i % 3 == 0) {
            /* This block belongs to both loop's bitmaps */
            j = 0;
            /* Second loop - partially overlapping */
            while (j < m) {
                sum += j;
                j++;
                /* Shared computation */
                if (j % 2 == 0) {
                    sum += shared;
                }
            }
        }
    } while (i < n);
    
    sink = sum;
}

/* Pattern C: Sequential loops with shared preheader */
__attribute__((target("arch=armv7-a")))
void sequential_loops(int n, int m) {
    int sum = 0;
    
    /* Shared preheader block */
    int setup = n + m;
    
    /* First loop */
    for (int i = 0; i < n; i++) {
        sum += i * setup;
        /* Split block */
        if (__builtin_expect(i == n/2, 0)) {
            sink = i;
        }
    }
    
    /* Shared block between loops */
    sum += setup;
    
    /* Second loop - sibling with shared blocks */
    for (int j = 0; j < m; j++) {
        sum += j * setup;
        /* Different block structure */
        if (j % 2 == 0) {
            sum -= 1;
        } else {
            sum += 1;
        }
    }
    
    sink = sum;
}

/* Pattern D: Complex nested loops with early exits */
__attribute__((target("arch=armv7-a")))
void complex_nesting(int n, int m, int k) {
    int total = 0;
    
    /* Outer loop */
    for (int i = 0; i < n; i++) {
        /* Middle loop */
        int j = 0;
        while (j < m) {
            /* Inner loop with early exit */
            for (int l = 0; l < k; l++) {
                total += i + j + l;
                if (total > 10000) {
                    /* Early exit affects bitmap */
                    goto middle_loop_continue;
                }
                /* Split block */
                if (__builtin_expect(l == k/2, 0)) {
                    sink = l;
                }
            }
            
            middle_loop_continue:
            j++;
            
            /* Another inner loop */
            int p = 0;
            do {
                total -= p;
                p++;
            } while (p < 5);
        }
        
        /* Conditional inner loop */
        if (i % 2 == 0) {
            for (int q = 0; q < i; q++) {
                total += q * 2;
            }
        }
    }
    
    sink = total;
}

/* Pattern E: Loops with irregular control flow using goto */
__attribute__((target("arch=armv7-a")))
void irregular_loops(int n) {
    int i = 0;
    int sum = 0;
    
    /* Loop with goto creating overlapping blocks */
    while (i < n) {
        sum += i;
        i++;
        
        if (i % 4 == 0) {
            /* Jump to shared block */
            goto shared_block;
        }
        
        continue;
        
        shared_block:
        sum += 100;
        /* This block is reachable from multiple paths */
    }
    
    /* Another loop that shares the shared_block */
    for (int j = 0; j < n/2; j++) {
        sum -= j;
        if (j % 3 == 0) {
            goto shared_block;
        }
    }
    
    sink = sum;
}

/* Main function to ensure all patterns are used */
int main(void) {
    /* Use different parameters to create varied loop structures */
    perfect_nesting(100, 50);
    partial_overlap(100, 50);
    sequential_loops(100, 50);
    complex_nesting(30, 20, 10);
    irregular_loops(100);
    
    return 0;
}
