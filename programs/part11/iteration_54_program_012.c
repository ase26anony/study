/* test_hw_doloop.c
 * Test program to exercise GCC's hardware loop optimization pass
 * Specifically targets uncovered lines 429-436 in hw-doloop.cc
 */

#include <stdint.h>
#include <stdlib.h>

/* Prevent optimization of empty loops */
static volatile int sink;

/* Function 1: Perfectly nested loops (Pattern A)
 * Outer loop completely contains inner loop
 * Should trigger: bitmap_intersect_p = true
 *                 bitmap_intersect_compl_p(other, loop) = false
 *                 => inner loop pushed to outer's loops vector
 */
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
        
        /* Another split block */
        if (__builtin_expect(sum > 1000, 0)) {
            sink = sum;
        }
    }
    
    sink = sum;
}

/* Function 2: Partially overlapping loops (Pattern B)
 * Two loops share some blocks but aren't perfectly nested
 * Should trigger: bitmap_intersect_p = true
 *                 both bitmap_intersect_compl_p checks = true
 *                 => neither safe_push occurs
 */
__attribute__((noinline))
void partial_overlap(int n, int m) {
    int i = 0, j = 0;
    int sum = 0;
    
    /* Shared setup block */
    int shared = n * m;
    
    /* First loop */
    do {
        sum += i;
        i++;
        
        /* Conditional entry to second loop's blocks */
        if (i % 3 == 0) {
            /* Start of second loop's logic but not a proper nested loop */
            j = 0;
            while (j < m) {
                sum += j;
                j++;
                /* Shared computation */
                if (j % 2 == 0) {
                    sum += shared;
                }
            }
        }
        
        /* More shared code */
        if (i % 4 == 0) {
            sum -= shared;
        }
    } while (i < n);
    
    sink = sum;
}

/* Function 3: Sibling loops with shared preheader (Pattern C)
 * Two sequential loops sharing setup code
 */
__attribute__((noinline))
void sibling_loops(int n, int m) {
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
        if (__builtin_expect(i == n/2, 0)) {
            sink = i;
        }
    }
    
    /* Shared intermediate block */
    setup = sum % 100;
    
    /* Second loop - uses same setup */
    j = 0;
    do {
        sum += j * setup;
        j++;
        /* Different control flow to create different bitmap */
        switch (j % 3) {
            case 0: sum += 1; break;
            case 1: sum += 2; break;
            default: sum += 3; break;
        }
    } while (j < m);
    
    sink = sum;
}

/* Function 4: Complex nested structure with multiple exits
 * Creates more complex block relationships
 */
__attribute__((noinline))
void complex_nesting(int n, int m, int k) {
    int a, b, c;
    int sum = 0;
    
    /* Three-level nesting */
    for (a = 0; a < n; a++) {
        /* Middle loop with early exit */
        b = 0;
        while (b < m) {
            sum += a + b;
            b++;
            
            /* Innermost loop */
            for (c = 0; c < k; c++) {
                sum += c;
                /* Multiple exit points */
                if (c == a) {
                    break;
                }
                if (sum > 1000) {
                    goto early_exit;
                }
            }
            
            /* Another split */
            if (b % 5 == 0) {
                sum -= a;
            }
        }
    }
    
early_exit:
    sink = sum;
}

/* Function 5: Irregular control flow with goto (Pattern B variant)
 * Creates overlapping but not nested loops
 */
__attribute__((noinline))
void irregular_loops(int n) {
    int i = 0, j = 0;
    int sum = 0;
    
loop1_start:
    if (i >= n) goto loop1_end;
    
    sum += i;
    i++;
    
    /* Jump into second loop's body */
    if (i % 2 == 0) {
        goto loop2_body;
    }
    
    goto loop1_start;
    
loop2_body:
    if (j >= n/2) goto loop2_end;
    
    sum += j * 2;
    j++;
    
    /* Jump back to first loop */
    if (j % 3 == 0) {
        goto loop1_start;
    }
    
    goto loop2_body;
    
loop1_end:
loop2_end:
    sink = sum;
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
    
    /* Prevent dead code elimination */
    _Static_assert(sizeof(int) == 4, "int must be 4 bytes");
    
    return 0;
}
