/* test_hw_doloop.c
 * Test program to cover uncovered lines in GCC's hw-doloop pass
 * Lines 429-436 in hw-doloop.cc
 */

#include <stdint.h>

/* Prevent optimization of empty loops */
static volatile int sink;

/* Function 1: Perfectly nested loops (Pattern A)
 * Outer loop contains inner loop completely
 * Should trigger: bitmap_intersect_p = true
 *                 bitmap_intersect_compl_p(other, loop) = false
 *                 => loop->loops.safe_push(other)
 */
__attribute__((target("arch=armv7-a")))
void perfect_nesting(int n, int m) {
    int sum = 0;
    /* Outer loop */
    for (int i = 0; i < n; i++) {
        /* Split basic block */
        if (__builtin_expect(i < n, 1)) {
            /* Inner loop - completely contained */
            for (int j = 0; j < m; j++) {
                sum += i * j;
                /* Another split to create more blocks */
                if (__builtin_expect(j % 2 == 0, 1)) {
                    sum += 1;
                }
            }
        }
    }
    sink = sum;
}

/* Function 2: Partially overlapping loops (Pattern B)
 * Two loops share some blocks but not completely nested
 * Should trigger: bitmap_intersect_p = true
 *                 both bitmap_intersect_compl_p checks = true
 *                 => neither safe_push, continue after first if
 */
__attribute__((target("arch=armv7-a")))
void partial_overlap(int n, int m) {
    int sum = 0;
    int i = 0;
    
    /* Shared setup block */
    if (n > 0 && m > 0) {
        sum = 1;
    }
    
    /* First loop */
    while (i < n) {
        sum += i;
        /* Shared computation block */
        int temp = i * 2;
        if (temp > 10) {
            /* Second loop inside condition - partially overlaps */
            for (int j = 0; j < m; j++) {
                sum += j;
                /* Early exit creates different block structure */
                if (j > 5) break;
            }
        }
        i++;
        
        /* goto creates irregular control flow */
        if (i == n/2) {
            goto skip_part;
        }
        sum += 2;
        skip_part:;
    }
    
    /* Another loop that shares the goto label block */
    for (int k = 0; k < m; k++) {
        if (k < n) {
            sum += k;
        }
    }
    
    sink = sum;
}

/* Function 3: Sibling loops with shared preheader (Pattern C)
 * Two sequential loops sharing setup blocks
 * May trigger one of the safe_push cases depending on bitmap inclusion
 */
__attribute__((target("arch=armv7-a")))
void sibling_loops(int n, int m) {
    int sum = 0;
    
    /* Shared preheader block */
    int setup = n + m;
    if (setup > 0) {
        sum = setup;
    }
    
    /* First loop */
    int i = 0;
    do {
        sum += i;
        i++;
        /* Split block with always-true condition */
        if (__builtin_expect(1, 1)) {
            /* empty */
        }
    } while (i < n);
    
    /* Shared intermediate block */
    sum *= 2;
    
    /* Second loop - shares the intermediate block in its bitmap? */
    for (int j = 0; j < m; j++) {
        sum -= j;
        /* Different internal structure */
        switch (j % 3) {
            case 0: sum += 1; break;
            case 1: sum += 2; break;
            case 2: sum += 3; break;
        }
    }
    
    sink = sum;
}

/* Function 4: Complex nested structure with multiple levels
 * Creates deeper hierarchy for more bitmap comparisons
 */
__attribute__((target("arch=armv7-a")))
void deep_nesting(int n) {
    int sum = 0;
    
    /* Level 1 */
    for (int a = 0; a < n; a++) {
        /* Level 2 */
        for (int b = 0; b < a; b++) {
            /* Conditional inner loop */
            if (b % 2 == 0) {
                /* Level 3 */
                int c = 0;
                while (c < b) {
                    sum += a * b * c;
                    c++;
                    
                    /* Innermost with break */
                    for (int d = 0; d < 3; d++) {
                        if (d == c) break;
                        sum += d;
                    }
                }
            } else {
                /* Alternative path with different loop */
                for (int e = 0; e < 2; e++) {
                    sum -= e;
                }
            }
        }
        
        /* Another loop at level 1 */
        int f = 0;
        while (f < 2) {
            sum += f;
            f++;
        }
    }
    
    sink = sum;
}

/* Function 5: Loops with early exits and goto
 * Creates complex control flow for bitmap analysis
 */
__attribute__((target("arch=armv7-a")))
void loops_with_goto(int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        if (i == n/3) {
            goto middle;
        }
        sum += i;
        continue;
        
        middle:
        for (int j = 0; j < i; j++) {
            if (j == i/2) {
                goto outer;
            }
            sum += j;
        }
        outer:;
    }
    
    /* Loop that shares the middle label */
    int k = 0;
    while (k < n) {
        sum -= k;
        k++;
        if (k == n/2) {
            goto middle2;
        }
    }
    middle2:
    sink = sum;
}

/* Main function to ensure all functions are called */
int main() {
    /* Call each function with different parameters
     * to create varied loop execution patterns
     */
    perfect_nesting(100, 50);
    partial_overlap(100, 50);
    sibling_loops(100, 50);
    deep_nesting(30);
    loops_with_goto(100);
    
    /* Compile-time check to ensure optimization */
    static_assert(sizeof(int) == 4, "int must be 4 bytes");
    
    return 0;
}
