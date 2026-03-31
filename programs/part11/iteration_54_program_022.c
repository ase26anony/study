/* test_hw_doloop.c - Test program for GCC hardware loop optimization pass */
/* Compile with: gcc -O2 -march=armv7-a -fdump-rtl-all -fprofile-arcs -ftest-coverage test_hw_doloop.c -o test_hw_doloop */

#include <stdint.h>
#include <stdlib.h>

/* Prevent optimization of empty loops */
static volatile int sink;

/* Function 1: Perfectly nested loops (Pattern A) */
__attribute__((target("thumb")))
void perfect_nesting(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Outer loop */
    for (i = 0; i < n; i++) {
        /* Split basic block to create more complex bitmap */
        if (__builtin_expect(i < n, 1)) {
            /* Inner loop - perfectly nested */
            for (j = 0; j < m; j++) {
                sum += i * j;
                /* Another split to increase block count */
                if (__builtin_expect(j < m, 1)) {
                    sink = sum;
                }
            }
        }
    }
    
    /* Prevent dead code elimination */
    sink = sum;
}

/* Function 2: Partially overlapping loops (Pattern B) */
__attribute__((used))
void partial_overlap(int n, int m) {
    int i = 0, j = 0;
    int sum = 0;
    
    /* First loop with early exit */
    while (i < n) {
        sum += i;
        i++;
        
        /* Conditional inner loop - not always executed */
        if (i % 2 == 0) {
            /* This creates partial overlap */
            for (j = 0; j < m; j++) {
                sum += j;
                /* Early exit from inner loop */
                if (j > m/2) break;
            }
        }
        
        /* Split block after conditional */
        if (__builtin_expect(i < n, 1)) {
            sink = sum;
        }
    }
    
    /* Second loop that shares some blocks */
    j = 0;
    do {
        sum -= j;
        j++;
        /* Shared computation with first loop */
        if (j < m/2) {
            sink = sum;
        }
    } while (j < m);
    
    sink = sum;
}

/* Function 3: Sibling loops with shared preheader (Pattern C) */
__attribute__((target("arch=armv7-a")))
void sibling_loops(int n, int m) {
    int i, j;
    int sum = 0;
    int shared_var = 0;
    
    /* Shared preheader block */
    shared_var = n + m;
    sink = shared_var;
    
    /* First sibling loop */
    for (i = 0; i < n; i++) {
        sum += i * shared_var;
        /* Split block */
        if (__builtin_expect(i < n, 1)) {
            sink = sum;
        }
    }
    
    /* Shared intermediate block */
    shared_var = sum % 256;
    
    /* Second sibling loop */
    for (j = 0; j < m; j++) {
        sum -= j * shared_var;
        /* Different split pattern */
        if (__builtin_expect(j > 0, 1)) {
            sink = sum;
        }
    }
    
    sink = sum;
}

/* Function 4: Complex nested structure with multiple exits */
__attribute__((used))
void complex_nesting(int n, int m, int k) {
    int i, j, l;
    int sum = 0;
    
    /* Outer loop */
    for (i = 0; i < n; i++) {
        /* Middle loop with conditional continue */
        j = 0;
        while (j < m) {
            if (j % 3 == 0) {
                j++;
                continue;  /* Affects block structure */
            }
            
            /* Innermost loop */
            for (l = 0; l < k; l++) {
                sum += i * j * l;
                /* Multiple exit points */
                if (l > k/2) {
                    break;
                }
                if (sum > 1000) {
                    goto early_exit;
                }
            }
            j++;
        }
        
        /* Another inner loop at same level */
        for (l = 0; l < i; l++) {
            sum -= l;
        }
    }
    
early_exit:
    sink = sum;
}

/* Function 5: Irregular control flow with goto */
void irregular_loops(int n) {
    int i = 0, j = 0;
    int sum = 0;
    
start_outer:
    if (i >= n) goto end;
    
    sum += i;
    i++;
    
    /* Conditional jump creates overlapping blocks */
    if (i % 2 == 0) {
        j = 0;
    start_inner:
        if (j >= 5) goto end_inner;
        sum += j;
        j++;
        goto start_inner;
    end_inner:
        sink = sum;
    }
    
    goto start_outer;
    
end:
    sink = sum;
}

/* Main function to ensure all functions are called */
int main(void) {
    int test_n = 100;
    int test_m = 50;
    int test_k = 20;
    
    /* Call each function multiple times with different parameters
       to ensure various paths are taken */
    perfect_nesting(test_n, test_m);
    perfect_nesting(test_n/2, test_m*2);
    
    partial_overlap(test_n, test_m);
    partial_overlap(test_n*2, test_m/2);
    
    sibling_loops(test_n, test_m);
    sibling_loops(test_m, test_n);
    
    complex_nesting(test_n, test_m, test_k);
    complex_nesting(test_k, test_n, test_m);
    
    irregular_loops(test_n);
    irregular_loops(test_m);
    
    /* Compile-time check to ensure optimization */
    _Static_assert(sizeof(int) == 4, "int must be 32-bit for consistent behavior");
    
    return 0;
}
