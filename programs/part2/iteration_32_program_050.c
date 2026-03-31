/* hw-doloop-test.c */
/* Compile with: gcc -O2 -fdump-rtl-hw-doloop -fcompare-debug hw-doloop-test.c -o test */

#include <stdio.h>

volatile int global_counter = 0;

/* Function to prevent loop unrolling and maintain loop structure */
__attribute__((noinline))
void disjoint_loops(volatile int* counter) {
    /* Two sequential, non-intersecting loops */
    for (int i = 0; i < 10; i++) {
        *counter += i * 2;
    }
    
    /* This loop's blocks should not intersect with the first loop's blocks */
    for (int j = 0; j < 15; j++) {
        *counter -= j;
    }
}

__attribute__((noinline))
void perfectly_nested(volatile int* counter) {
    /* Outer loop containing inner loop - perfect nesting */
    for (int outer = 0; outer < 8; outer++) {
        *counter += outer;
        
        /* Inner loop - all blocks are subset of outer loop's blocks */
        for (int inner = 0; inner < 5; inner++) {
            *counter += inner * 3;
        }
    }
}

__attribute__((noinline))
void partially_overlapping(volatile int* counter) {
    int temp = 0;
    
    /* First loop with conditional branching */
    for (int i = 0; i < 12; i++) {
        if (i & 1) {  /* Shared block when condition true */
            temp += i * 2;
            *counter += 1;
        } else {      /* Unique block to this loop */
            temp -= i;
        }
    }
    
    /* Second loop that shares the if-true block but has different else block */
    for (int j = 0; j < 12; j++) {
        if (j & 1) {  /* Shared block - same condition as above */
            temp += j * 3;
            *counter -= 1;
        } else {      /* Different unique block */
            temp += j * 5;
        }
    }
    
    /* Use temp to prevent dead code elimination */
    *counter += temp & 1;
}

__attribute__((noinline))
void complex_nesting(volatile int* counter) {
    /* Multiple levels of nesting with different structures */
    
    /* Loop A */
    for (int a = 0; a < 7; a++) {
        *counter += a;
        
        /* Loop B - nested in A */
        for (int b = 0; b < 4; b++) {
            *counter += b * 2;
            
            /* Loop C - deeply nested */
            for (int c = 0; c < 3; c++) {
                *counter += c * 3;
            }
        }
    }
    
    /* Loop D - disjoint from the triple-nested loops above */
    for (int d = 0; d < 9; d++) {
        *counter -= d;
    }
}

__attribute__((noinline))
void loops_with_breaks(volatile int* counter) {
    /* Loop with early exit - creates more complex CFG */
    for (int i = 0; i < 20; i++) {
        if (i > 15) {
            break;  /* Creates additional basic block */
        }
        *counter += i;
    }
    
    /* Another loop with continue */
    for (int j = 0; j < 20; j++) {
        if (j % 3 == 0) {
            continue;  /* Creates additional control flow */
        }
        *counter += j * 2;
    }
}

int main() {
    volatile int counter = 0;
    
    /* Call all loop patterns to ensure they're analyzed together */
    disjoint_loops(&counter);
    perfectly_nested(&counter);
    partially_overlapping(&counter);
    complex_nesting(&counter);
    loops_with_breaks(&counter);
    
    /* Also use global counter to create additional loop relationships */
    for (int i = 0; i < 5; i++) {
        global_counter += i;
        for (int j = 0; j < 3; j++) {
            global_counter += j;
        }
    }
    
    /* Mix local and global counters */
    counter += global_counter;
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", counter);
    
    return counter != 0 ? 0 : 1;
}
