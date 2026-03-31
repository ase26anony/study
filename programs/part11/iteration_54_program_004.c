/* test_hw_doloop.c
 * 
 * This program creates various loop patterns to trigger GCC's hw-doloop
 * optimization pass, specifically targeting the bitmap intersection
 * logic at lines 429-436 in hw-doloop.cc.
 */

#include <stdint.h>

/* Prevent optimization of empty loops */
static volatile int sink;

/* Pattern A: Perfectly nested loops - inner loop is subset of outer */
__attribute__((target("arch=armv7-a")))
void pattern_a_perfect_nesting(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Outer loop - will contain inner loop completely */
    for (i = 0; i < n; i++) {
        /* Inner loop - all blocks are within outer loop */
        for (j = 0; j < m; j++) {
            sum += i * j;
            /* Split basic block to create more complex bitmap */
            if (__builtin_expect(sum > 1000, 0)) {
                sink = sum;
            }
        }
        
        /* Additional block in outer loop but not in inner */
        if (i % 2 == 0) {
            sum += 1;
        }
    }
    
    sink = sum;
}

/* Pattern B: Partially overlapping loops with goto */
__attribute__((target("arch=armv7-a")))
void pattern_b_partial_overlap(int n, int m) {
    int i = 0, j = 0;
    int sum = 0;
    
    /* First loop */
loop1:
    if (i >= n) goto end_loop1;
    
    sum += i;
    i++;
    
    /* Shared block - both loops can reach this */
    if (sum % 3 == 0) {
        j = 0;
        /* Second loop that partially overlaps */
    loop2:
        if (j >= m) goto end_loop2;
        
        sum += j;
        j++;
        
        /* Block only in loop2 */
        if (j % 2 == 0) {
            sum += 100;
        }
        
        goto loop2;
    end_loop2:
        /* Block shared by both loops */
        sum += 1000;
    }
    
    /* Block only in loop1 */
    if (i % 5 == 0) {
        sum += 5000;
    }
    
    goto loop1;
end_loop1:
    sink = sum;
}

/* Pattern C: Sequential loops sharing a preheader */
__attribute__((target("arch=armv7-a")))
void pattern_c_sequential_loops(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Shared preheader block */
    int common = n + m;
    
    /* First loop */
    i = 0;
    do {
        sum += i * common;
        i++;
        
        /* Split block to create more bitmap entries */
        if (__builtin_expect(i == n/2, 0)) {
            sink = sum;
        }
    } while (i < n);
    
    /* Block between loops - could be considered shared */
    common = sum % 100;
    
    /* Second loop */
    for (j = 0; j < m; j++) {
        sum += j + common;
        
        /* Different block structure than first loop */
        switch (j % 3) {
            case 0: sum += 1; break;
            case 1: sum += 2; break;
            case 2: sum += 3; break;
        }
    }
    
    sink = sum;
}

/* Pattern D: Complex nested loops with early exits */
__attribute__((target("arch=armv7-a")))
void pattern_d_complex_nesting(int n, int m, int k) {
    int i, j, l;
    int sum = 0;
    
    /* Outer loop */
    for (i = 0; i < n; i++) {
        /* Middle loop with early exit */
        j = 0;
        while (j < m) {
            sum += i + j;
            j++;
            
            /* Early exit creates different block structure */
            if (sum > 10000) {
                break;
            }
            
            /* Innermost loop */
            for (l = 0; l < k; l++) {
                sum += l;
                
                /* Conditional to split blocks */
                if (l % 7 == 0) {
                    sum -= 1;
                } else if (l % 11 == 0) {
                    sum += 2;
                }
            }
        }
        
        /* Another inner loop at same level */
        for (j = 0; j < i; j++) {
            sum += j * 2;
        }
    }
    
    sink = sum;
}

/* Pattern E: Loops with irregular control flow using switch */
__attribute__((target("arch=armv7-a")))
void pattern_e_irregular_loops(int n) {
    int i = 0;
    int state = 0;
    int sum = 0;
    
    while (i < n) {
        switch (state) {
            case 0:
                sum += i;
                i++;
                if (i % 10 == 0) state = 1;
                break;
                
            case 1:
                /* Inner loop-like structure */
                for (int j = 0; j < 5; j++) {
                    sum += j;
                    /* Shared block with outer loop */
                    if (sum % 2 == 0) {
                        sum += 100;
                    }
                }
                state = 0;
                i += 2;
                break;
        }
        
        /* Block that's in outer loop but sometimes skipped */
        if (__builtin_expect(sum > 500, 0)) {
            sink = sum;
        }
    }
    
    sink = sum;
}

/* Main function to ensure all patterns are used */
int main() {
    /* Use different parameters to avoid constant folding */
    int n = 100;
    int m = 50;
    int k = 10;
    
    pattern_a_perfect_nesting(n, m);
    pattern_b_partial_overlap(n, m);
    pattern_c_sequential_loops(n, m);
    pattern_d_complex_nesting(n, m, k);
    pattern_e_irregular_loops(n);
    
    /* Ensure functions aren't optimized away */
    __attribute__((used)) static void (*funcs[])(void) = {
        (void (*)(void))pattern_a_perfect_nesting,
        (void (*)(void))pattern_b_partial_overlap,
        (void (*)(void))pattern_c_sequential_loops,
        (void (*)(void))pattern_d_complex_nesting,
        (void (*)(void))pattern_e_irregular_loops
    };
    
    return 0;
}
