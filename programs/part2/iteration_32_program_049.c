/* hw-doloop-test.c - Test program for hardware loop bitmap intersection analysis */

#include <stdio.h>
#include <stdlib.h>

volatile int global_counter = 0;

/* Function to prevent loop optimization and ensure loops remain as candidates */
__attribute__((noinline, noipa))
void disjoint_loops(volatile int* counter) {
    /* Two sequential loops with disjoint basic blocks */
    for (int i = 0; i < 100; ++i) {
        *counter += i * 2;
    }
    
    for (int j = 0; j < 50; ++j) {
        *counter -= j;
    }
}

__attribute__((noinline, noipa))
void perfectly_nested(volatile int* counter) {
    /* Outer loop containing inner loop - perfect nesting */
    for (int outer = 0; outer < 20; ++outer) {
        *counter += outer;
        for (int inner = 0; inner < 10; ++inner) {
            *counter += inner * outer;
        }
    }
}

__attribute__((noinline, noipa))
void partially_overlapping_loops(volatile int* counter) {
    int temp = 0;
    
    /* First loop with conditional branching */
    for (int i = 0; i < 30; ++i) {
        if (i & 1) {
            /* Shared block - will appear in both loops' bitmaps */
            temp = *counter + i;
            *counter = temp;
        } else {
            /* Unique to first loop */
            *counter += i * 3;
        }
    }
    
    /* Second loop that shares the if block but has different else block */
    for (int j = 0; j < 25; ++j) {
        if (j & 1) {
            /* Shared block - same basic block as in first loop */
            temp = *counter - j;
            *counter = temp;
        } else {
            /* Unique to second loop */
            *counter -= j * 2;
        }
    }
}

__attribute__((noinline, noipa))
void complex_nesting_pattern(volatile int* counter) {
    /* Create more complex relationships between loops */
    
    /* Loop A */
    for (int a = 0; a < 15; ++a) {
        *counter += a;
        
        /* Loop B nested in A */
        for (int b = 0; b < 8; ++b) {
            *counter += b;
            
            /* Loop C deeply nested */
            for (int c = 0; c < 5; ++c) {
                *counter += c;
            }
        }
    }
    
    /* Loop D - partially overlaps with A's blocks through shared condition */
    for (int d = 0; d < 12; ++d) {
        if (d % 3 == 0) {
            /* This block might be shared with loop A's if we add similar logic */
            *counter += d * 10;
        } else {
            *counter += d;
        }
    }
}

__attribute__((noinline, noipa))
void loops_with_breaks(volatile int* counter) {
    /* Loops with early exits create more complex block relationships */
    
    /* Loop with conditional break */
    for (int i = 0; i < 40; ++i) {
        if (i > 25) {
            break;  /* Creates additional basic blocks */
        }
        *counter += i;
    }
    
    /* Another loop that might share some blocks */
    for (int j = 0; j < 35; ++j) {
        if (j < 10) {
            *counter += j * 2;
        } else {
            /* This else block is unique */
            *counter -= j;
        }
    }
}

int main(void) {
    volatile int counter = 0;
    
    /* Call all loop patterns to ensure the hardware loop pass analyzes them */
    disjoint_loops(&counter);
    perfectly_nested(&counter);
    partially_overlapping_loops(&counter);
    complex_nesting_pattern(&counter);
    loops_with_breaks(&counter);
    
    /* Also use global counter to create additional loop relationships */
    for (int i = 0; i < 10; ++i) {
        global_counter += i;
        for (int j = 0; j < 5; ++j) {
            global_counter += j;
        }
    }
    
    /* Mix local and global counters to prevent optimization */
    counter += global_counter;
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", counter);
    
    return counter != 0 ? 0 : 1;
}
