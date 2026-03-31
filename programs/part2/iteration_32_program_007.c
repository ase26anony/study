/* hw-doloop-coverage.c
 * 
 * This program is designed to trigger the hardware loop optimization pass
 * in GCC, specifically exercising the bitmap intersection analysis logic
 * in hw-doloop.cc lines 429-436.
 *
 * Compile with: gcc -O2 -fdump-rtl-hw-doloop -fcompare-debug -o hw-doloop-test hw-doloop-coverage.c
 * For targets with hardware loop support: add -mdoloop if available
 */

#include <stdio.h>

volatile int global_counter = 0;

/* Function to prevent loop optimizations that might bypass analysis */
__attribute__((noinline))
void disjoint_loops(volatile int *counter) {
    /* Two sequential, non-nested loops - should be disjoint */
    for (int i = 0; i < 10; i++) {
        *counter += i * 2;
    }
    
    for (int j = 0; j < 15; j++) {
        *counter -= j;
    }
}

__attribute__((noinline))
void perfectly_nested(volatile int *counter) {
    /* Outer loop with inner loop completely inside it */
    for (int outer = 0; outer < 8; outer++) {
        *counter += outer;
        for (int inner = 0; inner < 5; inner++) {
            *counter += inner * outer;
        }
    }
}

__attribute__((noinline))
void partially_overlapping(volatile int *counter) {
    /* First loop with conditional branching */
    for (int i = 0; i < 12; i++) {
        if (i & 1) {
            /* Shared block - both loops will have this basic block */
            *counter += i * 3;
        } else {
            /* Unique to first loop */
            *counter -= i;
        }
    }
    
    /* Second loop that shares the if block but has different else block */
    for (int j = 0; j < 12; j++) {
        if (j & 1) {
            /* Shared block - same as above but different operation */
            *counter -= j * 2;
        } else {
            /* Unique to second loop */
            *counter += j * 5;
        }
    }
}

__attribute__((noinline))
void complex_nesting(volatile int *counter) {
    /* More complex nesting to create multiple loop relationships */
    for (int a = 0; a < 6; a++) {
        *counter += a;
        
        /* Inner loop 1 */
        for (int b = 0; b < 4; b++) {
            *counter += b;
        }
        
        /* Sequential inner loop 2 - shares some blocks with outer */
        for (int c = 0; c < 3; c++) {
            if (a > 2) {
                *counter += c * a;
            } else {
                *counter -= c;
            }
        }
    }
    
    /* Another disjoint loop in same function */
    for (int d = 0; d < 7; d++) {
        *counter += d * d;
    }
}

__attribute__((noinline))
void loop_with_break(volatile int *counter) {
    /* Loop with early exit to create interesting control flow */
    for (int i = 0; i < 20; i++) {
        if (i > 15) {
            break;  /* Creates additional basic blocks */
        }
        *counter += i;
        
        /* Small inner loop */
        for (int j = 0; j < 3; j++) {
            *counter += j;
        }
    }
}

int main(void) {
    volatile int counter = 0;
    
    /* Call all loop patterns to ensure various relationships are analyzed */
    disjoint_loops(&counter);
    perfectly_nested(&counter);
    partially_overlapping(&counter);
    complex_nesting(&counter);
    loop_with_break(&counter);
    
    /* Also use global counter to prevent optimization */
    global_counter = counter;
    
    /* Print result to prevent dead code elimination */
    printf("Final counter value: %d\n", counter);
    
    return counter != 0 ? 0 : 1;
}
