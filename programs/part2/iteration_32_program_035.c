/* hw-doloop-test.c */
/* Compile with: gcc -O2 -fdump-rtl-hw-doloop -fcompare-debug hw-doloop-test.c -o test */

#include <stdio.h>
#include <stdlib.h>

volatile int global_counter = 0;

/* Function 1: Two disjoint loops (no block intersection) */
__attribute__((noinline))
int disjoint_loops(int limit) {
    volatile int local_sum = 0;
    
    /* First loop - completely separate blocks from second loop */
    for (int i = 0; i < limit; i++) {
        local_sum += i * 2;
        /* Simple arithmetic to keep loop alive */
        if (i % 3 == 0) {
            local_sum -= 1;
        }
    }
    
    /* Second loop - no block intersection with first loop */
    for (int j = 0; j < limit; j++) {
        local_sum += j * 3;
        /* Different condition pattern */
        if (j % 4 == 0) {
            local_sum += 2;
        }
    }
    
    return local_sum;
}

/* Function 2: Perfectly nested loops (inner is subset of outer) */
__attribute__((noinline))
int perfectly_nested(int outer_limit, int inner_limit) {
    volatile int total = 0;
    
    /* Outer loop contains inner loop completely */
    for (int i = 0; i < outer_limit; i++) {
        total += i;
        
        /* Inner loop - all blocks are within outer loop */
        for (int j = 0; j < inner_limit; j++) {
            total += j * i;
            /* Small computation */
            if (j % 2 == 0) {
                total -= 1;
            }
        }
        
        /* More outer loop work */
        if (i % 2 == 0) {
            total *= 2;
        }
    }
    
    return total;
}

/* Function 3: Partially overlapping loops (share some blocks but not all) */
__attribute__((noinline))
int partially_overlapping(int limit) {
    volatile int result = 0;
    volatile int shared_var = 0;
    
    /* Loop A - has unique blocks and shared blocks */
    for (int i = 0; i < limit; i++) {
        /* Unique block for Loop A */
        result += i * i;
        
        /* Shared block (also appears in Loop B) */
        if (i & 1) {  /* Shared condition pattern */
            shared_var += i;
            result -= shared_var;
        } else {
            /* Unique to Loop A */
            result += 100;
        }
    }
    
    /* Loop B - overlaps with Loop A in the shared block */
    for (int j = 0; j < limit; j++) {
        /* Unique block for Loop B */
        result -= j * 3;
        
        /* Shared block (same as in Loop A) */
        if (j & 1) {  /* Same condition pattern */
            shared_var -= j;
            result += shared_var;
        } else {
            /* Unique to Loop B */
            result -= 50;
        }
    }
    
    return result;
}

/* Function 4: Complex nesting with multiple relationships */
__attribute__((noinline))
int complex_nesting(int n) {
    volatile int acc = 0;
    
    /* Loop 1: Will be disjoint from Loop 3 */
    for (int a = 0; a < n; a++) {
        acc += a;
        if (a % 3 == 0) {
            acc *= 2;
        }
    }
    
    /* Loop 2: Contains Loop 2a perfectly nested */
    for (int b = 0; b < n; b++) {
        acc -= b;
        
        /* Loop 2a: Perfectly nested inside Loop 2 */
        for (int c = 0; c < 3; c++) {
            acc += b * c;
            if (c % 2 == 0) {
                acc += 1;
            }
        }
        
        if (b % 4 == 0) {
            acc /= 2;
        }
    }
    
    /* Loop 3: Disjoint from Loop 1 */
    for (int d = 0; d < n; d++) {
        acc += d * d;
        if (d % 5 == 0) {
            acc -= 10;
        }
    }
    
    /* Loop 4: Partially overlaps with Loop 5 via shared conditional */
    for (int e = 0; e < n; e++) {
        acc += e * 3;
        /* Shared conditional block */
        if (e > n/2) {
            acc += 100;
            /* Additional shared computation */
            int temp = e * e;
            acc -= temp;
        } else {
            /* Unique to Loop 4 */
            acc += 50;
        }
    }
    
    /* Loop 5: Shares conditional block with Loop 4 */
    for (int f = 0; f < n; f++) {
        acc -= f * 2;
        /* Same shared conditional */
        if (f > n/2) {
            acc += 100;
            /* Same shared computation */
            int temp = f * f;
            acc -= temp;
        } else {
            /* Unique to Loop 5 */
            acc -= 25;
        }
    }
    
    return acc;
}

/* Function 5: Loops with early exits to create complex CFG */
__attribute__((noinline))
int loops_with_early_exit(int limit) {
    volatile int sum = 0;
    
    /* Loop with possible early break */
    for (int i = 0; i < limit; i++) {
        sum += i;
        if (sum > 1000) {
            /* Early exit creates unique block */
            sum -= 500;
            break;
        }
        if (i % 7 == 0) {
            sum += 10;
        }
    }
    
    /* Another loop with different early exit pattern */
    for (int j = 0; j < limit; j++) {
        sum -= j;
        if (sum < -500) {
            /* Different early exit */
            sum += 250;
            break;
        }
        if (j % 5 == 0) {
            sum -= 5;
        }
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Call all functions to ensure all loops are compiled */
    int result = 0;
    
    result += disjoint_loops(iterations);
    result += perfectly_nested(iterations, 5);
    result += partially_overlapping(iterations);
    result += complex_nesting(iterations);
    result += loops_with_early_exit(iterations);
    
    /* Update global to prevent optimization */
    global_counter = result;
    
    printf("Result: %d\n", result);
    printf("Global counter: %d\n", global_counter);
    
    return result != 0 ? 0 : 1;
}
