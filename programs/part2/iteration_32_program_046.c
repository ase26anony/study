/* hw-doloop-test.c */
/* Compile with: gcc -O2 -fdump-rtl-hw-doloop -fcompare-debug hw-doloop-test.c -o test */

#include <stdio.h>

volatile int global_counter = 0;

/* Function 1: Two disjoint loops (no block intersection) */
__attribute__((noinline))
static int disjoint_loops(void) {
    volatile int local = 0;
    /* First loop - completely separate blocks */
    for (int i = 0; i < 100; i++) {
        local += i * 2;
    }
    
    /* Second loop - separate from first */
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
        sum += outer;
        
        /* Inner loop - completely within outer */
        for (int inner = 0; inner < 10; inner++) {
            sum += inner * outer;
        }
    }
    
    return sum;
}

/* Function 3: Partially overlapping loops (share some blocks but not all) */
__attribute__((noinline))
static int partially_overlapping(void) {
    volatile int result = 0;
    
    /* Loop A */
    for (int i = 0; i < 30; i++) {
        if (i & 1) {
            /* Shared block - both loops execute this */
            result += i * 3;
        } else {
            /* Unique to Loop A */
            result -= i;
        }
    }
    
    /* Loop B - shares the if(i & 1) block but has different else */
    for (int j = 0; j < 30; j++) {
        if (j & 1) {
            /* Shared block - same as Loop A's if block */
            result += j * 3;
        } else {
            /* Unique to Loop B - different from Loop A's else */
            result += j * 5;
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
        
        /* Loop 2 - nested inside Loop 1 */
        for (int b = 0; b < 10; b++) {
            total += b;
            
            /* Loop 3 - deeply nested */
            for (int c = 0; c < 5; c++) {
                total += c;
            }
        }
    }
    
    /* Loop 4 - disjoint from the triple-nested loops above */
    for (int d = 0; d < 25; d++) {
        total -= d;
    }
    
    /* Loop 5 - partially overlaps with Loop 4 via shared condition */
    for (int e = 0; e < 25; e++) {
        if (e % 3 == 0) {
            total += e * 2;
        } else {
            total += e;
        }
    }
    
    return total;
}

/* Function 5: Loops with early exits creating partial overlap */
__attribute__((noinline))
static int early_exit_loops(void) {
    volatile int val = 0;
    
    /* Loop with early continue */
    for (int x = 0; x < 40; x++) {
        if (x < 10) {
            continue;  /* Early path */
        }
        val += x;
    }
    
    /* Loop with similar structure but different condition */
    for (int y = 0; y < 40; y++) {
        if (y < 15) {
            continue;  /* Similar but not identical early path */
        }
        val -= y;
    }
    
    return val;
}

int main(void) {
    int final_result = 0;
    
    /* Execute all loop patterns to trigger various bitmap intersections */
    final_result += disjoint_loops();
    final_result += perfectly_nested();
    final_result += partially_overlapping();
    final_result += complex_nesting();
    final_result += early_exit_loops();
    
    /* Use the result to prevent optimization */
    global_counter = final_result;
    
    printf("Result: %d\n", final_result);
    return final_result != 0 ? 0 : 1;
}
