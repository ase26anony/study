/* 
 * Test program for hardware loop optimization coverage.
 * Designed to trigger bitmap intersection logic in hw-doloop.cc lines 429-436.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * Target: ARMv8-A with hardware loop support
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function compilation */
#define NOINLINE __attribute__((noinline, cold))

/* Create register pressure */
#define PRESSURE(x) do { \
    int a = (x), b = (x)*2, c = b - a; \
    asm volatile ("" : : "r"(a), "r"(b), "r"(c)); \
} while(0)

/* Function 1: Perfect nesting - other is subset of loop (Condition 2) */
NOINLINE int perfect_nesting(int N) {
    int result = 0;
    volatile int seed = N; /* Prevent constant propagation */
    
    /* Outer loop - this will be 'loop' */
    for (int i = 0; i < (seed % 50) + 10; ++i) {
        /* No code here ensures loop's blocks are exactly outer header + inner loop */
        
        /* Inner loop - this will be 'other' (subset of loop) */
        for (int j = 0; j < (seed % 20) + 5; ++j) {
            result ^= i * j;
            PRESSURE(j);
        }
        
        /* No code here either - ensures perfect nesting */
    }
    
    return result & 0xFF;
}

/* Function 2: Reverse nesting - loop is subset of other (Condition 3) */
NOINLINE int reverse_nesting(int N) {
    int result = 0;
    volatile int seed = N;
    
    /* Outer loop - this will be 'other' */
    for (int i = 0; i < (seed % 40) + 15; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < (seed % 10) + 3; ++j) {
            result += i - j;
            PRESSURE(j);
        }
        
        /* Some intermediate code in outer loop */
        int temp = i * 2;
        asm volatile ("" : : "r"(temp));
        
        /* Second inner loop - this will be 'loop' (subset of other) */
        for (int k = 0; k < (seed % 15) + 4; ++k) {
            result ^= temp * k;
            PRESSURE(k);
        }
    }
    
    return result & 0xFF;
}

/* Function 3: Partial overlap with goto (Condition 1 - intersection) */
NOINLINE int partial_overlap(int N) {
    int result = 0;
    volatile int seed = N;
    
    /* Loop A - will be 'loop' */
    for (int i = 0; i < (seed % 30) + 8; ++i) {
    loop_a_body:
        result += i * 3;
        PRESSURE(i);
        
        /* Loop B - will be 'other' */
        for (int j = 0; j < (seed % 12) + 3; ++j) {
            result ^= j;
            
            /* Jump into loop A's body, creating intersection */
            if (j == (seed % 5) + 1) {
                goto loop_a_body;
            }
            
            PRESSURE(j);
        }
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types with sibling relationship */
NOINLINE int mixed_loops(int N) {
    int result = 0;
    volatile int seed = N;
    
    /* do-while inside for */
    for (int i = 0; i < (seed % 25) + 7; ++i) {
        int counter = 0;
        
        /* do-while loop */
        do {
            result += i * counter;
            PRESSURE(counter);
            counter++;
        } while (counter < (seed % 8) + 2);
        
        /* while loop that follows */
        int k = 0;
        while (k < (seed % 6) + 2) {
            result ^= (i << k);
            PRESSURE(k);
            k++;
        }
    }
    
    return result & 0xFF;
}

/* Function 5: Complex nested structure with multiple exits */
NOINLINE int complex_nesting(int N) {
    int result = 0;
    volatile int seed = N;
    
    /* Triple nesting */
    for (int i = 0; i < (seed % 20) + 5; ++i) {
        for (int j = 0; j < (seed % 15) + 4; ++j) {
            /* Innermost loop with early exit */
            for (int k = 0; k < (seed % 10) + 3; ++k) {
                result += i * j * k;
                PRESSURE(k);
                
                if ((i + j + k) % 7 == 0) {
                    /* Break to middle loop */
                    goto middle_loop_continue;
                }
            }
            
        middle_loop_continue:
            result ^= j;
        }
        
        /* Additional computation in outer loop */
        result += i << 2;
    }
    
    return result & 0xFF;
}

/* Main function with volatile inputs to prevent optimization */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile and command line to prevent constant propagation */
    volatile int base = argc;
    int N1 = (base % 100) + 20;
    int N2 = (base % 80) + 25;
    int N3 = (base % 60) + 30;
    int N4 = (base % 40) + 35;
    int N5 = (base % 20) + 40;
    
    /* Call all functions to ensure they're compiled and executed */
    total ^= perfect_nesting(N1);
    total ^= reverse_nesting(N2);
    total ^= partial_overlap(N3);
    total ^= mixed_loops(N4);
    total ^= complex_nesting(N5);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
