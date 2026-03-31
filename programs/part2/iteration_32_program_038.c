/* hw-doloop-test.c */
/* Compile with: gcc -O2 -fdump-rtl-hw-doloop -fcompare-debug hw-doloop-test.c -o test */

#include <stdio.h>

volatile int global_counter = 0;

/* Function 1: Two disjoint loops (no block intersection) */
__attribute__((noinline))
static int disjoint_loops(void) {
    volatile int local = 0;
    
    /* First loop - completely separate blocks from second loop */
    for (int i = 0; i < 100; i++) {
        local += i * 2;
    }
    
    /* Second loop - no block intersection with first loop */
    for (int j = 0; j < 50; j++) {
        local -= j;
    }
    
    return local;
}

/* Function 2: Perfectly nested loops (inner is subset of outer) */
__attribute__((noinline))
static int perfectly_nested(void) {
    volatile int sum = 0;
    
    /* Outer loop */
    for (int i = 0; i < 20; i++) {
        /* Inner loop - all blocks are within outer loop */
        for (int j = 0; j < 10; j++) {
            sum += i * j;
        }
    }
    
    return sum;
}

/* Function 3: Partially overlapping loops */
__attribute__((noinline))
static int partially_overlapping(void) {
    volatile int result = 0;
    int i, j;
    
    /* Loop A with conditional branching */
    for (i = 0; i < 30; i++) {
        if (i & 1) {
            /* Shared block - will also appear in Loop B */
            result += i * 3;
        } else {
            /* Unique to Loop A */
            result -= i;
        }
    }
    
    /* Loop B - shares the if(i & 1) block with Loop A */
    for (j = 0; j < 25; j++) {
        if (j & 1) {
            /* Shared block - same as in Loop A */
            result += j * 2;
        } else {
            /* Unique to Loop B */
            result += 100;
        }
    }
    
    return result;
}

/* Function 4: Complex nesting with multiple relationships */
__attribute__((noinline))
static int complex_nesting(void) {
    volatile int total = 0;
    
    /* Loop 1 */
    for (int a = 0; a < 15; a++) {
        total += a;
    }
    
    /* Loop 2 - nested inside Loop 3 */
    for (int b = 0; b < 10; b++) {
        /* Loop 3 - outer to Loop 2 */
        for (int c = 0; c < 8; c++) {
            total += b * c;
            
            /* Loop 4 - innermost */
            for (int d = 0; d < 5; d++) {
                total -= d;
            }
        }
    }
    
    /* Loop 5 - sequential to previous loops */
    for (int e = 0; e < 12; e++) {
        if (e % 3 == 0) {
            total += e * 2;
        }
    }
    
    return total;
}

/* Function 5: Loops with early exits creating partial overlap */
__attribute__((noinline))
static int early_exit_loops(void) {
    volatile int val = 0;
    
    /* Loop with break */
    for (int i = 0; i < 40; i++) {
        if (i > 20) {
            break;  /* Creates unique exit block */
        }
        val += i;
    }
    
    /* Loop with similar structure but no break */
    for (int j = 0; j < 40; j++) {
        if (j > 20) {
            val -= j;  /* Different block than the break above */
        } else {
            val += j;  /* Same as first loop's block when i <= 20 */
        }
    }
    
    return val;
}

int main(void) {
    int final_result = 0;
    
    /* Call all functions to ensure all loops are compiled */
    final_result += disjoint_loops();
    final_result += perfectly_nested();
    final_result += partially_overlapping();
    final_result += complex_nesting();
    final_result += early_exit_loops();
    
    /* Update global to prevent dead code elimination */
    global_counter = final_result;
    
    /* Print to ensure side effects */
    printf("Result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
