/* test_hw_doloop.c - Test program for GCC hardware loop optimization pass */
/* Compile with: gcc -O2 -march=armv7-a -fdump-rtl-all -fprofile-arcs -ftest-coverage test_hw_doloop.c -o test_hw_doloop */

#include <stdint.h>
#include <stdlib.h>

/* Force ARM Thumb mode for hardware loop support */
#ifdef __arm__
__attribute__((target("thumb")))
#endif
/* Pattern A: Perfectly nested loops - inner loop bitmap is subset of outer */
void perfect_nesting(int n, int m) {
    volatile int count = 0; /* volatile to prevent optimization */
    int i, j;
    
    for (i = 0; i < n; i++) {
        /* Add extra basic block inside outer loop */
        if (__builtin_expect(1, 1)) {
            count++;
        }
        
        for (j = 0; j < m; j++) {
            /* Split inner loop into multiple basic blocks */
            if (__builtin_expect(j % 2, 0)) {
                count += 2;
            } else {
                count += 3;
            }
            
            /* Early exit to affect bitmap shape */
            if (j == m - 2) {
                break;
            }
        }
        
        /* Another block in outer loop after inner */
        count *= 2;
    }
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(count));
}

#ifdef __arm__
__attribute__((target("thumb")))
#endif
/* Pattern B: Partially overlapping loops with shared blocks but not nested */
void partial_overlap(int n, int m) {
    volatile int count = 0;
    int i = 0, j = 0;
    
    /* Shared setup block */
    int shared = n * 2;
    
    /* First loop with complex control flow */
    while (i < n) {
        if (__builtin_expect(i % 3 == 0, 0)) {
            /* This block is only in first loop */
            count += shared;
            goto skip_second; /* Create irregular CFG */
        }
        
        /* Start of potential overlap region */
        shared += i;
        
        /* Second loop that partially overlaps */
        do {
            if (__builtin_expect(j < m, 1)) {
                count += j;
                j++;
                
                /* Conditional continue affects bitmap */
                if (j % 4 == 0) continue;
                
                /* Block shared with first loop's region */
                shared -= 1;
            }
        } while (j < m && count < 1000);
        
    skip_second:
        i++;
        
        /* Reset j for next iteration */
        if (i % 2 == 0) {
            j = 0;
        }
    }
    
    /* Third loop that shares exit block with first */
    for (int k = 0; k < m; k++) {
        count -= k;
        /* Shared computation with first loop's blocks */
        if (k == shared % 10) {
            break; /* Early exit creates different bitmap */
        }
    }
    
    asm volatile("" : : "r"(count), "r"(shared));
}

#ifdef __arm__
__attribute__((target("thumb")))
#endif
/* Pattern C: Sibling loops with shared preheader block */
void sibling_loops(int n, int m) {
    volatile int count = 0;
    
    /* Shared preheader block */
    int setup = n + m;
    if (__builtin_expect(setup > 0, 1)) {
        count = setup * 2;
    }
    
    /* First sibling loop */
    int i = 0;
    while (i < n) {
        /* Unique block to first loop */
        if (i % 2 == 0) {
            count += i * 3;
        }
        
        /* Block that could be considered shared due to CFG merging */
        int temp = count;
        while (temp > 0) {
            temp--;
            if (temp % 5 == 0) break;
        }
        
        i++;
    }
    
    /* Shared intermediate block */
    count /= 2;
    
    /* Second sibling loop */
    for (int j = 0; j < m; j++) {
        /* Different structure than first loop */
        switch (j % 3) {
            case 0: count += j; break;
            case 1: count += j * 2; break;
            case 2: count += j * 3; 
                    if (j == m/2) goto loop_exit;
                    break;
        }
        
        /* Another unique block */
        for (int k = 0; k < 2; k++) {
            count -= k;
        }
    }
loop_exit:
    
    /* Shared post-loop block */
    count = abs(count);
    
    asm volatile("" : : "r"(count));
}

#ifdef __arm__
__attribute__((target("thumb")))
#endif
/* Pattern D: Complex nested loops with multiple exits and continues */
void complex_nesting(int n, int m, int p) {
    volatile int count = 0;
    
    /* Outer loop */
    for (int i = 0; i < n; i++) {
        /* Middle loop with early exit */
        int j = 0;
        while (j < m) {
            /* Innermost loop with multiple blocks */
            for (int k = 0; k < p; k++) {
                /* Split innermost into multiple blocks */
                if (k % 2 == 0) {
                    count += i + j + k;
                    if (count > 1000) {
                        goto middle_loop_exit;
                    }
                } else {
                    count -= (i * j * k) % 7;
                }
                
                /* Continue with different probability */
                if (__builtin_expect(k % 3 == 0, 0)) {
                    continue;
                }
                
                /* Another block in innermost */
                count++;
            }
            
            j++;
            
            /* Conditional continue in middle loop */
            if (j % 4 == 0) {
                continue;
            }
        }
    middle_loop_exit:
        
        /* Block after middle loop but still in outer */
        count = count % 256;
    }
    
    /* Separate loop that shares some blocks through function inlining potential */
    int q = 0;
    do {
        /* Similar structure to innermost above to create bitmap intersection */
        if (q % 2 == 0) {
            count += q;
        } else {
            count -= q;
        }
        q++;
    } while (q < n && count < 500);
    
    asm volatile("" : : "r"(count));
}

/* Force usage of all functions */
__attribute__((used))
static void use_all_patterns(void) {
    perfect_nesting(10, 5);
    partial_overlap(8, 6);
    sibling_loops(7, 9);
    complex_nesting(5, 4, 3);
}

/* Main function to execute loops */
int main(void) {
    /* Compile-time check to ensure optimization */
    _Static_assert(sizeof(int) >= 2, "int too small");
    
    /* Execute each pattern multiple times with different parameters */
    for (int run = 0; run < 3; run++) {
        perfect_nesting(50 + run, 30 + run);
        partial_overlap(40 + run, 20 + run);
        sibling_loops(25 + run, 35 + run);
        complex_nesting(15 + run, 10 + run, 8 + run);
    }
    
    return 0;
}
