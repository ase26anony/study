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
#define PRESSURE(x) \
    do { \
        int a = (x), b = (x)*2, c = b - a; \
        asm volatile("" : : "r"(a), "r"(b), "r"(c)); \
    } while(0)

/* Function 1: Perfect nesting - other is subset of loop */
NOINLINE int test_perfect_nesting(int N) {
    int result = 0;
    
    /* Outer loop (will be 'loop' in hierarchy) */
    for (int i = 0; i < N; ++i) {
        /* No code here to ensure inner loop blocks are subset */
        
        /* Inner loop (will be 'other' in hierarchy) */
        for (int j = 0; j < N/2; ++j) {
            PRESSURE(j);
            result ^= (i * j) & 0xFF;
        }
        
        /* No code here either to maintain subset relationship */
    }
    
    return result;
}

/* Function 2: Loop is subset of other */
NOINLINE int test_loop_subset_of_other(int N) {
    int result = 0;
    
    /* Outer loop (will be 'other' in hierarchy) */
    for (int i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 3; ++j) {
            PRESSURE(j);
            result += j * 7;
        }
        
        /* Second inner loop (will be 'loop' in hierarchy) */
        for (int k = 0; k < N/3; ++k) {
            PRESSURE(k);
            result ^= (i * k) & 0xFF;
        }
    }
    
    return result;
}

/* Function 3: Partially overlapping loops with goto */
NOINLINE int test_partial_overlap(int N) {
    int result = 0;
    
    /* Loop A */
    for (int i = 0; i < N; ++i) {
        PRESSURE(i);
        
        /* Loop B that shares blocks via goto */
        for (int j = 0; j < N/2; ++j) {
            PRESSURE(j);
            
            if (j == N/4) {
                /* Jump into Loop A's body, creating intersection */
                goto shared_block;
            }
            
            result += j * 3;
        }
        
        /* This label creates shared basic block */
        shared_block:
        result ^= i * 7;
    }
    
    return result;
}

/* Function 4: Mixed loop types for varied CFG */
NOINLINE int test_mixed_loops(int N) {
    int result = 0;
    int i = 0;
    
    /* while loop */
    while (i < N/2) {
        PRESSURE(i);
        
        /* do-while nested inside while */
        int j = 0;
        do {
            PRESSURE(j);
            result += (i * j) & 0xFF;
            j++;
        } while (j < 5);
        
        i++;
    }
    
    /* for loop after while */
    for (int k = 0; k < N; ++k) {
        PRESSURE(k);
        
        /* Another nested for */
        for (int m = 0; m < 2; ++m) {
            PRESSURE(m);
            result ^= (k * m) & 0xFF;
        }
    }
    
    return result;
}

/* Function 5: Complex nested structure with sibling loops */
NOINLINE int test_sibling_loops(int N) {
    int result = 0;
    
    /* Outer loop */
    for (int outer = 0; outer < N/4; ++outer) {
        PRESSURE(outer);
        
        /* First sibling inner loop */
        for (int inner1 = 0; inner1 < 3; ++inner1) {
            PRESSURE(inner1);
            result += inner1 * 11;
        }
        
        /* Code between siblings - ensures blocks are not subsets */
        int temp = outer * 3;
        asm volatile("" : : "r"(temp));
        
        /* Second sibling inner loop */
        for (int inner2 = 0; inner2 < N/8; ++inner2) {
            PRESSURE(inner2);
            result ^= (outer * inner2) & 0xFF;
        }
    }
    
    return result;
}

/* Main driver with volatile inputs to prevent constant propagation */
int main(int argc, char *argv[]) {
    /* Use volatile to prevent optimization */
    volatile int seed = argc;
    int N = (seed % 100) + 20;  /* Ensure N >= 20 for all loops */
    
    int total = 0;
    
    /* Call all test functions */
    total += test_perfect_nesting(N);
    total += test_loop_subset_of_other(N);
    total += test_partial_overlap(N);
    total += test_mixed_loops(N);
    total += test_sibling_loops(N);
    
    /* Generate side effect to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return total & 1;
}
