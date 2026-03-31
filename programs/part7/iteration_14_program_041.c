/* 
 * Test program for hardware loop optimization coverage.
 * Designed to trigger bitmap intersection logic in hw-doloop.cc lines 429-436.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * Target: ARMv8-A with hardware loop support
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline, cold))

/* Create register pressure */
#define PRESSURE(x) \
    do { \
        int _a = (x); \
        int _b = _a * 2; \
        int _c = _b - _a; \
        int _d = _a ^ _b; \
        int _e = _c | _d; \
        asm volatile("" : : "r"(_a), "r"(_b), "r"(_c), "r"(_d), "r"(_e)); \
    } while(0)

/* Function 1: Perfect nesting - other is subset of loop (Condition 2) */
NOINLINE int perfect_nesting(int N) {
    int result = 0;
    
    /* Outer loop - this will be 'loop' */
    for (int i = 0; i < N; ++i) {
        /* No code here to ensure other is subset */
        
        /* Inner loop - this will be 'other' */
        for (int j = 0; j < 5; ++j) {
            result ^= i * j;
            PRESSURE(j);
        }
        
        /* No code here either to maintain subset relationship */
    }
    
    return result;
}

/* Function 2: Reverse nesting - loop is subset of other (Condition 3) */
NOINLINE int reverse_nesting(int N) {
    int result = 0;
    
    /* Outer loop - this will be 'other' */
    for (int i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 3; ++j) {
            result += j * 2;
            PRESSURE(j);
        }
        
        /* Second inner loop - this will be 'loop' (subset of 'other') */
        for (int k = 0; k < 4; ++k) {
            result ^= i * k;
            PRESSURE(k);
        }
    }
    
    return result;
}

/* Function 3: Partial overlap with goto - triggers first condition */
NOINLINE int partial_overlap_goto(int N) {
    int result = 0;
    
    /* Loop A - will be 'loop' */
    for (int i = 0; i < N; ++i) {
        result += i;
        PRESSURE(i);
        
    loop_body:
        /* Shared block label */
        if (i % 2 == 0) {
            result *= 2;
        }
    }
    
    /* Loop B - will be 'other' with goto into loop A */
    for (int j = 0; j < N/2; ++j) {
        result -= j;
        PRESSURE(j);
        
        if (j == N/4) {
            /* Jump into loop A's body, creating intersection */
            goto loop_body;
        }
    }
    
    return result;
}

/* Function 4: Mixed loop types for varied CFG */
NOINLINE int mixed_loop_types(int N) {
    int result = 0;
    int i = 0;
    
    /* while loop */
    while (i < N) {
        /* do-while nested inside while */
        int j = 0;
        do {
            result ^= (i * j) & 0xFF;
            PRESSURE(j);
            j++;
        } while (j < 5);
        
        i++;
        
        /* for loop after do-while */
        for (int k = 0; k < 3; k++) {
            result += k;
            PRESSURE(k);
        }
    }
    
    return result;
}

/* Function 5: Complex sibling loops with break/continue */
NOINLINE int complex_siblings(int N) {
    int result = 0;
    
    /* Two sibling loops that might be discovered in different orders */
    for (int i = 0; i < N; i++) {
        if (i % 3 == 0) {
            /* This creates a basic block in the first loop */
            result += i * 2;
        }
        PRESSURE(i);
    }
    
    /* Second loop shares some control flow patterns */
    for (int j = N-1; j >= 0; j--) {
        result ^= j;
        PRESSURE(j);
        
        /* Nested loop inside second loop */
        for (int k = 0; k < 2; k++) {
            result += k;
        }
    }
    
    return result;
}

/* Main function with volatile inputs to prevent optimization */
int main(int argc, char *argv[]) {
    volatile int seed = argc;
    int N1 = (seed % 50) + 10;
    int N2 = (seed % 40) + 15;
    int N3 = (seed % 30) + 20;
    int N4 = (seed % 25) + 25;
    int N5 = (seed % 20) + 30;
    
    int total = 0;
    
    /* Call all functions to ensure they're compiled and executed */
    total += perfect_nesting(N1);
    total += reverse_nesting(N2);
    total += partial_overlap_goto(N3);
    total += mixed_loop_types(N4);
    total += complex_siblings(N5);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
