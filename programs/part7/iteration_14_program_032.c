/* 
 * Test program for hardware loop optimization coverage.
 * Designed to trigger bitmap intersection logic in hw-doloop.cc lines 429-436.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * Target: ARMv8-A with hardware loop support (or similar architecture)
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization and inlining */
#define KEEP(V) asm volatile("" : : "r"(V))
#define NOINLINE __attribute__((noinline, cold))

/* Function 1: Perfect nesting - other is subset of loop (Condition 2) */
NOINLINE int perfect_nesting(int N) {
    int result = 0;
    volatile int seed = N;
    int limit = (seed % 50) + 10;
    
    /* Outer loop - this will be 'loop' */
    for (int i = 0; i < limit; ++i) {
        /* No code here to ensure inner loop blocks are subset */
        
        /* Inner loop - this will be 'other' (subset of outer) */
        for (int j = 0; j < 5; ++j) {
            int a = i + j;
            int b = a * 2;
            int c = b - a;
            result ^= (a * b) >> (c & 3);
            KEEP(result);
        }
        
        /* No code here either to maintain subset relationship */
    }
    
    return result;
}

/* Function 2: Reverse nesting - loop is subset of other (Condition 3) */
NOINLINE int reverse_nesting(int N) {
    int result = 0;
    volatile int seed = N;
    int limit = (seed % 40) + 15;
    
    /* Outer loop - this will be 'other' */
    for (int i = 0; i < limit; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 3; ++j) {
            int a = i * j;
            result += a & 0xFF;
            KEEP(result);
        }
        
        /* Second inner loop - this will be 'loop' (subset of outer) */
        for (int k = 0; k < 7; ++k) {
            int b = i + k;
            int c = b * 3;
            result ^= (b + c) & 0xFF;
            KEEP(result);
        }
    }
    
    return result;
}

/* Function 3: Partially overlapping loops with goto (Condition 1) */
NOINLINE int overlapping_loops(int N) {
    int result = 0;
    volatile int seed = N;
    int limit = (seed % 30) + 20;
    
    /* Loop A - will be 'loop' */
    for (int i = 0; i < limit; ++i) {
        int a = i * 2;
        
    shared_block:
        result += a & 0xF;
        KEEP(result);
        
        /* Loop B - will be 'other', shares block via goto */
        int j = 0;
        while (j < 5) {
            int b = i + j;
            if (b % 3 == 0) {
                goto shared_block;  /* Jump into loop A's body */
            }
            result ^= b;
            KEEP(result);
            j++;
        }
        
        /* Do-while loop for variety */
        int k = 0;
        do {
            result += k * i;
            KEEP(result);
            k++;
        } while (k < 3);
    }
    
    return result;
}

/* Function 4: Complex nested structure with mixed loop types */
NOINLINE int complex_nesting(int N) {
    int result = 0;
    volatile int seed = N;
    int limit = (seed % 25) + 25;
    
    /* Outer while loop */
    int outer = 0;
    while (outer < limit) {
        /* First for loop inside while */
        for (int i = 0; i < 4; ++i) {
            int a = outer + i;
            
            /* Inner do-while */
            int dw = 0;
            do {
                result += (a * dw) & 0xFF;
                KEEP(result);
                dw++;
            } while (dw < 2);
            
            /* Another for loop in sequence */
            for (int j = 0; j < 3; ++j) {
                int b = a + j;
                result ^= (b << 2) & 0xFF;
                KEEP(result);
            }
        }
        
        outer++;
    }
    
    return result;
}

/* Function 5: Sibling loops with partial overlap via shared condition */
NOINLINE int sibling_loops(int N) {
    int result = 0;
    volatile int seed = N;
    int limit = (seed % 35) + 10;
    
    int shared_var = 0;
    
    /* Loop X */
    for (int x = 0; x < limit; ++x) {
        shared_var = x * 2;
        
        /* Conditional block that might be shared */
        if (shared_var % 4 == 0) {
            result += shared_var;
            KEEP(result);
        }
    }
    
    /* Loop Y - shares the conditional block structure */
    for (int y = 0; y < limit; ++y) {
        shared_var = y * 3;
        
        /* Same conditional structure - creates bitmap intersection */
        if (shared_var % 4 == 0) {
            result += shared_var;
            KEEP(result);
        }
        
        /* Extra code in this loop to make blocks not identical */
        result ^= y;
        KEEP(result);
    }
    
    return result;
}

/* Main driver that calls all functions */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile and command line to prevent constant propagation */
    volatile int base = argc;
    int N = (base % 100) + 20;
    
    /* Call all functions to ensure they're compiled and executed */
    total ^= perfect_nesting(N);
    total ^= reverse_nesting(N + 1);
    total ^= overlapping_loops(N + 2);
    total ^= complex_nesting(N + 3);
    total ^= sibling_loops(N + 4);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return total & 0xFF;
}
