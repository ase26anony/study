/* test_hw_doloop.c
 * 
 * This program is designed to trigger specific conditions in GCC's
 * hardware loop optimization pass (hw-doloop.cc lines 429-436).
 * It creates various loop nesting patterns to exercise bitmap
 * intersection and complement checks.
 */

#include <stdint.h>

/* Prevent optimization of empty loops */
volatile int counter = 0;
#define KEEP_ALIVE(x) do { counter += (x); } while(0)

/* Pattern A: Perfectly nested loops - inner loop is subset of outer */
__attribute__((noinline))
void pattern_a_perfect_nesting(int n, int m) {
    int i, j;
    for (i = 0; i < n; i++) {
        /* Add some basic blocks inside the outer loop */
        if (__builtin_expect(i & 1, 0)) {
            KEEP_ALIVE(1);
        }
        
        for (j = 0; j < m; j++) {
            /* Inner loop body with multiple basic blocks */
            if (__builtin_expect(j & 1, 0)) {
                KEEP_ALIVE(2);
            } else {
                KEEP_ALIVE(3);
            }
        }
        
        /* Another block in outer loop after inner loop */
        KEEP_ALIVE(4);
    }
}

/* Pattern B: Partially overlapping loops - share some blocks but not nested */
__attribute__((noinline))
void pattern_b_partial_overlap(int n, int m) {
    int i = 0, j = 0;
    
    /* Shared setup block */
    int shared = n + m;
    KEEP_ALIVE(shared);
    
    /* First loop */
    while (i < n) {
        /* Block A1 */
        if (__builtin_expect(i & 1, 0)) {
            KEEP_ALIVE(5);
        }
        
        /* Shared block between loops */
        if (shared > 0) {
            KEEP_ALIVE(6);
        }
        
        i++;
        
        /* Conditional second loop inside first */
        if (i == n/2) {
            /* Second loop that partially overlaps */
            for (j = 0; j < m; j++) {
                /* Block B1 - not in first loop */
                if (__builtin_expect(j & 2, 0)) {
                    KEEP_ALIVE(7);
                }
                
                /* Shared block */
                if (shared > 0) {
                    KEEP_ALIVE(8);
                }
                
                /* Early exit creates more blocks */
                if (j > m/2) break;
            }
        }
    }
    
    /* Continuation after loops */
    KEEP_ALIVE(9);
}

/* Pattern C: Sibling loops with shared preheader */
__attribute__((noinline)) 
void pattern_c_sibling_loops(int n, int m) {
    int i, j;
    
    /* Shared preheader block */
    int setup = n * 2;
    KEEP_ALIVE(setup);
    
    /* First sibling loop */
    i = 0;
    do {
        if (__builtin_expect(i & 3, 0)) {
            KEEP_ALIVE(10);
        }
        i++;
    } while (i < n);
    
    /* Shared middle block */
    KEEP_ALIVE(setup);
    
    /* Second sibling loop */
    for (j = 0; j < m; j++) {
        if (__builtin_expect(j & 3, 0)) {
            KEEP_ALIVE(11);
        } else {
            KEEP_ALIVE(12);
        }
        
        /* Early exit creates different block structure */
        if (j > m/3) {
            KEEP_ALIVE(13);
            if (j > m/2) break;
        }
    }
    
    /* Shared post-loop block */
    KEEP_ALIVE(setup);
}

/* Pattern D: Complex nested structure with multiple levels */
__attribute__((noinline))
void pattern_d_complex_nesting(int n, int m, int k) {
    int a, b, c;
    
    /* Level 1 loop */
    for (a = 0; a < n; a++) {
        KEEP_ALIVE(14);
        
        /* Level 2 loop - perfectly nested */
        for (b = 0; b < m; b++) {
            /* Split block in middle loop */
            if (__builtin_expect(b & 1, 0)) {
                KEEP_ALIVE(15);
            }
            
            /* Level 3 loop - innermost */
            c = 0;
            while (c < k) {
                /* Multiple blocks in innermost loop */
                if (__builtin_expect(c & 1, 0)) {
                    KEEP_ALIVE(16);
                    if (__builtin_expect(c & 2, 0)) {
                        KEEP_ALIVE(17);
                    }
                }
                c++;
            }
            
            /* Another block in middle loop */
            KEEP_ALIVE(18);
        }
        
        /* Conditional inner loop that's not always executed */
        if (__builtin_expect(a & 1, 0)) {
            for (b = m; b > 0; b--) {
                KEEP_ALIVE(19);
                /* Early exit */
                if (b < m/2) break;
            }
        }
    }
}

/* Pattern E: Loops with goto creating irregular control flow */
__attribute__((noinline))
void pattern_e_goto_loops(int n, int m) {
    int i = 0, j = 0;
    
start_outer:
    if (i >= n) goto end;
    
    /* Block in outer loop */
    KEEP_ALIVE(20);
    
    /* Conditional jump to inner loop */
    if (i % 2 == 0) {
        j = 0;
start_inner:
        if (j >= m) goto after_inner;
        
        KEEP_ALIVE(21);
        j++;
        
        /* Shared block between goto paths */
        if (j < m/2) {
            KEEP_ALIVE(22);
            goto start_inner;
        } else {
            KEEP_ALIVE(23);
            goto start_inner;
        }
    }
    
after_inner:
    i++;
    goto start_outer;
    
end:
    KEEP_ALIVE(24);
}

/* Main function to ensure all patterns are used */
int main() {
    /* Call each pattern with different parameters
     * to create varied loop structures */
    pattern_a_perfect_nesting(100, 50);
    pattern_b_partial_overlap(100, 50);
    pattern_c_sibling_loops(100, 50);
    pattern_d_complex_nesting(10, 20, 30);
    pattern_e_goto_loops(100, 50);
    
    /* Use the counter to prevent dead code elimination */
    return counter > 0 ? 0 : 1;
}
