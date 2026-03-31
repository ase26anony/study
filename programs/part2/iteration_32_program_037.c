/* hw-doloop-coverage.c
 * Designed to trigger bitmap intersection analysis in GCC's hardware loop optimization.
 * Compile with: gcc -O2 -fdump-rtl-hw-doloop -fcompare-debug hw-doloop-coverage.c -o test
 */

#include <stdio.h>
#include <stdint.h>

volatile int global_counter = 0;

/* Function to prevent loop optimization and inlining */
__attribute__((noinline, noipa))
void disjoint_loops(volatile int* counter) {
    /* Two sequential, non-intersecting loops */
    for (int i = 0; i < 10; i++) {
        *counter += i * 2;
    }
    
    for (int j = 0; j < 15; j++) {
        *counter -= j;
    }
}

/* Perfectly nested loops - inner loop blocks are subset of outer */
__attribute__((noinline, noipa))
void perfectly_nested(volatile int* counter) {
    for (int outer = 0; outer < 8; outer++) {
        *counter += outer;
        for (int inner = 0; inner < 5; inner++) {
            *counter += inner * outer;
        }
    }
}

/* Partially overlapping loops - share some blocks but not all */
__attribute__((noinline, noipa))
void partially_overlapping(volatile int* counter) {
    volatile int local = 0;
    
    /* Loop A with conditional branching */
    for (int i = 0; i < 12; i++) {
        if (i & 1) {
            /* Shared block - both loops execute this */
            *counter += i * 3;
            local++;
        } else {
            /* Unique to Loop A */
            *counter -= i;
        }
    }
    
    /* Loop B with similar structure but different else branch */
    for (int j = 0; j < 12; j++) {
        if (j & 1) {
            /* Shared block - same as Loop A's if branch */
            *counter -= j * 2;
            local--;
        } else {
            /* Unique to Loop B */
            *counter += j * 5;
        }
    }
}

/* Complex nesting with multiple relationships */
__attribute__((noinline, noipa))
void complex_nesting(volatile int* counter) {
    /* Outer loop 1 */
    for (int a = 0; a < 6; a++) {
        *counter += a;
        
        /* Inner loop 1 - perfectly nested */
        for (int b = 0; b < 4; b++) {
            *counter += b * a;
            
            /* Innermost loop - triple nesting */
            for (int c = 0; c < 3; c++) {
                *counter -= c;
            }
        }
        
        /* Sequential loop after inner loop - partially overlapping
           with outer loop but not with inner loop */
        if (a > 2) {
            for (int d = 0; d < 3; d++) {
                *counter += d * 10;
            }
        }
    }
    
    /* Disjoint loop after the nested structure */
    for (int e = 0; e < 7; e++) {
        *counter += e * 100;
    }
}

/* Function with early exits to create complex CFG */
__attribute__((noinline, noipa))
void loops_with_early_exit(volatile int* counter) {
    /* Loop with break condition */
    for (int i = 0; i < 20; i++) {
        if (i == 15) break;
        *counter += i;
        
        /* Nested loop with continue */
        for (int j = 0; j < 8; j++) {
            if (j == 5) continue;
            *counter += j;
        }
    }
    
    /* Another loop that shares the break block */
    for (int k = 0; k < 20; k++) {
        if (k == 18) {
            *counter -= 100;  /* Shared break-like block */
            break;
        }
        *counter += k * 2;
    }
}

int main() {
    volatile int result = 0;
    
    /* Call all loop patterns to ensure all relationships are analyzed */
    disjoint_loops(&result);
    perfectly_nested(&result);
    partially_overlapping(&result);
    complex_nesting(&result);
    loops_with_early_exit(&result);
    
    /* Also update global to prevent optimization */
    global_counter = result;
    
    /* Print to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
