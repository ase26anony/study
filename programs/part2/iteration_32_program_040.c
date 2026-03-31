/* hw-doloop-test.c */
/* Compile with: gcc -O2 -fdump-rtl-hw-doloop -fcompare-debug hw-doloop-test.c -o test */

#include <stdio.h>

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
    
    /* Outer loop */
    for (int i = 0; i < outer_limit; i++) {
        /* Inner loop - all blocks are within outer loop */
        for (int j = 0; j < inner_limit; j++) {
            result += i * j;
        }
        /* Some outer loop only computation */
        result += i;
    }
    
    return result;
}

/* Function 3: Partially overlapping loops (share some blocks but not all) */
__attribute__((noinline))
static int partially_overlapping(int limit) {
    volatile int acc = 0;
    
    /* Loop A with conditional branching */
    for (int i = 0; i < limit; i++) {
        if (i & 1) {
            /* Shared block - both loops execute this */
            acc += i * 3;
        } else {
            /* Unique to Loop A */
            acc -= i;
        }
    }
    
    /* Loop B - shares the if(i&1) block but has different else block */
    for (int j = 0; j < limit; j++) {
        if (j & 1) {
            /* Shared block - same as Loop A's if block */
            acc += j * 3;
        } else {
            /* Unique to Loop B - different from Loop A's else */
            acc += j * 5;
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
    }
    
    /* Loop 2 - nested inside conditional */
    for (int b = 0; b < n; b++) {
        if (b % 3 == 0) {
            /* Loop 3 - inner loop with partial overlap */
            for (int c = 0; c < b; c++) {
                total += c;
            }
        } else {
            total -= b;
        }
    }
    
    /* Loop 4 - sequential but shares some control flow with Loop 2 */
    for (int d = 0; d < n; d++) {
        if (d % 3 == 0) {
            total += d * 2;  /* Similar condition as Loop 2 */
        }
    }
    
    return total;
}

/* Function 5: Loop with early exit creating unique blocks */
__attribute__((noinline))
static int loop_with_break(int limit) {
    volatile int val = 0;
    
    for (int i = 0; i < limit; i++) {
        if (i > limit / 2) {
            break;  /* Creates unique exit block */
        }
        val += i;
    }
    
    /* Another loop that doesn't have the break */
    for (int j = 0; j < limit; j++) {
        val -= j;
    }
    
    return val;
}

int main(void) {
    int result = 0;
    
    /* Trigger disjoint loops analysis */
    result += disjoint_loops(10);
    
    /* Trigger perfectly nested analysis */
    result += perfectly_nested(5, 3);
    
    /* Trigger partially overlapping analysis */
    result += partially_overlapping(8);
    
    /* Trigger complex relationships */
    result += complex_nesting(7);
    
    /* Trigger loops with different exit blocks */
    result += loop_with_break(12);
    
    /* Use result to prevent optimization */
    global_counter = result;
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
