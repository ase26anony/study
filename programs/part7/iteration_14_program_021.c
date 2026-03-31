/* 
 * Test program for hardware loop optimization coverage.
 * Designed to trigger bitmap intersection logic in hw-doloop.cc lines 429-436.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * For generic targets: gcc -O3 -funroll-loops -fpeel-loops -fprofile-arcs -ftest-coverage
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline, cold))

/* Create register pressure */
#define PRESSURE(x) \
    do { \
        int a = (x), b = (x)*2, c = b - a; \
        asm volatile("" : : "r"(a), "r"(b), "r"(c)); \
    } while(0)

/* Function 1: Perfect nesting - other is subset of loop (Condition 2) */
NOINLINE int perfect_nesting(int N) {
    int result = 0;
    
    /* Outer loop - this will be 'loop' in the analysis */
    for (int i = 0; i < N; ++i) {
        /* No code here ensures loop's blocks are exactly outer header + inner loop */
        
        /* Inner loop - this will be 'other' in the analysis */
        for (int j = 0; j < N/2; ++j) {
            /* Create register pressure */
            int a = i + j, b = i * j, c = b - a;
            result ^= (a * b) >> (c & 7);
            PRESSURE(result);
        }
        
        /* No code here either - ensures perfect nesting */
    }
    
    return result & 0xFF;
}

/* Function 2: Reverse nesting - loop is subset of other (Condition 3) */
NOINLINE int reverse_nesting(int N) {
    int result = 0;
    
    /* Outer loop - this will be 'other' in the analysis */
    for (int i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 3; ++j) {
            result += i * j;
            PRESSURE(result);
        }
        
        /* Second inner loop - this will be 'loop' in the analysis */
        /* loop's blocks are subset of other's blocks */
        for (int k = 0; k < N/3; ++k) {
            int a = i + k, b = i * k;
            result ^= (a * b) >> (k & 3);
            PRESSURE(result);
        }
    }
    
    return result & 0xFF;
}

/* Function 3: Partial overlap with goto - triggers first condition */
NOINLINE int partial_overlap(int N) {
    int result = 0;
    int i = 0, j = 0;
    
    /* Loop A - will be 'loop' */
    for (i = 0; i < N; ++i) {
        result += i * 2;
        PRESSURE(result);
        
    loop_body:
        /* This label creates shared basic block */
        result ^= i;
        
        /* Loop B - will be 'other' */
        for (j = 0; j < 5; ++j) {
            if (j == 3 && i < N/2) {
                /* Jump into loop A's body - creates intersection */
                goto loop_body;
            }
            result += j * i;
            PRESSURE(result);
        }
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types with sibling loops */
NOINLINE int mixed_loops(int N) {
    int result = 0;
    int i = 0;
    
    /* do-while inside for */
    for (i = 0; i < N; ++i) {
        int j = 0;
        do {
            result += i * j;
            PRESSURE(result);
            j++;
        } while (j < 4);
        
        /* while loop after do-while */
        int k = 0;
        while (k < 3) {
            result ^= (i + k);
            PRESSURE(result);
            k++;
        }
    }
    
    /* Additional for loop that shares no blocks (disjoint case) */
    for (int m = 0; m < 10; ++m) {
        result -= m;
    }
    
    return result & 0xFF;
}

/* Function 5: Complex nested structure with multiple levels */
NOINLINE int complex_nesting(int N) {
    int result = 0;
    
    /* Level 1 */
    for (int i = 0; i < N; ++i) {
        /* Level 2 - first inner */
        for (int j = 0; j < i+1; ++j) {
            /* Level 3 */
            int k = 0;
            while (k < 5) {
                result += i * j * k;
                PRESSURE(result);
                k++;
                
                /* Small do-while inside while */
                int m = 0;
                do {
                    result ^= m;
                    m++;
                } while (m < 2);
            }
        }
        
        /* Level 2 - second inner (sibling) */
        for (int p = 0; p < 3; ++p) {
            result -= i * p;
            PRESSURE(result);
        }
    }
    
    return result & 0xFF;
}

int main(int argc, char *argv[]) {
    /* Use volatile to prevent constant propagation */
    volatile int seed = argc;
    int N1 = (seed % 50) + 20;
    int N2 = (seed % 40) + 15;
    int N3 = (seed % 30) + 10;
    int N4 = (seed % 25) + 5;
    int N5 = (seed % 20) + 3;
    
    int total = 0;
    
    /* Call all functions to ensure they're compiled and executed */
    total += perfect_nesting(N1);
    total += reverse_nesting(N2);
    total += partial_overlap(N3);
    total += mixed_loops(N4);
    total += complex_nesting(N5);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
