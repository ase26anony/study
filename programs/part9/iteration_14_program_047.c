/* 
 * Test program for hw-doloop.cc coverage
 * Designed to trigger bitmap intersection logic in discover_loop_hierarchy
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function analysis */
#define NOINLINE __attribute__((noinline, cold))

/* Use volatile to prevent constant propagation */
volatile int global_seed = 42;

/* Function 1: Perfect nesting - other is subset of loop */
NOINLINE int perfect_nesting(int n) {
    int result = 0;
    int i, j;
    
    /* Outer loop (will be 'loop' in hierarchy) */
    for (i = 0; i < n; ++i) {
        /* No code here to ensure other is subset */
        
        /* Inner loop (will be 'other' in hierarchy) */
        for (j = 0; j < i; ++j) {
            /* Create register pressure */
            int a = i * j;
            int b = a ^ j;
            int c = b - i;
            int d = c * a;
            int e = d >> 2;
            result ^= e;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(result));
        }
        
        /* No code here either to maintain subset relationship */
    }
    
    return result & 0xFF;
}

/* Function 2: Reverse nesting - loop is subset of other */
NOINLINE int reverse_nesting(int n) {
    int result = 0;
    int i, j, k;
    
    /* Outer loop (will be 'other' in hierarchy) */
    for (i = 0; i < n; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (j = 0; j < 3; ++j) {
            result += i * j;
            asm volatile("" : : "r"(result));
        }
        
        /* Second inner loop (will be 'loop' in hierarchy) */
        for (k = 0; k < i; ++k) {
            /* Register pressure */
            int a = i * k;
            int b = a ^ k;
            int c = b - i;
            result ^= c;
            asm volatile("" : : "r"(result));
        }
        
        /* More code in 'other' after 'loop' */
        result += i;
    }
    
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops with goto */
NOINLINE int overlapping_loops(int n) {
    int result = 0;
    int i, j;
    
    /* Loop A */
    for (i = 0; i < n; ++i) {
        result += i * 2;
        
    loop_b_start:
        /* Loop B - shares block via goto */
        for (j = 0; j < 5; ++j) {
            if (result > 1000 && i % 2 == 0) {
                /* Jump into Loop A's body */
                goto shared_block;
            }
            result ^= j;
            asm volatile("" : : "r"(result));
        }
        
        if (i % 3 == 0) {
            goto loop_b_start;  /* Create backedge to Loop B */
        }
        
    shared_block:
        result += 7;
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types with complex relationships */
NOINLINE int mixed_loops(int n) {
    int result = 0;
    int i = 0;
    
    /* while loop */
    while (i < n) {
        int j = 0;
        
        /* do-while loop inside while */
        do {
            /* Create blocks that will be shared */
            int a = i * j;
            int b = a ^ result;
            result += b;
            asm volatile("" : : "r"(result));
            j++;
        } while (j < 3);
        
        /* Another for loop in same parent */
        for (int k = 0; k < i; k++) {
            result ^= k;
            asm volatile("" : : "r"(result));
        }
        
        i++;
    }
    
    /* Follow-up for loop that might intersect */
    for (int m = 0; m < n/2; m++) {
        result += m * 3;
        asm volatile("" : : "r"(result));
    }
    
    return result & 0xFF;
}

/* Function 5: Sibling loops with partial overlap via break */
NOINLINE int sibling_loops(int n) {
    int result = 0;
    int i, j;
    
    /* First loop */
    for (i = 0; i < n; ++i) {
        if (i % 2 == 0) {
            /* This block might be shared with second loop via break */
            result += i * 3;
            asm volatile("" : : "r"(result));
            
            /* Break to shared label */
            if (result > 500) goto shared_sibling_block;
        }
    }
    
    /* Second loop - partially overlaps */
    for (j = 0; j < n * 2; ++j) {
        result ^= j;
        
    shared_sibling_block:
        /* Shared block between both loops */
        result += 1;
        asm volatile("" : : "r"(result));
        
        if (j % 4 == 0) break;
    }
    
    return result & 0xFF;
}

/* Main function to drive execution */
int main(int argc, char **argv) {
    int total = 0;
    
    /* Use volatile to prevent constant propagation */
    volatile int base = argc;
    int n1 = (base % 50) + 10;
    int n2 = (base % 40) + 15;
    int n3 = (base % 30) + 20;
    int n4 = (base % 20) + 25;
    int n5 = (base % 10) + 30;
    
    /* Call all functions to ensure they're compiled and executed */
    total ^= perfect_nesting(n1);
    total ^= reverse_nesting(n2);
    total ^= overlapping_loops(n3);
    total ^= mixed_loops(n4);
    total ^= sibling_loops(n5);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
