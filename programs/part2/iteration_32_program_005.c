/* hw-doloop-coverage.c
 * Program designed to trigger hardware loop optimization bitmap intersection analysis
 * Compile with: gcc -O2 -fdump-rtl-hw-doloop -fcompare-debug hw-doloop-coverage.c -o hw-doloop-coverage
 */

#include <stdio.h>
#include <stdlib.h>

volatile int global_counter = 0;

/* Function 1: Two disjoint loops (no block intersection) */
__attribute__((noinline))
static int disjoint_loops(int limit) {
    volatile int local_sum = 0;
    
    /* First loop - completely separate blocks from second loop */
    for (int i = 0; i < limit; i++) {
        local_sum += i * 2;
    }
    
    /* Second loop - no block intersection with first loop */
    for (int j = 0; j < limit; j++) {
        local_sum -= j;
    }
    
    return local_sum;
}

/* Function 2: Perfectly nested loops (inner loop blocks are subset of outer) */
__attribute__((noinline))
static int perfectly_nested(int outer_limit, int inner_limit) {
    volatile int result = 0;
    
    for (int i = 0; i < outer_limit; i++) {
        /* Outer loop preamble */
        result += i * 10;
        
        /* Inner loop - all blocks are within outer loop */
        for (int j = 0; j < inner_limit; j++) {
            result += j + i;
        }
        
        /* Outer loop postamble */
        result -= i * 5;
    }
    
    return result;
}

/* Function 3: Partially overlapping loops (share some blocks but not all) */
__attribute__((noinline))
static int partially_overlapping(int limit) {
    volatile int acc = 0;
    
    /* Loop A - has unique blocks in else branch */
    for (int i = 0; i < limit; i++) {
        if (i & 1) {  /* Shared block with Loop B */
            acc += i * 3;
            global_counter++;  /* Side effect to prevent elimination */
        } else {  /* Unique to Loop A */
            acc -= i;
            /* Additional unique operations */
            for (int k = 0; k < 2; k++) {
                acc += k;
            }
        }
    }
    
    /* Loop B - shares the if block but has different else branch */
    for (int j = 0; j < limit; j++) {
        if (j & 1) {  /* Shared block with Loop A */
            acc -= j * 2;
            global_counter--;  /* Side effect to prevent elimination */
        } else {  /* Unique to Loop B */
            acc += j * 5;
            /* Different unique operations */
            for (int m = 0; m < 3; m++) {
                acc -= m;
            }
        }
    }
    
    return acc;
}

/* Function 4: Complex nesting with multiple relationships */
__attribute__((noinline))
static int complex_nesting(int n) {
    volatile int total = 0;
    
    /* Loop 1 */
    for (int a = 0; a < n; a++) {
        total += a;
        
        /* Loop 2 - nested inside Loop 1 */
        for (int b = 0; b < 3; b++) {
            total += b * a;
            
            /* Loop 3 - deeply nested */
            for (int c = 0; c < 2; c++) {
                total -= c;
            }
        }
    }
    
    /* Loop 4 - disjoint from the triple-nested loops above */
    for (int d = 0; d < n; d++) {
        total += d * d;
    }
    
    /* Loop 5 - partially overlaps with Loop 4 through shared conditional */
    for (int e = 0; e < n; e++) {
        if (e % 3 == 0) {
            total += e * 10;  /* Shared block */
        } else {
            total -= e;  /* Unique to Loop 5 */
        }
    }
    
    return total;
}

/* Function 5: Loops with early exits creating complex block relationships */
__attribute__((noinline))
static int loops_with_early_exit(int limit) {
    volatile int val = 0;
    
    /* Loop with conditional continue */
    for (int i = 0; i < limit; i++) {
        if (i % 4 == 0) {
            continue;  /* Creates additional basic block */
        }
        val += i;
        
        /* Nested loop with break */
        for (int j = 0; j < 5; j++) {
            if (j == 3) {
                break;  /* Creates exit block */
            }
            val += j * i;
        }
    }
    
    /* Another loop that shares some control flow patterns */
    for (int k = 0; k < limit; k++) {
        if (k % 4 == 0) {
            /* Shared pattern with first loop's continue block */
            val -= k;
            continue;
        }
        val += k * 2;
    }
    
    return val;
}

int main(int argc, char **argv) {
    /* Use command line args to prevent constant folding */
    int base = argc > 1 ? atoi(argv[1]) : 10;
    
    /* Call all loop patterns to ensure they're analyzed together */
    int result = 0;
    
    result += disjoint_loops(base);
    result += perfectly_nested(base, base / 2);
    result += partially_overlapping(base);
    result += complex_nesting(base);
    result += loops_with_early_exit(base);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d (global_counter: %d)\n", result, global_counter);
    
    return result != 0 ? 0 : 1;
}
