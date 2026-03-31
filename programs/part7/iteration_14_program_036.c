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
#define PRESSURE_OP(x) \
    do { \
        int a = (x); \
        int b = (x) * 2; \
        int c = b - a; \
        asm volatile ("" : : "r"(a), "r"(b), "r"(c)); \
    } while(0)

/* 
 * Function 1: Perfect nesting - other is subset of loop
 * Should trigger: !bitmap_intersect_compl_p(other->block_bitmap, loop->block_bitmap)
 * Result: loop->loops.safe_push(other)
 */
NOINLINE int perfect_nesting(int N) {
    int result = 0;
    
    /* Outer loop - this is 'loop' in the coverage context */
    for (int i = 0; i < N; ++i) {
        /* No code here to ensure other is subset */
        
        /* Inner loop - this is 'other' in the coverage context */
        for (int j = 0; j < N/2; ++j) {
            PRESSURE_OP(i + j);
            result ^= (i * j) & 0xFF;
        }
        
        /* No code here either to maintain subset relationship */
    }
    
    return result;
}

/* 
 * Function 2: Reverse nesting - loop is subset of other
 * Should trigger: !bitmap_intersect_compl_p(loop->block_bitmap, other->block_bitmap)
 * Result: other->loops.safe_push(loop)
 */
NOINLINE int reverse_nesting(int N) {
    int result = 0;
    
    /* Outer loop - this is 'other' in the coverage context */
    for (int i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 3; ++j) {
            PRESSURE_OP(i + j);
            result += j;
        }
        
        /* Second inner loop - this is 'loop' in the coverage context */
        for (int k = 0; k < N/3; ++k) {
            PRESSURE_OP(i + k);
            result ^= (i * k) & 0xFF;
        }
    }
    
    return result;
}

/* 
 * Function 3: Partial overlap with goto - blocks intersect but neither is subset
 * Should trigger: bitmap_intersect_p passes, both intersect_compl_p fail
 * Result: neither push occurs
 */
NOINLINE int partial_overlap(int N) {
    int result = 0;
    
    /* Loop A - could be 'loop' or 'other' depending on discovery order */
    for (int i = 0; i < N; ++i) {
        PRESSURE_OP(i);
        
        if (i % 3 == 0) {
            /* Jump into Loop B's body */
            goto inside_loop_b;
        }
        
        result += i;
        continue;
        
inside_loop_b:
        /* Loop B - shares this block with Loop A via goto */
        for (int j = 0; j < N/4; ++j) {
            PRESSURE_OP(i + j);
            result ^= j;
            
            if (j % 2 == 0) {
                /* Break back to Loop A */
                break;
            }
        }
        
        /* More code in Loop A after the break */
        result -= i;
    }
    
    return result;
}

/* 
 * Function 4: Mixed loop types with complex control flow
 * Creates varied CFG structures for the loop analyzer
 */
NOINLINE int mixed_loops(int N) {
    int result = 0;
    int i = 0;
    
    /* while loop */
    while (i < N) {
        PRESSURE_OP(i);
        
        /* do-while nested inside while */
        int j = 0;
        do {
            PRESSURE_OP(i + j);
            result += (i * j) & 0xFF;
            j++;
        } while (j < 5);
        
        /* Another for loop in sequence */
        for (int k = 0; k < 3; ++k) {
            PRESSURE_OP(i + k);
            result ^= k;
        }
        
        i++;
    }
    
    /* Follow with a for loop containing goto */
    for (int a = 0; a < N/2; ++a) {
        PRESSURE_OP(a);
        
        if (a % 4 == 0) {
            /* Create overlapping block with next loop */
            result += a;
            goto overlap_point;
        }
        
        result -= a;
        continue;
        
overlap_point:
        /* This block is shared */
        for (int b = 0; b < 2; ++b) {
            PRESSURE_OP(a + b);
            result |= b;
        }
    }
    
    return result;
}

/* 
 * Function 5: Sibling loops with shared header
 * Two adjacent loops that share some setup code
 */
NOINLINE int sibling_loops(int N) {
    int result = 0;
    int shared = N * 2;
    
    /* Shared setup - could be considered part of both loops' bitmaps */
    asm volatile ("" : : "r"(shared));
    
    /* First sibling loop */
    for (int i = 0; i < N; ++i) {
        PRESSURE_OP(i);
        result += (i + shared) & 0xFF;
    }
    
    /* Intermediate code - breaks perfect nesting */
    shared = result;
    
    /* Second sibling loop */
    for (int j = 0; j < N/2; ++j) {
        PRESSURE_OP(j);
        result ^= (j * shared) & 0xFF;
    }
    
    return result;
}

/* Main driver with volatile inputs to prevent constant propagation */
int main(int argc, char *argv[]) {
    volatile int seed = argc;
    int N = (seed % 100) + 10;  /* Ensure loops run */
    
    int total = 0;
    
    /* Call all functions to ensure they're compiled and executed */
    total += perfect_nesting(N);
    total += reverse_nesting(N);
    total += partial_overlap(N);
    total += mixed_loops(N);
    total += sibling_loops(N);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
