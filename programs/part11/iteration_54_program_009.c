/* test_hw_doloop.c
 * Test program to exercise GCC's hardware loop optimization pass
 * Specifically targets lines 429-436 in hw-doloop.cc
 */

#include <stdint.h>

/* Prevent optimization of empty loops */
static volatile int sink;

/* Function 1: Perfectly nested loops (Pattern A)
 * Outer loop completely contains inner loop
 * Should trigger: bitmap_intersect_p = true
 *                 bitmap_intersect_compl_p(other->block_bitmap, loop->block_bitmap) = false
 * Result: inner loop pushed into outer's loops vector
 */
__attribute__((target("thumb")))
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

/* Function 2: Partially overlapping loops (Pattern B)
 * Two loops share some blocks but not perfectly nested
 * Should trigger: bitmap_intersect_p = true
 *                 both bitmap_intersect_compl_p checks = true
 * Result: Neither safe_push occurs, continue after first if
 */
__attribute__((target("arch=armv7-a")))
void partial_overlap(int n, int m) {
    int sum = 0;
    int i = 0;
    
    /* First loop with complex control flow */
    do {
        /* Shared block between loops */
        if (__builtin_expect(sum < 100, 1)) {
            sink = sum;
        }
        
        sum += i;
        i++;
        
        /* Conditional second loop inside first */
        if (i % 3 == 0) {
            int j = 0;
            /* Second loop - partially overlapping */
            while (j < m) {
                sum -= j;
                j++;
                /* Break to create irregular control flow */
                if (j > m/2) {
                    goto skip_rest;
                }
            }
skip_rest:
            sink = j;
        }
        
        /* Another block in first loop only */
        if (i & 1) {
            sum *= 2;
        }
    } while (i < n);
    
    sink = sum;
}

/* Function 3: Sibling loops with shared preheader (Pattern C)
 * Two sequential loops sharing setup code
 * May trigger various bitmap intersection conditions
 */
#pragma GCC target("arch=armv7-a")
void sibling_loops(int n, int m) {
    int sum1 = 0, sum2 = 0;
    
    /* Shared preheader block */
    int setup = n + m;
    sink = setup;
    
    /* First loop */
    for (int i = 0; i < n; i++) {
        sum1 += i;
        /* Split block */
        if (__builtin_expect(i == setup, 0)) {
            break;
        }
    }
    
    /* Shared block between loops? */
    int intermediate = sum1;
    sink = intermediate;
    
    /* Second loop - different structure */
    int j = 0;
    while (j < m) {
        sum2 += j * 2;
        j++;
        /* Nested conditional loop */
        if (j % 5 == 0) {
            for (int k = 0; k < 3; k++) {
                sum2 -= k;
            }
        }
    }
    
    sink = sum1 + sum2;
}

/* Function 4: Complex nested loops with multiple exits
 * Creates deep hierarchy for loop tree construction
 */
__attribute__((target("thumb")))
void complex_hierarchy(int n, int m, int p) {
    int total = 0;
    
    /* Level 1 loop */
    for (int i = 0; i < n; i++) {
        /* Level 2 loop */
        int j = 0;
        while (j < m) {
            /* Level 3 loop - innermost */
            for (int k = 0; k < p; k++) {
                total += i * j * k;
                
                /* Multiple exit points */
                if (k == i) {
                    break;
                }
                if (total > 1000) {
                    goto level2_continue;
                }
            }
            
            j++;
            if (j == i) {
                break;
            }
        }
level2_continue:
        /* Another loop at same level as while */
        for (int x = 0; x < 2; x++) {
            total -= x;
        }
    }
    
    sink = total;
}

/* Function 5: Disjoint loops with no intersection
 * Should not trigger the uncovered code
 * But helps build complete loop tree
 */
void disjoint_loops(int n, int m) {
    int a = 0, b = 0;
    
    /* First independent loop */
    for (int i = 0; i < n; i++) {
        a += i;
    }
    
    /* Unrelated code between loops */
    int temp = a * 2;
    sink = temp;
    
    /* Second independent loop */
    int j = m;
    do {
        b += j;
        j--;
    } while (j > 0);
    
    sink = a + b;
}

/* Function 6: Loop with switch inside creating multiple blocks */
__attribute__((used))
void switch_in_loop(int n) {
    int result = 0;
    
    for (int i = 0; i < n; i++) {
        switch (i % 4) {
            case 0:
                result += i;
                break;
            case 1:
                result -= i;
                /* Fall through */
            case 2:
                result *= 2;
                break;
            default:
                result /= (i + 1);
                break;
        }
        
        /* Small nested loop in some cases */
        if (i % 3 == 0) {
            for (int j = 0; j < 2; j++) {
                result += j;
            }
        }
    }
    
    sink = result;
}

/* Main function to call all test patterns */
int main(void) {
    /* Call functions with different parameters
     * to create varied loop structures */
    perfect_nesting(100, 50);
    partial_overlap(75, 25);
    sibling_loops(60, 40);
    complex_hierarchy(10, 20, 30);
    disjoint_loops(50, 50);
    switch_in_loop(40);
    
    /* Compile-time check to ensure compilation proceeds */
    _Static_assert(sizeof(int) == 4, "int must be 4 bytes");
    
    return 0;
}
