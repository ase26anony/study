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
    
    /* Outer loop - contains all blocks of inner loop */
    for (int outer = 0; outer < 20; outer++) {
        /* Inner loop - all blocks are within outer loop's blocks */
        for (int inner = 0; inner < 10; inner++) {
            sum += outer * inner;
        }
    }
    
    return sum;
}

/* Function 3: Partially overlapping loops (share some blocks but not all) */
__attribute__((noinline))
static int partially_overlapping(void) {
    volatile int result = 0;
    
    /* Loop A: Has unique blocks in else branch */
    for (int i = 0; i < 30; i++) {
        if (i & 1) {
            /* Shared block - both loops execute this */
            result += i * 3;
        } else {
            /* Unique to Loop A */
            result -= i;
        }
    }
    
    /* Loop B: Shares the if block but has different else block */
    for (int j = 0; j < 30; j++) {
        if (j & 1) {
            /* Shared block - same as Loop A's if block */
            result += j * 2;
        } else {
            /* Unique to Loop B - different from Loop A's else */
            result += 100;
        }
    }
    
    return result;
}

/* Function 4: Complex nesting with multiple relationships */
__attribute__((noinline))
static int complex_nesting(void) {
    volatile int total = 0;
    int x, y, z;
    
    /* Loop X */
    for (x = 0; x < 15; x++) {
        total += x;
        
        /* Loop Y - nested inside X */
        for (y = 0; y < 8; y++) {
            total += y;
            
            /* Loop Z - deeply nested */
            for (z = 0; z < 5; z++) {
                total += z;
            }
        }
    }
    
    /* Another disjoint loop after the nested ones */
    for (int w = 0; w < 25; w++) {
        total -= w;
    }
    
    return total;
}

/* Function 5: Loops with early exits creating partial overlap */
__attribute__((noinline))
static int loops_with_breaks(void) {
    volatile int val = 0;
    
    /* Loop with conditional break */
    for (int i = 0; i < 40; i++) {
        val += i;
        if (i > 20) {
            break;  /* Creates unique exit block */
        }
    }
    
    /* Similar loop but different break condition */
    for (int j = 0; j < 40; j++) {
        val += j * 2;
        if (j > 25) {
            break;  /* Different exit condition */
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
    final_result += loops_with_breaks();
    
    /* Use volatile to prevent optimization */
    global_counter = final_result;
    
    /* Print to prevent dead code elimination */
    printf("Result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
