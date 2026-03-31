/* hw-doloop-test.c - Test program for hardware loop bitmap intersection analysis */

#include <stdio.h>
#include <stdlib.h>

volatile int global_counter = 0;

/* Prevent inlining to preserve loop structures */
__attribute__((noinline))
void disjoint_loops(volatile int *counter) {
    /* Two sequential loops with no block intersection */
    for (int i = 0; i < 10; i++) {
        *counter += i * 2;
    }
    
    for (int j = 0; j < 15; j++) {
        *counter -= j;
    }
}

__attribute__((noinline))
void perfectly_nested(volatile int *counter) {
    /* Outer loop completely contains inner loop */
    for (int i = 0; i < 8; i++) {
        *counter += i;
        for (int j = 0; j < 5; j++) {
            *counter += i * j;
        }
        *counter -= 1;
    }
}

__attribute__((noinline))
void partially_overlapping_1(volatile int *counter) {
    /* First loop with conditional branching */
    for (int i = 0; i < 12; i++) {
        if (i & 1) {
            /* Shared block - executed for odd i */
            *counter += i * 3;
        } else {
            /* Unique to this loop */
            *counter += 100;
        }
    }
    
    /* Second loop with similar structure but different else block */
    for (int j = 0; j < 12; j++) {
        if (j & 1) {
            /* Shared block - executed for odd j */
            *counter -= j * 2;
        } else {
            /* Unique to this loop */
            *counter -= 50;
        }
    }
}

__attribute__((noinline))
void partially_overlapping_2(volatile int *counter) {
    /* Loops that share preamble but diverge in body */
    int limit = 7;
    
    /* Loop A */
    for (int a = 0; a < limit; a++) {
        /* Shared preamble */
        int temp = a * 2;
        
        if (temp > 5) {
            /* Block unique to Loop A */
            *counter += temp * 10;
        } else {
            /* Shared middle block */
            *counter += temp;
        }
        
        /* Shared postamble */
        *counter += 1;
    }
    
    /* Loop B - shares some blocks with Loop A */
    for (int b = 0; b < limit; b++) {
        /* Shared preamble (same as Loop A) */
        int temp = b * 2;
        
        if (temp < 3) {
            /* Block unique to Loop B */
            *counter -= temp * 5;
        } else {
            /* Shared middle block (same as Loop A's else) */
            *counter -= temp;
        }
        
        /* Shared postamble */
        *counter -= 1;
    }
}

__attribute__((noinline))
void complex_nesting(volatile int *counter) {
    /* Multiple levels of nesting with varying structures */
    for (int x = 0; x < 6; x++) {
        *counter += x;
        
        /* Inner loop 1 */
        for (int y = 0; y < 4; y++) {
            *counter += x * y;
            
            /* Innermost loop */
            for (int z = 0; z < 3; z++) {
                *counter += z;
            }
        }
        
        /* Another inner loop at same level */
        for (int w = 0; w < 3; w++) {
            *counter -= w;
        }
    }
}

__attribute__((noinline))
void loops_with_break_continue(volatile int *counter) {
    /* Loops with early exits creating complex CFG */
    
    /* Loop with break */
    for (int i = 0; i < 20; i++) {
        if (i == 10) {
            break;  /* Creates additional basic block */
        }
        *counter += i;
    }
    
    /* Loop with continue */
    for (int j = 0; j < 15; j++) {
        if (j % 3 == 0) {
            continue;  /* Creates additional basic block */
        }
        *counter += j * 2;
    }
}

int main() {
    volatile int counter = 0;
    
    /* Call all loop pattern functions to create various relationships */
    disjoint_loops(&counter);
    perfectly_nested(&counter);
    partially_overlapping_1(&counter);
    partially_overlapping_2(&counter);
    complex_nesting(&counter);
    loops_with_break_continue(&counter);
    
    /* Also use global counter to prevent optimization */
    global_counter = counter;
    
    /* Print result to prevent dead code elimination */
    printf("Final counter value: %d\n", counter);
    
    return counter != 0 ? 0 : 1;
}
