/* test_hw_doloop.c - Test program for GCC hardware loop optimization pass */
/* Compile with: gcc -O2 -march=armv7-a -fdump-rtl-all -fprofile-arcs -ftest-coverage test_hw_doloop.c -o test_hw_doloop */

#include <stdint.h>
#include <stdlib.h>

/* Prevent optimization of empty loops */
static volatile int sink;

/* Pattern A: Perfectly nested loops - inner loop bitmap is subset of outer */
__attribute__((target("thumb")))
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
            /* Another block splitter */
            if (__builtin_expect(j & 1, 1)) {
                sink = j;
            }
        }
        
        /* Early exit to affect block structure */
        if (i > n/2) {
            break;
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
    int shared = n * m;
    
    /* First loop */
    do {
        sum += i;
        i++;
        
        /* Conditional that may execute second loop */
        if (i % 3 == 0 && j < m) {
            /* Second loop that overlaps but isn't nested */
            while (j < m) {
                sum -= j;
                j++;
                /* Shared computation block */
                if (j % 2 == 0) {
                    sum += shared;
                }
                /* Early exit creates different block structure */
                if (j > m/2) goto finish_second;
            }
            finish_second:
            continue;
        }
        
        /* Block only in first loop */
        if (__builtin_expect(i & 4, 0)) {
            sink = i;
        }
    } while (i < n);
    
    sink = sum;
}

/* Pattern C: Sequential loops with shared preheader */
#pragma GCC target("arch=armv7-a")
void sequential_loops(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Shared preheader block */
    int setup = n + m;
    sink = setup;
    
    /* First loop */
    i = 0;
    while (i < n) {
        sum += i * setup;
        i++;
        /* Split block */
        if (__builtin_expect(i & 2, 1)) {
            sink = i;
        }
    }
    
    /* Shared intermediate block */
    setup = sum % 256;
    
    /* Second loop - shares setup block in bitmap */
    for (j = 0; j < m; j++) {
        sum -= j * setup;
        /* Different block structure */
        switch (j % 3) {
            case 0: sink = j; break;
            case 1: sum += 1; break;
            default: sum -= 1; break;
        }
    }
    
    sink = sum;
}

/* Pattern D: Complex nested structure with multiple levels */
__attribute__((used, target("thumb")))
void multi_level_nesting(int n, int m, int k) {
    int a, b, c;
    int total = 0;
    
    /* Level 1 */
    for (a = 0; a < n; a++) {
        /* Always-true if to split block */
        if (__builtin_expect(1, 1)) {
            sink = a;
        }
        
        /* Level 2 */
        b = 0;
        while (b < m) {
            total += a * b;
            
            /* Level 3 - innermost */
            for (c = 0; c < k; c++) {
                total -= c;
                /* Conditional exit affects bitmap */
                if (c > k/3) {
                    total += 100;
                    if (c > k/2) break;
                }
            }
            
            b++;
            /* Another block splitter */
            if (b % 4 == 0) {
                sink = b;
            }
        }
        
        /* Early exit at middle level */
        if (a > n/4) {
            total += 1000;
            if (a > n/2) break;
        }
    }
    
    sink = total;
}

/* Pattern E: Loops with irregular control flow using goto */
__attribute__((used))
void irregular_loops(int n) {
    int i = 0, j = 0;
    int val = 0;
    
start_first:
    if (i >= n) goto end_all;
    
    val += i;
    i++;
    
    /* Jump into second loop */
    if (i % 2 == 0) goto middle_second;
    
    /* Second loop with shared entry point */
    j = 0;
start_second:
    if (j >= 5) goto end_second;
    
    val -= j;
    j++;
    
    /* Shared block between loops */
    if (__builtin_expect(val & 1, 0)) {
        sink = val;
    }
    
    goto start_second;

middle_second:
    val *= 2;
    goto start_second;

end_second:
    /* Block only in first loop */
    if (i % 3 == 0) {
        val += 100;
    }
    
    goto start_first;

end_all:
    sink = val;
}

/* Main function to ensure all patterns are executed */
int main(void) {
    int test_n = 100;
    int test_m = 50;
    int test_k = 20;
    
    /* Execute all patterns */
    perfect_nesting(test_n, test_m);
    partial_overlap(test_n, test_m);
    sequential_loops(test_n, test_m);
    multi_level_nesting(test_n, test_m, test_k);
    irregular_loops(test_n);
    
    /* Compile-time check to ensure optimization */
    _Static_assert(sizeof(int) == 4, "int must be 32-bit for consistent behavior");
    
    return 0;
}
