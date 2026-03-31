/* test_hw_doloop.c - Test program for GCC hardware loop optimization pass */
/* Compile with: gcc -O2 -march=armv7-a -fdump-rtl-all -fprofile-arcs -ftest-coverage test_hw_doloop.c -o test_hw_doloop */

#include <stdint.h>

/* Prevent optimization of empty loops */
static volatile int sink;

/* Function 1: Perfectly nested loops (Pattern A) */
/* Should trigger: bitmap_intersect_p = true, bitmap_intersect_compl_p(inner, outer) = false */
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
            /* Another basic block split */
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

/* Function 2: Partially overlapping loops (Pattern B) */
/* Should trigger: bitmap_intersect_p = true, both bitmap_intersect_compl_p checks = true */
__attribute__((used))
void partial_overlap(int n, int m) {
    int i = 0, j = 0;
    int sum = 0;
    
    /* First loop */
    while (i < n) {
        sum += i;
        i++;
        
        /* Shared block that both loops might include */
        if (__builtin_expect(sum & 1, 0)) {
            sink = sum;
        }
        
        /* Conditional second loop inside first */
        if (i % 3 == 0) {
            /* Second loop - partially overlapping */
            for (j = 0; j < m; j++) {
                sum += j;
                /* Shared computation */
                if (__builtin_expect(j & 1, 1)) {
                    sink = j * 2;
                }
                
                /* Early exit creates different block structure */
                if (j > m/2) {
                    break;
                }
            }
        }
    }
    
    /* Additional disjoint loop to create sibling relationship */
    for (int k = 0; k < n; k++) {
        sum -= k;
    }
    
    sink = sum;
}

/* Function 3: Sibling loops with shared preheader (Pattern C) */
/* Should trigger bitmap_intersect_p = true and one bitmap_intersect_compl_p = false */
#pragma GCC target("arch=armv7-a")
void sibling_loops(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Shared setup block - may be included in both loop bitmaps */
    int shared = n * 2;
    sink = shared;
    
    /* First sibling loop */
    i = 0;
    do {
        sum += i + shared;
        i++;
        
        /* Split block */
        if (__builtin_expect(i & 3, 0)) {
            sink = i;
        }
    } while (i < n);
    
    /* Shared intermediate block */
    shared = sum % 256;
    
    /* Second sibling loop - shares some blocks with first */
    for (j = 0; j < m; j++) {
        sum -= j + shared;
        
        /* Different block structure */
        switch (j % 4) {
            case 0: sink = j; break;
            case 1: sink = j * 2; break;
            case 2: sink = j * 3; break;
            default: sink = j * 4; break;
        }
        
        if (j > m/3) {
            /* Nested mini-loop */
            for (int k = 0; k < 2; k++) {
                sum += k;
            }
        }
    }
    
    sink = sum;
}

/* Function 4: Complex irregular nesting with goto */
/* Creates unusual control flow for bitmap analysis */
__attribute__((noinline))
void irregular_nesting(int n) {
    int i = 0, j = 0;
    int sum = 0;
    
    /* Loop with goto creating overlap */
start_loop1:
    if (i >= n) goto end_loop1;
    
    sum += i;
    i++;
    
    /* Conditional jump to second loop */
    if (i % 2 == 0) {
        j = 0;
        /* Second loop partially overlapping */
    start_loop2:
        if (j >= 5) goto end_loop2;
        
        sum += j;
        j++;
        
        /* Jump back to first loop */
        if (j == 3) {
            goto start_loop1;
        }
        
        goto start_loop2;
    end_loop2:
        ;
    }
    
    /* Split block */
    if (__builtin_expect(sum > 100, 0)) {
        sink = sum;
    }
    
    goto start_loop1;
end_loop1:
    
    sink = sum;
}

/* Function 5: Multiple depth nesting */
void multi_depth_nesting(int a, int b, int c) {
    int sum = 0;
    
    /* Level 1 */
    for (int i = 0; i < a; i++) {
        /* Level 2 */
        for (int j = 0; j < b; j++) {
            /* Level 3 */
            for (int k = 0; k < c; k++) {
                sum += i * j * k;
                
                /* Innermost conditional */
                if (__builtin_expect(k & 1, 1)) {
                    sink = k;
                }
            }
            
            /* Early exit at level 2 */
            if (j > b/2) {
                break;
            }
        }
        
        /* Additional block at level 1 */
        sum += i * 100;
    }
    
    /* Final sibling loop */
    for (int x = 0; x < 10; x++) {
        sum -= x;
    }
    
    sink = sum;
}

/* Main function to ensure all functions are called */
int main(void) {
    /* Call each function with different parameters to create
       different loop execution patterns */
    perfect_nesting(100, 50);
    partial_overlap(200, 100);
    sibling_loops(150, 75);
    irregular_nesting(50);
    multi_depth_nesting(20, 15, 10);
    
    /* Compile-time check to ensure optimization */
    _Static_assert(sizeof(int) == 4, "int must be 4 bytes");
    
    return 0;
}
