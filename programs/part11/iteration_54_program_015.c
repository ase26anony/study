/* test_hw_doloop.c - Test program for GCC hardware loop optimization pass */
/* Compile with: gcc -O2 -march=armv7-a -fdump-rtl-all -fprofile-arcs -ftest-coverage test_hw_doloop.c -o test_hw_doloop */

#include <stdint.h>
#include <stdlib.h>

/* Prevent optimization of empty loops */
static volatile int sink;

/* Function 1: Perfectly nested loops (Pattern A)
   Should trigger: bitmap_intersect_p true, bitmap_intersect_compl_p false for inner vs outer */
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
            /* Early exit to affect block structure */
            if (j == m/2 && (i & 1)) {
                break;
            }
        }
        
        /* Another split in outer loop */
        if (__builtin_expect(sum > 1000, 0)) {
            sink = sum;
        }
    }
    
    sink = sum;
}

/* Function 2: Partially overlapping loops (Pattern B)
   Should trigger: bitmap_intersect_p true, both bitmap_intersect_compl_p true */
__attribute__((noinline))
void partial_overlap(int n, int m) {
    int i = 0, j = 0;
    int sum = 0;
    
    /* First loop with complex control flow */
    while (i < n) {
        sum += i;
        
        /* Conditional inner loop - not always executed */
        if (i > n/2) {
            /* This creates partial overlap - some blocks shared, some not */
            for (j = 0; j < m; j++) {
                sum -= j;
                if (j == m/3) {
                    /* Early continue affects bitmap */
                    continue;
                }
                sum += 1;
            }
        }
        
        /* Another condition that splits the block */
        if (__builtin_expect(i % 3 == 0, 1)) {
            sink = i;
        }
        
        i++;
    }
    
    /* Second loop that shares some setup code */
    j = 0;
    do {
        sum += j * 2;
        /* Use same sink variable creating implicit overlap */
        if (j % 4 == 0) {
            sink = j;
        }
        j++;
    } while (j < m);
    
    sink = sum;
}

/* Function 3: Sibling loops with shared preheader (Pattern C)
   Should trigger bitmap_intersect_p true with specific complement pattern */
__attribute__((noinline))
void sibling_loops(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Shared setup block - may be included in both loop bitmaps */
    int shared = n + m;
    sink = shared;
    
    /* First sibling loop */
    for (i = 0; i < n; i++) {
        sum += i * shared;
        /* Complex if to split blocks */
        if (__builtin_expect(i == n/2, 0)) {
            shared--;
            sink = shared;
        }
    }
    
    /* Shared intermediate computation */
    shared = sum % 100;
    
    /* Second sibling loop - shares some blocks through common variables */
    for (j = 0; j < m; j++) {
        sum -= j * shared;
        /* Different condition pattern */
        if (j % 5 == 0) {
            sink = j;
            shared += 2;
        }
    }
    
    sink = sum + shared;
}

/* Function 4: Complex nested loops with multiple exits
   Creates various intersection patterns */
__attribute__((noinline))
void complex_nesting(int n, int m, int k) {
    int i, j, l;
    int sum = 0;
    
    /* Triple nesting */
    for (i = 0; i < n; i++) {
        /* Middle loop with early exit */
        j = 0;
        while (j < m) {
            sum += i + j;
            
            /* Innermost loop */
            for (l = 0; l < k; l++) {
                sum += l;
                /* Multiple exit points */
                if (l == i) break;
                if (sum > 1000) goto outer_break;
            }
            
            j++;
            if (j == i) break;
        }
        
        /* Label for goto */
        outer_break:
        if (i == n/2) break;
    }
    
    /* Another disjoint loop that might share blocks through sink */
    for (i = 0; i < 10; i++) {
        sink = sum + i;
    }
}

/* Function 5: Irregular control flow with goto creating overlapping regions */
__attribute__((noinline))
void irregular_loops(int n) {
    int i = 0, j = 0;
    int sum = 0;
    
    /* Loop with goto creating overlapping block structure */
    for (i = 0; i < n; i++) {
        if (i % 2 == 0) {
            sum += i;
            goto shared_block;
        } else {
            sum -= i;
        }
        
        shared_block:
        sink = sum;
        
        /* Nested loop that shares the shared_block */
        for (j = 0; j < 5; j++) {
            if (j == 2) goto shared_block;
            sum += j;
        }
    }
}

/* Main function to ensure all functions are called */
int main(void) {
    int test_n = 100;
    int test_m = 50;
    int test_k = 20;
    
    /* Call all functions to ensure they're not optimized away */
    perfect_nesting(test_n, test_m);
    partial_overlap(test_n, test_m);
    sibling_loops(test_n, test_m);
    complex_nesting(test_n, test_m, test_k);
    irregular_loops(test_n);
    
    /* Use results to prevent dead code elimination */
    if (sink > 1000000) {
        return 1;
    }
    
    return 0;
}
