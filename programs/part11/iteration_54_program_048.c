/* test_hw_doloop.c - Test program for GCC hardware loop optimization pass */
/* Compile with: gcc -O2 -march=armv7-a -fdump-rtl-all -fprofile-arcs -ftest-coverage test_hw_doloop.c -o test_hw_doloop */

#include <stdint.h>

/* Prevent optimization of empty loops */
static volatile int sink;

/* Pattern A: Perfectly nested loops - inner loop bitmap is subset of outer */
__attribute__((noinline, used))
void perfect_nesting(int n, int m) {
    int sum = 0;
    /* Outer loop */
    for (int i = 0; i < n; i++) {
        /* Split basic block to create more complex bitmap */
        if (__builtin_expect(i & 1, 0)) {
            sink = i;
        }
        
        /* Inner loop - perfectly nested */
        for (int j = 0; j < m; j++) {
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

/* Pattern B: Partially overlapping loops with shared blocks */
__attribute__((noinline, used))
void partial_overlap(int n, int m) {
    int sum = 0;
    int i = 0;
    
    /* First loop with complex control flow */
    do {
        sum += i;
        /* Shared block that both loops will include */
        if (__builtin_expect(i < n/2, 1)) {
            sink = i;
            
            /* Second loop that partially overlaps */
            int j = 0;
            while (j < m) {
                sum += j;
                j++;
                /* Conditional break affecting bitmap */
                if (j > m/2 && (i & 1)) {
                    goto skip_rest;
                }
            }
        }
skip_rest:
        i++;
    } while (i < n);
    
    sink = sum;
}

/* Pattern C: Sibling loops with shared preheader block */
__attribute__((noinline, used))
void sibling_loops(int n, int m) {
    int sum = 0;
    
    /* Shared setup block - may be included in both loop bitmaps */
    int setup = n * m;
    sink = setup;
    
    /* First sibling loop */
    for (int i = 0; i < n; i++) {
        sum += i;
        /* Split block */
        if (__builtin_expect(i == setup % 7, 0)) {
            sink = i;
        }
    }
    
    /* Shared intermediate block */
    int intermediate = sum;
    sink = intermediate;
    
    /* Second sibling loop */
    int k = 0;
    while (k < m) {
        sum += k * intermediate;
        k++;
        /* Early exit */
        if (k > m/3 && (sum & 1)) {
            break;
        }
    }
    
    sink = sum;
}

/* Pattern D: Complex nested loops with multiple exits */
__attribute__((noinline, used))
void complex_nesting(int n, int m, int p) {
    int sum = 0;
    
    /* Level 1 loop */
    for (int i = 0; i < n; i++) {
        /* Level 2 loop */
        int j = 0;
        while (j < m) {
            /* Level 3 loop - innermost */
            for (int k = 0; k < p; k++) {
                sum += i * j * k;
                
                /* Multiple exit points */
                if (k == p/2) {
                    goto next_j;
                }
                if (sum > 1000) {
                    goto finish;
                }
            }
next_j:
            j++;
            
            /* Another basic block split */
            if (__builtin_expect(j & 3, 0)) {
                sink = j;
            }
        }
        
        /* Early exit from outer loop */
        if (i > n/2 && (sum & 2)) {
            break;
        }
    }
finish:
    sink = sum;
}

/* Pattern E: Loops with irregular control flow using goto */
__attribute__((noinline, used))
void irregular_loops(int n) {
    int sum = 0;
    int i = 0;
    
loop1_start:
    if (i >= n) goto loop1_end;
    
    sum += i;
    
    /* Jump into another loop structure */
    if (i & 1) {
        int j = 0;
    loop2_start:
        if (j >= 5) goto loop2_end;
        sum += j;
        j++;
        
        /* Shared block between loops */
        sink = i + j;
        goto loop2_start;
    loop2_end:
        ;
    }
    
    i++;
    goto loop1_start;
loop1_end:
    
    sink = sum;
}

/* Pattern F: Mixed loop types with volatile to prevent removal */
__attribute__((noinline, used))
void mixed_loops(int n) {
    volatile int count = n;
    int sum = 0;
    
    /* do-while loop */
    int i = 0;
    do {
        sum += i;
        
        /* Nested for loop */
        for (int j = 0; j < count; j++) {
            sum -= j;
            
            /* while loop inside for */
            int k = 0;
            while (k < 3) {
                sum += k;
                k++;
                if (k == 2) {
                    goto skip_inner;
                }
            }
        skip_inner:
            ;
        }
        
        i++;
    } while (i < count);
    
    sink = sum;
}

/* Main function to ensure all patterns are executed */
int main(void) {
    /* Small iteration counts to keep runtime reasonable */
    const int small_n = 10;
    const int small_m = 5;
    const int small_p = 3;
    
    /* Execute each pattern multiple times with different parameters */
    for (int run = 0; run < 3; run++) {
        perfect_nesting(small_n + run, small_m + run);
        partial_overlap(small_n + run, small_m + run);
        sibling_loops(small_n + run, small_m + run);
        complex_nesting(small_n, small_m, small_p);
        irregular_loops(small_n + run);
        mixed_loops(small_m + run);
    }
    
    /* Compile-time check to ensure loops aren't optimized away */
    static_assert(sizeof(int) == 4, "Unexpected int size");
    
    return 0;
}
