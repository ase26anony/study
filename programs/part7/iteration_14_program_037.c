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
        
        /* Second inner loop - this will be 'loop' (subset of other) */
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
    int counter = 0;
    
    /* Loop A - will be 'loop' */
    for (int i = 0; i < N; ++i) {
        result += i;
        PRESSURE(i);
        
    loop_body:
        /* This label creates shared basic block */
        if (counter++ > 100) break;
        
        /* Loop B - will be 'other' with partial overlap */
        for (int j = 0; j < 3; ++j) {
            result ^= j;
            PRESSURE(j);
            
            if (result % 7 == 0) {
                /* Jump into loop A's body, creating intersection */
                goto loop_body;
            }
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
            result += i * j;
            PRESSURE(j);
            j++;
        } while (j < 3);
        
        /* for loop after do-while */
        for (int k = 0; k < 2; ++k) {
            result ^= k;
            PRESSURE(k);
        }
        
        i++;
    }
    
    return result;
}

/* Function 5: Complex sibling loops with break to shared block */
NOINLINE int sibling_loops_break(int N) {
    int result = 0;
    
    /* First loop */
    for (int i = 0; i < N; ++i) {
        if (i % 2 == 0) {
            /* Inner loop that can break to shared code */
            for (int j = 0; j < 5; ++j) {
                result += j;
                PRESSURE(j);
                if (result % 11 == 0) break;
            }
        }
        
    shared_block:
        /* Shared block between loops */
        result ^= 0x55;
        
        /* Second loop that can goto shared block */
        for (int k = 0; k < 3; ++k) {
            result += k * 2;
            PRESSURE(k);
            if (k == 1) goto shared_block;
        }
    }
    
    return result;
}

/* Function 6: Adjacent loops with no intersection */
NOINLINE int adjacent_disjoint(int N) {
    int result = 0;
    
    /* First loop - completely disjoint from second */
    for (int i = 0; i < N; ++i) {
        result += i * 3;
        PRESSURE(i);
    }
    
    /* Some intermediate code to ensure separation */
    result ^= 0xFF;
    
    /* Second loop - no blocks in common with first */
    for (int j = 0; j < N/2; ++j) {
        result -= j * 2;
        PRESSURE(j);
    }
    
    return result;
}

/* Main driver with volatile inputs to prevent constant propagation */
int main(int argc, char *argv[]) {
    volatile int seed = argc;
    int total = 0;
    
    /* Use volatile to prevent optimization */
    int N1 = (seed % 50) + 10;
    int N2 = (seed % 40) + 15;
    int N3 = (seed % 30) + 20;
    int N4 = (seed % 60) + 5;
    int N5 = (seed % 70) + 8;
    int N6 = (seed % 80) + 12;
    
    /* Call all functions to ensure they're compiled and executed */
    total ^= perfect_nesting(N1);
    total ^= reverse_nesting(N2);
    total ^= partial_overlap_goto(N3);
    total ^= mixed_loop_types(N4);
    total ^= sibling_loops_break(N5);
    total ^= adjacent_disjoint(N6);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
