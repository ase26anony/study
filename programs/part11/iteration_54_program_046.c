/* test_hw_doloop.c - Test program for GCC hardware loop optimization pass
 * Specifically targets uncovered lines 429-436 in hw-doloop.cc
 * Compile with: gcc -O2 -march=armv7-a -fdump-rtl-all -fprofile-arcs -ftest-coverage test_hw_doloop.c -o test_hw_doloop
 */

#include <stdint.h>

/* Prevent optimization of loops */
static volatile int g_counter __attribute__((used));

/* Pattern A: Perfectly nested loops - inner loop is subset of outer */
__attribute__((target("thumb")))
void perfect_nesting(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Outer loop */
    for (i = 0; i < n; i++) {
        g_counter = i; /* Force block split */
        
        /* Inner loop - perfectly nested */
        for (j = 0; j < m; j++) {
            /* Split block inside inner loop */
            if (__builtin_expect(j % 2 == 0, 1)) {
                sum += i * j;
            } else {
                sum += i + j;
            }
        }
        
        /* Additional block in outer loop only */
        if (__builtin_expect(i % 3 == 0, 0)) {
            sum -= 1;
        }
    }
    
    g_counter = sum;
}

/* Pattern B: Partially overlapping loops with shared blocks */
__attribute__((target("arch=armv7-a")))
void partial_overlap(int n, int m) {
    int i = 0, j = 0;
    int sum = 0;
    
    /* Shared pre-header block */
    int shared = n + m;
    
    /* First loop */
    do {
        /* Block A1 - only in first loop */
        if (__builtin_expect(i < n/2, 1)) {
            sum += i * 2;
        }
        
        /* Shared block between loops */
        shared += 1;
        
        /* Conditional second loop inside first */
        if (i % 2 == 0) {
            /* Second loop - partially overlaps with first */
            j = 0;
            while (j < m) {
                /* Block in second loop only */
                sum += j * 3;
                j++;
                
                /* Shared block again */
                if (__builtin_expect(j % 4 == 0, 0)) {
                    shared += 2;
                }
            }
        }
        
        i++;
    } while (i < n);
    
    g_counter = sum + shared;
}

/* Pattern C: Sequential loops with shared pre-header */
#pragma GCC target("arch=armv7-a")
void sequential_loops(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Shared pre-header block */
    int setup = n * m;
    
    /* First loop */
    i = 0;
    while (i < n) {
        /* Block only in first loop */
        sum += i * i;
        
        /* Early exit creates different block structure */
        if (__builtin_expect(i > n/2, 0)) {
            break;
        }
        
        i++;
    }
    
    /* Shared middle block */
    setup /= 2;
    
    /* Second loop - sequential but shares setup block */
    for (j = 0; j < m; j++) {
        /* Block only in second loop */
        sum += j * j * j;
        
        /* Nested mini-loop inside second loop */
        {
            int k = 0;
            do {
                sum += k;
                k++;
            } while (k < 2);
        }
    }
    
    g_counter = sum + setup;
}

/* Pattern D: Complex overlapping with goto creating irregular CFG */
__attribute__((target("thumb")))
void irregular_cfg(int n, int m) {
    int i = 0, j = 0;
    int sum = 0;
    
    /* Loop 1 */
    for (i = 0; i < n; i++) {
        /* Block A */
        sum += i;
        
        if (i % 3 == 0) {
            /* Jump to shared block */
            goto shared_block;
        }
        
        /* Block B - only in loop 1 */
        sum += i * 2;
        
        continue;
        
    shared_block:
        /* Shared block between loops */
        sum += 777;
        
        /* Loop 2 - overlaps with loop 1 via shared_block */
        if (j < m) {
            for (j = 0; j < m; j++) {
                /* Block C - only in loop 2 */
                sum += j * 3;
                
                if (j % 2 == 0) {
                    /* Jump back to loop 1 */
                    goto back_to_loop1;
                }
            }
        }
        
        /* Block D - only in loop 1 after shared_block */
        sum += 999;
        continue;
        
    back_to_loop1:
        /* Block E - back in loop 1 */
        sum += 111;
    }
    
    g_counter = sum;
}

/* Pattern E: Multiple nested loops at different depths */
__attribute__((target("arch=armv7-a")))
void multi_depth_nesting(int n, int m, int p) {
    int i, j, k;
    int sum = 0;
    
    /* Level 1 loop */
    for (i = 0; i < n; i++) {
        /* Split block */
        if (__builtin_expect(i % 5 == 0, 0)) {
            sum += 5;
        }
        
        /* Level 2 loop */
        j = 0;
        while (j < m) {
            /* Level 3 loop - innermost */
            for (k = 0; k < p; k++) {
                sum += i * j * k;
                
                /* Conditional break affects bitmap */
                if (__builtin_expect(k > p/2, 0)) {
                    break;
                }
            }
            
            j++;
            
            /* Another level 2 loop sibling */
            if (j % 2 == 0) {
                int l;
                for (l = 0; l < 3; l++) {
                    sum += l;
                }
            }
        }
        
        /* Another level 1 loop sibling */
        {
            int x = 0;
            do {
                sum += x;
                x++;
            } while (x < 2);
        }
    }
    
    g_counter = sum;
}

/* Pattern F: Loops with function calls creating complex block relationships */
static int helper(int x) {
    /* Small loop in helper function */
    int i, sum = 0;
    for (i = 0; i < 3; i++) {
        sum += x + i;
    }
    return sum;
}

__attribute__((target("thumb")))
void loops_with_calls(int n) {
    int i;
    int sum = 0;
    
    /* Main loop */
    for (i = 0; i < n; i++) {
        /* Call helper with loop inside */
        sum += helper(i);
        
        /* Inner loop after call */
        {
            int j = 0;
            while (j < 5) {
                sum += j;
                j++;
                
                /* Conditional call creates overlapping blocks */
                if (j % 2 == 0) {
                    sum += helper(j);
                }
            }
        }
    }
    
    g_counter = sum;
}

/* Main function to ensure all patterns are executed */
int main(void) {
    /* Compile-time check to ensure optimization */
    _Static_assert(sizeof(int) == 4, "int must be 32-bit for consistent behavior");
    
    /* Execute each pattern with different parameters */
    perfect_nesting(100, 50);
    partial_overlap(100, 50);
    sequential_loops(100, 50);
    irregular_cfg(100, 50);
    multi_depth_nesting(10, 20, 30);
    loops_with_calls(100);
    
    /* Use result to prevent dead code elimination */
    return g_counter == 0 ? 0 : 1;
}
