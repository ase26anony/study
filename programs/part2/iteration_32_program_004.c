/* hw-doloop-coverage.c
 * 
 * This program is designed to trigger the hardware loop optimization
 * pass in GCC, specifically to cover the bitmap intersection analysis
 * in hw-doloop.cc lines 429-436.
 *
 * Compile with: gcc -O2 -fdump-rtl-hw-doloop -fcompare-debug -o hw-doloop-coverage hw-doloop-coverage.c
 * For targets with hardware loop support: add -mdoloop if available
 */

#include <stdio.h>

volatile int global_counter = 0;

/* Function 1: Two disjoint loops (no block intersection) */
__attribute__((noinline))
int disjoint_loops(int limit) {
    volatile int local = 0;
    
    /* First loop - completely separate basic blocks */
    for (int i = 0; i < limit; i++) {
        local += i * 2;
    }
    
    /* Second loop - separate, non-intersecting */
    for (int j = limit; j > 0; j--) {
        local -= j / 2;
    }
    
    return local;
}

/* Function 2: Perfectly nested loops (inner is subset of outer) */
__attribute__((noinline))
int perfectly_nested(int outer_limit, int inner_limit) {
    volatile int sum = 0;
    
    for (int i = 0; i < outer_limit; i++) {
        /* Outer loop preamble */
        sum += i;
        
        /* Inner loop - all blocks are within outer loop */
        for (int j = 0; j < inner_limit; j++) {
            sum += (i * j);
        }
        
        /* Outer loop postamble */
        sum -= i;
    }
    
    return sum;
}

/* Function 3: Partially overlapping loops (share some blocks but not all) */
__attribute__((noinline))
int partially_overlapping(int limit) {
    volatile int result = 0;
    
    /* Loop A - has unique blocks in else branch */
    for (int i = 0; i < limit; i++) {
        if (i & 1) {
            /* Shared block with Loop B */
            result += i * 3;
            global_counter++;
        } else {
            /* Unique to Loop A */
            result -= i;
            /* Additional unique operation */
            for (int k = 0; k < 2; k++) {
                result += k;
            }
        }
    }
    
    /* Loop B - shares the if block but has different else */
    for (int j = 0; j < limit; j++) {
        if (j & 1) {
            /* Shared block with Loop A */
            result -= j * 2;
            global_counter--;
        } else {
            /* Unique to Loop B */
            result += j * j;
            /* Different unique operation */
            volatile int temp = j;
            while (temp > 0) {
                result++;
                temp--;
            }
        }
    }
    
    return result;
}

/* Function 4: Complex nesting with multiple relationships */
__attribute__((noinline))
int complex_nesting(int n) {
    volatile int acc = 0;
    
    /* Loop 1 */
    for (int a = 0; a < n; a++) {
        acc += a;
        
        /* Loop 2 - nested inside Loop 1 */
        for (int b = 0; b < 3; b++) {
            acc -= b;
            
            /* Loop 3 - deeply nested */
            for (int c = 0; c < 2; c++) {
                acc += a * b * c;
            }
        }
    }
    
    /* Loop 4 - disjoint from the triple-nested loops above */
    int temp = n;
    while (temp > 0) {
        acc += temp;
        temp--;
    }
    
    /* Loop 5 - partially overlaps with Loop 1 through shared condition */
    for (int d = 0; d < n; d++) {
        if (d % 3 == 0) {
            /* Block that might be similar to one in Loop 1 */
            acc += d * 10;
        } else {
            /* Unique block */
            for (int e = 0; e < 2; e++) {
                acc -= e;
            }
        }
    }
    
    return acc;
}

int main() {
    volatile int total = 0;
    int iterations = 10;
    
    /* Trigger all loop patterns */
    total += disjoint_loops(iterations);
    total += perfectly_nested(5, 3);
    total += partially_overlapping(iterations);
    total += complex_nesting(7);
    
    /* Add global counter to prevent elimination */
    total += global_counter;
    
    /* Print to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
