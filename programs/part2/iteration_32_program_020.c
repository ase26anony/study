/* hw-doloop-test.c */
/* Compile with: gcc -O2 -fdump-rtl-hw-doloop -fcompare-debug hw-doloop-test.c -o test */

#include <stdio.h>
#include <stdlib.h>

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
    for (int outer = 0; outer < 20; outer++) {
        /* Inner loop - all blocks are within outer loop */
        for (int inner = 0; inner < 10; inner++) {
            sum += outer * inner;
        }
    }
    
    return sum;
}

/* Function 3: Partially overlapping loops */
__attribute__((noinline))
static int partially_overlapping(void) {
    volatile int result = 0;
    
    /* Loop A with conditional branching */
    for (int i = 0; i < 30; i++) {
        if (i & 1) {
            /* Shared block - executed for odd i */
            result += i * 3;
        } else {
            /* Unique to Loop A - executed for even i */
            result += i;
        }
    }
    
    /* Loop B with similar but not identical structure */
    for (int j = 0; j < 30; j++) {
        if (j & 1) {
            /* Shared block - same basic block pattern as Loop A's if-true path */
            result -= j * 2;
        } else {
            /* Unique to Loop B - different from Loop A's else path */
            result -= j / 2;
        }
    }
    
    return result;
}

/* Function 4: Complex nesting with multiple relationships */
__attribute__((noinline))
static int complex_nesting(void) {
    volatile int acc = 0;
    
    /* Loop X */
    for (int x = 0; x < 15; x++) {
        acc += x;
        
        /* Loop Y - nested inside X */
        for (int y = 0; y < 8; y++) {
            acc += y;
            
            /* Loop Z - deeply nested */
            for (int z = 0; z < 5; z++) {
                acc += z;
            }
        }
    }
    
    /* Loop W - disjoint from the X-Y-Z nest */
    for (int w = 0; w < 12; w++) {
        acc -= w;
    }
    
    return acc;
}

/* Function 5: Loops with early exits creating partial overlap */
__attribute__((noinline))
static int early_exit_loops(void) {
    volatile int val = 0;
    
    /* Loop with break condition */
    for (int a = 0; a < 25; a++) {
        if (a > 15) {
            break;  /* Creates unique exit block */
        }
        val += a;
    }
    
    /* Similar loop but different break condition */
    for (int b = 0; b < 25; b++) {
        if (b > 18) {
            break;  /* Different exit block */
        }
        val -= b;
    }
    
    return val;
}

int main(void) {
    int total = 0;
    
    /* Call all functions to ensure all loops are compiled */
    total += disjoint_loops();
    total += perfectly_nested();
    total += partially_overlapping();
    total += complex_nesting();
    total += early_exit_loops();
    
    /* Use volatile global to prevent optimization */
    global_counter = total;
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
