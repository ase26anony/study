/* 
 * Test program for hardware loop optimization coverage.
 * Designed to trigger bitmap intersection logic in hw-doloop.cc lines 429-436.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * or: gcc -O3 -funroll-loops -fpeel-loops -fprofile-arcs -ftest-coverage -march=native
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
NOINLINE int test_perfect_nesting(int N) {
    int result = 0;
    
    /* Outer loop - this will be 'loop' in the analysis */
    for (int i = 0; i < N; ++i) {
        /* No code here ensures loop has no blocks outside other */
        
        /* Inner loop - this will be 'other' in the analysis */
        for (int j = 0; j < N/2; ++j) {
            result ^= i * j;
            PRESSURE(j);
        }
        
        /* No code here either - perfect nesting */
    }
    
    return result & 0xFF;
}

/* Function 2: Reverse subset - loop is subset of other (Condition 3) */
NOINLINE int test_reverse_subset(int N) {
    int result = 0;
    
    /* Outer loop - this will be 'other' in the analysis */
    for (int i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 3; ++j) {
            result += j;
            PRESSURE(j);
        }
        
        /* Second inner loop - this will be 'loop' in the analysis */
        for (int k = 0; k < N/3; ++k) {
            result ^= i * k;
            PRESSURE(k);
        }
    }
    
    return result & 0xFF;
}

/* Function 3: Partial overlap with goto (Condition 1) */
NOINLINE int test_partial_overlap(int N) {
    int result = 0;
    
    /* Loop A - will be 'loop' */
    for (int i = 0; i < N; ++i) {
        result += i;
        PRESSURE(i);
        
    loop_body:
        if (i % 2 == 0) {
            /* Loop B - will be 'other' */
            for (int j = 0; j < 5; ++j) {
                result ^= j;
                PRESSURE(j);
                
                /* Jump into loop A's body to create intersection */
                if (j == 3 && i < N/2)
                    goto loop_body;
            }
        }
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types with complex nesting */
NOINLINE int test_mixed_loops(int N) {
    int result = 0;
    int i = 0;
    
    /* while loop as outer */
    while (i < N) {
        /* do-while as middle */
        int j = 0;
        do {
            /* for loop as inner - potential 'other' subset */
            for (int k = 0; k < 4; ++k) {
                result += i * j * k;
                PRESSURE(k);
            }
            j++;
        } while (j < 3);
        
        /* Another for loop in same outer - creates sibling relationship */
        for (int m = 0; m < 2; ++m) {
            result ^= m;
            PRESSURE(m);
        }
        
        i++;
    }
    
    return result & 0xFF;
}

/* Function 5: Adjacent loops with shared basic block via switch */
NOINLINE int test_adjacent_loops(int N) {
    int result = 0;
    
    /* First loop */
    for (int i = 0; i < N; ++i) {
        switch (i % 3) {
            case 0:
                result += i;
                break;
            case 1:
                result -= i;
                break;
            default:
                /* Shared block that both loops can reach */
                result ^= i;
                goto second_loop_entry;
        }
        PRESSURE(i);
    }
    
second_loop_entry:
    /* Second loop that shares the default case block */
    for (int j = 0; j < N/2; ++j) {
        result += j * 2;
        PRESSURE(j);
    }
    
    return result & 0xFF;
}

/* Main driver with volatile inputs to prevent optimization */
int main(int argc, char *argv[]) {
    volatile int seed = argc;
    int N1 = (seed % 50) + 20;
    int N2 = (seed % 40) + 30;
    int N3 = (seed % 30) + 10;
    int N4 = (seed % 60) + 5;
    int N5 = (seed % 25) + 15;
    
    int total = 0;
    
    /* Call all test functions to ensure they're compiled and executed */
    total += test_perfect_nesting(N1);
    total += test_reverse_subset(N2);
    total += test_partial_overlap(N3);
    total += test_mixed_loops(N4);
    total += test_adjacent_loops(N5);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
