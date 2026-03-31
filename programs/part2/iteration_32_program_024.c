/* hw-doloop-test.c */
/* Compile with: gcc -O2 -fdump-rtl-hw-doloop -fcompare-debug hw-doloop-test.c -o test */

#include <stdio.h>

volatile int global_counter = 0;

/* Function 1: Two disjoint loops (no block intersection) */
__attribute__((noinline))
int disjoint_loops(void) {
    volatile int local = 0;
    
    /* First loop - completely separate basic blocks */
    for (int i = 0; i < 100; i++) {
        local += i * 2;
    }
    
    /* Second loop - separate from first, no intersection */
    for (int j = 0; j < 50; j++) {
        local -= j / 2;
    }
    
    return local;
}

/* Function 2: Perfectly nested loops (inner is subset of outer) */
__attribute__((noinline))
int perfectly_nested(void) {
    volatile int sum = 0;
    
    /* Outer loop */
    for (int i = 0; i < 20; i++) {
        /* Inner loop - all blocks are within outer loop */
        for (int j = 0; j < 10; j++) {
            sum += i * j;
        }
        /* Some outer-loop-only code */
        sum += i;
    }
    
    return sum;
}

/* Function 3: Partially overlapping loops */
__attribute__((noinline))
int partially_overlapping(void) {
    volatile int result = 0;
    int temp = 0;
    
    /* Loop A */
    for (int i = 0; i < 30; i++) {
        if (i & 1) {
            /* Shared block with Loop B */
            result += i * 3;
            temp = i * 2;  /* Additional operation in shared block */
        } else {
            /* Unique to Loop A */
            result -= i;
        }
    }
    
    /* Loop B - shares the if(i & 1) block but has different else block */
    for (int j = 0; j < 25; j++) {
        if (j & 1) {
            /* Shared block with Loop A */
            result -= j * 2;
            temp = j * 3;  /* Different operation in shared block */
        } else {
            /* Unique to Loop B */
            result += j * 4;
        }
    }
    
    return result + temp;
}

/* Function 4: Complex nesting with multiple relationships */
__attribute__((noinline))
int complex_nesting(void) {
    volatile int acc = 0;
    
    /* Loop 1 */
    for (int a = 0; a < 15; a++) {
        acc += a;
        
        /* Loop 2 - nested inside Loop 1 */
        for (int b = 0; b < 8; b++) {
            acc += b;
            
            /* Loop 3 - deeply nested */
            for (int c = 0; c < 5; c++) {
                acc += c;
            }
        }
    }
    
    /* Loop 4 - separate from the nested structure above */
    for (int d = 0; d < 12; d++) {
        acc -= d;
    }
    
    /* Loop 5 - partially overlaps with Loop 4 through shared condition */
    for (int e = 0; e < 10; e++) {
        if (e < 5) {
            /* Shared with Loop 6 */
            acc += e * 2;
        } else {
            /* Unique to Loop 5 */
            acc -= e;
        }
    }
    
    /* Loop 6 - shares condition block with Loop 5 */
    for (int f = 0; f < 8; f++) {
        if (f < 5) {
            /* Shared with Loop 5 */
            acc += f * 3;
        } else {
            /* Unique to Loop 6 */
            acc += f * 10;
        }
    }
    
    return acc;
}

/* Function 5: Loops with early exits creating partial overlap */
__attribute__((noinline))
int early_exit_loops(void) {
    volatile int val = 0;
    
    /* Loop with early continue */
    for (int i = 0; i < 40; i++) {
        if (i % 3 == 0) {
            continue;  /* Creates separate basic block */
        }
        val += i;
        
        if (i > 20) {
            break;  /* Another separate block */
        }
    }
    
    /* Another loop with similar pattern but different conditions */
    for (int j = 0; j < 35; j++) {
        if (j % 4 == 0) {
            continue;  /* Similar structure but different condition */
        }
        val -= j;
        
        if (j > 15) {
            break;
        }
    }
    
    return val;
}

int main(void) {
    int total = 0;
    
    /* Execute all loop patterns to ensure they're compiled */
    total += disjoint_loops();
    total += perfectly_nested();
    total += partially_overlapping();
    total += complex_nesting();
    total += early_exit_loops();
    
    /* Use global volatile to prevent optimization */
    global_counter = total;
    
    /* Print to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
