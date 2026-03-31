/* hw-doloop-test.c */
/* Compile with: gcc -O2 -fdump-rtl-hw-doloop -fcompare-debug hw-doloop-test.c -o test */

#include <stdio.h>

volatile int global_counter = 0;

/* Function 1: Two disjoint loops (no block intersection) */
__attribute__((noinline))
int disjoint_loops(void) {
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
int perfectly_nested(void) {
    volatile int sum = 0;
    
    /* Outer loop */
    for (int i = 0; i < 20; i++) {
        /* Some preamble unique to outer loop */
        sum += i;
        
        /* Inner loop - all blocks are within outer loop */
        for (int j = 0; j < 10; j++) {
            sum += i * j;
        }
        
        /* Some postamble unique to outer loop */
        sum -= i;
    }
    
    return sum;
}

/* Function 3: Partially overlapping loops (share some blocks but not all) */
__attribute__((noinline))
int partially_overlapping(void) {
    volatile int result = 0;
    
    /* Loop A with conditional branching */
    for (int i = 0; i < 30; i++) {
        /* Block 1: Shared preamble (executed by both loops) */
        result += i;
        
        if (i & 1) {
            /* Block 2: Shared when condition true */
            result *= 2;
            global_counter++;
        } else {
            /* Block 3: Unique to Loop A */
            result -= 5;
        }
        
        /* Block 4: Shared postamble */
        result += 1;
    }
    
    /* Some separating code */
    int separator = result;
    result = separator / 2;
    
    /* Loop B - shares blocks 1, 2, and 4 with Loop A, but has different else block */
    for (int j = 0; j < 30; j++) {
        /* Block 1: Shared preamble (same as Loop A) */
        result += j;
        
        if (j & 1) {
            /* Block 2: Shared when condition true (same as Loop A) */
            result *= 2;
            global_counter--;
        } else {
            /* Block 5: Unique to Loop B (different from Block 3) */
            result += 10;
        }
        
        /* Block 4: Shared postamble (same as Loop A) */
        result += 1;
    }
    
    return result;
}

/* Function 4: Complex nesting with multiple relationships */
__attribute__((noinline))
int complex_nesting(void) {
    volatile int acc = 0;
    
    /* Loop X */
    for (int x = 0; x < 15; x++) {
        acc += x;
        
        /* Loop Y - nested inside X */
        for (int y = 0; y < 8; y++) {
            acc += x * y;
            
            /* Loop Z - deeply nested */
            for (int z = 0; z < 5; z++) {
                acc -= z;
            }
        }
    }
    
    /* Loop W - disjoint from the X-Y-Z nest */
    for (int w = 0; w < 12; w++) {
        acc += w * 3;
    }
    
    return acc;
}

/* Function 5: Loops with early exits creating partial overlap */
__attribute__((noinline))
int early_exit_loops(void) {
    volatile int val = 0;
    
    /* Loop with break */
    for (int i = 0; i < 25; i++) {
        val += i;
        if (i > 15) {
            break;  /* Creates unique exit block */
        }
        val *= 2;
    }
    
    /* Loop with similar structure but no break */
    for (int j = 0; j < 25; j++) {
        val += j;
        if (j > 15) {
            val /= 2;  /* Different from break */
        }
        val *= 2;
    }
    
    return val;
}

int main(void) {
    int total = 0;
    
    /* Call all functions to ensure all loops are compiled */
    total += disjoint_loops();
    total += perfectly_nested();
    total += partially_overlapping();
    total += complex_nesting();
    total += early_exit_loops();
    
    /* Use global_counter to prevent optimization */
    total += global_counter;
    
    printf("Result: %d\n", total);
    
    /* Return non-zero to ensure all code paths matter */
    return total != 0 ? 0 : 1;
}
