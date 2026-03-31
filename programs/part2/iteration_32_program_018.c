/* hw-doloop-test.c */
/* Compile with: gcc -O2 -fdump-rtl-hw-doloop -fcompare-debug -o test hw-doloop-test.c */

#include <stdio.h>
#include <stdlib.h>

volatile int global_counter = 0;

/* Function 1: Two disjoint loops (no block intersection) */
__attribute__((noinline))
static int disjoint_loops(void) {
    volatile int local = 0;
    
    /* First loop - completely separate blocks from second loop */
    for (int i = 0; i < 100; i++) {
        local += i * 2;
    }
    
    /* Some intermediate code to ensure separation */
    int temp = local;
    local = temp * 3;
    
    /* Second loop - disjoint from first */
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
    for (int i = 0; i < 20; i++) {
        /* Some outer loop work */
        sum += i;
        
        /* Inner loop - all blocks are within outer loop */
        for (int j = 0; j < 10; j++) {
            sum += i * j;
        }
        
        /* More outer loop work */
        sum -= i;
    }
    
    return sum;
}

/* Function 3: Partially overlapping loops */
__attribute__((noinline))
static int partially_overlapping(void) {
    volatile int result = 0;
    int shared_var = 0;
    
    /* Loop A */
    for (int a = 0; a < 30; a++) {
        /* Block unique to Loop A */
        result += a * 3;
        
        /* Shared block - both loops execute this when condition is true */
        if (a & 1) {
            shared_var += a;
            result -= shared_var;
        } else {
            /* Unique block for Loop A's else path */
            result += 100;
        }
    }
    
    /* Some intermediate computation */
    shared_var = result % 7;
    
    /* Loop B - partially overlaps with Loop A */
    for (int b = 0; b < 25; b++) {
        /* Block unique to Loop B */
        result -= b * 2;
        
        /* Shared block - same structure as in Loop A but different operation */
        if (b & 1) {
            shared_var -= b;
            result += shared_var;
        } else {
            /* Unique block for Loop B's else path */
            result -= 50;
        }
    }
    
    return result;
}

/* Function 4: Complex nesting with multiple relationships */
__attribute__((noinline))
static int complex_nesting(void) {
    volatile int total = 0;
    
    /* Loop X */
    for (int x = 0; x < 15; x++) {
        total += x;
        
        /* Loop Y - nested inside X */
        for (int y = 0; y < 8; y++) {
            total += x * y;
            
            /* Loop Z - deeply nested */
            for (int z = 0; z < 5; z++) {
                total -= z;
            }
        }
        
        /* Sequential loop after Y but still inside X */
        for (int w = 0; w < 3; w++) {
            total += w * 10;
        }
    }
    
    /* Loop outside X but in same function */
    for (int outer = 0; outer < 10; outer++) {
        total += outer * 100;
    }
    
    return total;
}

/* Function 5: Loops with early exits creating complex CFG */
__attribute__((noinline))
static int loops_with_early_exit(void) {
    volatile int val = 0;
    
    /* Loop with break condition */
    for (int i = 0; i < 40; i++) {
        val += i;
        if (i > 20) {
            /* Early exit creates unique block */
            break;
        }
        val -= i / 2;
    }
    
    /* Another loop with continue */
    for (int j = 0; j < 35; j++) {
        if (j % 3 == 0) {
            /* Skip iteration - creates shared continue block */
            continue;
        }
        val += j * 3;
        
        /* Additional unique block */
        if (j > 15) {
            val -= 5;
        }
    }
    
    return val;
}

int main(void) {
    int final_result = 0;
    
    /* Execute all loop patterns to ensure they're compiled */
    final_result += disjoint_loops();
    final_result += perfectly_nested();
    final_result += partially_overlapping();
    final_result += complex_nesting();
    final_result += loops_with_early_exit();
    
    /* Update global to prevent dead code elimination */
    global_counter = final_result;
    
    /* Print result to ensure side effects */
    printf("Result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
