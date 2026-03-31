/* test_hw_doloop.c - Test program for GCC hardware loop optimization pass */
/* Compile with: gcc -O2 -march=armv7-a -fdump-rtl-all -fprofile-arcs -ftest-coverage test_hw_doloop.c -o test_hw_doloop */

#include <stdint.h>

/* Prevent optimization of empty loops */
static volatile int sink;

/* Pattern A: Perfectly nested loops - inner loop bitmap is subset of outer */
__attribute__((noinline))
void perfect_nesting(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Outer loop */
    for (i = 0; i < n; i++) {
        /* Split basic block to create more complex bitmap */
        if (__builtin_expect(i & 1, 0)) {
            sink = i;
        }
        
        /* Inner loop - perfectly nested */
        for (j = 0; j < m; j++) {
            sum += i * j;
            /* Another split to affect bitmap */
            if (__builtin_expect(j & 1, 1)) {
                sink = j;
            }
        }
        
        /* Additional block in outer loop only */
        if (__builtin_expect(i == n - 1, 0)) {
            sink = sum;
        }
    }
    
    sink = sum;
}

/* Pattern B: Partially overlapping loops with shared blocks but not nested */
__attribute__((noinline))
void partial_overlap(int n, int m) {
    int i = 0, j = 0;
    int sum = 0;
    
    /* Shared setup block */
    if (n > 0 && m > 0) {
        sink = 1;
    }
    
    /* First loop */
    do {
        sum += i;
        i++;
        
        /* Conditional that sometimes executes second loop */
        if (i == n / 2) {
            /* Second loop that shares some blocks but not perfectly nested */
            j = 0;
            while (j < m) {
                sum -= j;
                j++;
                /* Early exit affects bitmap shape */
                if (j == m / 2) break;
            }
        }
    } while (i < n);
    
    /* Continue with first loop's blocks */
    if (i == n) {
        sink = sum;
    }
}

/* Pattern C: Sequential loops sharing a common preheader block */
__attribute__((noinline))
void sequential_loops(int n, int m) {
    int i, j;
    int sum1 = 0, sum2 = 0;
    
    /* Shared preheader block */
    int shared = n + m;
    sink = shared;
    
    /* First loop */
    for (i = 0; i < n; i++) {
        sum1 += i * shared;
        /* Split block */
        if (__builtin_expect(i & 2, 0)) {
            sink = i;
        }
    }
    
    /* Block between loops (not shared) */
    sink = sum1;
    
    /* Second loop - sequential, shares preheader but not body */
    for (j = 0; j < m; j++) {
        sum2 += j * shared;
        /* Different split pattern */
        if (__builtin_expect(j & 4, 1)) {
            sink = j;
        }
    }
    
    sink = sum1 + sum2;
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
            sum += i + j;
            j++;
            
            /* Early exit creates different block bitmap */
            if (j > m / 2 && (i & 1)) {
                break;
            }
            
            /* Innermost loop - triple nesting */
            for (l = 0; l < k; l++) {
                sum -= l;
                /* Conditional split */
                if (__builtin_expect(l == k - 1, 0)) {
                    sink = l;
                }
            }
        }
        
        /* Another inner loop at same level */
        if (i > n / 2) {
            for (l = 0; l < 3; l++) {
                sum += l * i;
            }
        }
    }
    
    sink = sum;
}

/* Pattern E: Loops with goto creating irregular control flow */
__attribute__((noinline))
void irregular_loops(int n) {
    int i = 0, j = 0;
    int sum = 0;
    
    /* Loop with goto creating overlap */
    for (i = 0; i < n; i++) {
        if (i & 1) {
            sum += i;
            goto shared_block;
        } else {
            sum -= i;
        }
        
        /* Block executed by both paths */
        shared_block:
        sink = i;
        
        /* Nested loop entered via goto */
        if (i == n / 3) {
            j = 0;
            another_loop:
            sum += j;
            j++;
            if (j < 5) goto another_loop;
        }
    }
    
    sink = sum;
}

/* Pattern F: Multiple functions with different loop styles */
__attribute__((noinline, target("thumb")))
void thumb_nested_loops(int n) {
    int i, j;
    int sum = 0;
    
    /* Simple double nested loop for Thumb target */
    for (i = 0; i < n; i++) {
        for (j = 0; j < 10; j++) {
            sum += i * j;
        }
    }
    
    sink = sum;
}

/* Main function to ensure all patterns are executed */
int main(void) {
    int test_n = 100;
    int test_m = 50;
    int test_k = 10;
    
    /* Execute all patterns to ensure compiler sees them */
    perfect_nesting(test_n, test_m);
    partial_overlap(test_n, test_m);
    sequential_loops(test_n, test_m);
    complex_nesting(test_n, test_m, test_k);
    irregular_loops(test_n);
    thumb_nested_loops(test_n);
    
    /* Prevent dead code elimination */
    __attribute__((used)) static int force_usage = 0;
    force_usage = sink;
    
    return 0;
}
