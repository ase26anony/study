/* 
 * Test program for hardware loop optimization coverage.
 * Designed to trigger bitmap intersection logic in hw-doloop.cc lines 429-436.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * or: gcc -O3 -funroll-loops -fpeel-loops -fprofile-arcs -ftest-coverage -march=native
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function compilation */
#define NOINLINE __attribute__((noinline, cold))

/* Create register pressure */
#define PRESSURE_OP(x) \
    do { \
        int _a = (x); \
        int _b = _a * 2; \
        int _c = _b - _a; \
        asm volatile ("" : : "r"(_a), "r"(_b), "r"(_c)); \
    } while(0)

/* Function 1: Perfect nesting - other is subset of loop (Condition 2) */
NOINLINE int perfect_nesting(int N) {
    int total = 0;
    
    /* Outer loop - this will be 'loop' */
    for (int i = 0; i < N; ++i) {
        /* No code here to ensure other is subset */
        
        /* Inner loop - this will be 'other' (subset of loop) */
        for (int j = 0; j < 5; ++j) {
            total += i * j;
            PRESSURE_OP(total);
        }
        
        /* No code here either to maintain subset relationship */
    }
    
    return total;
}

/* Function 2: Reverse nesting - loop is subset of other (Condition 3) */
NOINLINE int reverse_nesting(int N) {
    int total = 0;
    
    /* Outer loop - this will be 'other' */
    for (int i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 3; ++j) {
            total += i + j;
            PRESSURE_OP(total);
        }
        
        /* Second inner loop - this will be 'loop' (subset of other) */
        for (int k = 0; k < 4; ++k) {
            total += i * k;
            PRESSURE_OP(total);
        }
    }
    
    return total;
}

/* Function 3: Partial overlap with goto - loops intersect but neither is subset */
NOINLINE int partial_overlap(int N) {
    int total = 0;
    int i = 0;
    
    /* First loop - will be 'loop' */
    while (i < N) {
        if (total % 7 == 0) {
            /* Jump into second loop's body */
            goto overlap_point;
        }
        
        total += i;
        PRESSURE_OP(total);
        
        /* Second loop - will be 'other' */
        for (int j = 0; j < 3; ++j) {
overlap_point:
            total += i * j;
            PRESSURE_OP(total);
        }
        
        i++;
    }
    
    return total;
}

/* Function 4: Mixed loop types with complex control flow */
NOINLINE int mixed_loops(int N) {
    int total = 0;
    int i = 0;
    
    /* do-while inside for */
    for (i = 0; i < N; ++i) {
        int j = 0;
        do {
            total ^= (i << j);
            PRESSURE_OP(total);
            j++;
        } while (j < 4);
        
        /* while loop after do-while */
        int k = 0;
        while (k < 3) {
            total += k;
            PRESSURE_OP(total);
            k++;
        }
    }
    
    return total;
}

/* Function 5: Sibling loops with shared basic block via switch */
NOINLINE int sibling_loops(int N) {
    int total = 0;
    
    /* Two sequential loops that might be discovered in different orders */
    for (int i = 0; i < N; ++i) {
        switch (i % 3) {
            case 0:
                total += i;
                break;
            case 1:
                total -= i;
                break;
            default:
                total ^= i;
        }
        PRESSURE_OP(total);
    }
    
    /* Second loop shares some control flow patterns */
    for (int j = N; j > 0; --j) {
        switch (j % 3) {
            case 0:
                total += j;
                break;
            case 1:
                total -= j;
                break;
            default:
                total ^= j;
        }
        PRESSURE_OP(total);
    }
    
    return total;
}

/* Main function with volatile inputs to prevent constant propagation */
int main(int argc, char *argv[]) {
    volatile int seed = argc;
    int N1 = (seed % 100) + 10;
    int N2 = (seed % 50) + 5;
    int N3 = (seed % 30) + 3;
    int N4 = (seed % 40) + 8;
    int N5 = (seed % 60) + 12;
    
    int result = 0;
    
    /* Call all functions to ensure they're compiled and executed */
    result ^= perfect_nesting(N1) & 255;
    result ^= reverse_nesting(N2) & 255;
    result ^= partial_overlap(N3) & 255;
    result ^= mixed_loops(N4) & 255;
    result ^= sibling_loops(N5) & 255;
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result & 255);
    
    return 0;
}
