/* 
 * Test program for hardware loop optimization coverage.
 * Designed to trigger bitmap intersection logic in hw-doloop.cc.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function compilation */
#define NOINLINE __attribute__((noinline, cold))

/* Function 1: Perfectly nested loops - other is subset of loop */
NOINLINE int test_perfect_nesting(int N) {
    int result = 0;
    volatile int dummy; /* Prevent optimization */
    
    /* Outer loop (will be 'loop' in the analysis) */
    for (int i = 0; i < N; ++i) {
        /* No code here to ensure inner loop blocks are subset of outer */
        
        /* Inner loop (will be 'other' in the analysis) */
        for (int j = 0; j < N/2; ++j) {
            /* Create register pressure */
            int a = i + j;
            int b = a * 2;
            int c = b - a;
            int d = c * i;
            int e = d ^ j;
            
            /* Prevent dead code elimination */
            asm volatile ("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e));
            result ^= (a * b) >> (c & 3);
        }
        
        /* No code here either - inner loop is perfect subset */
    }
    
    dummy = result;
    return result & 0xFF;
}

/* Function 2: Loop is subset of other */
NOINLINE int test_loop_subset_of_other(int N) {
    int result = 0;
    volatile int dummy;
    
    /* Outer loop (will be 'other' in the analysis) */
    for (int i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 2; ++j) {
            int a = i * j;
            int b = a + 1;
            result += b;
            asm volatile ("" : : "r"(a), "r"(b));
        }
        
        /* Second inner loop (will be 'loop' in the analysis) */
        for (int k = 0; k < N/3; ++k) {
            /* This loop's blocks are subset of outer loop */
            int x = i + k;
            int y = x * 3;
            int z = y - x;
            result ^= z;
            asm volatile ("" : : "r"(x), "r"(y), "r"(z));
        }
    }
    
    dummy = result;
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops with goto */
NOINLINE int test_partial_overlap(int N) {
    int result = 0;
    volatile int dummy;
    
    /* Loop A (will be 'loop' in some analysis) */
    for (int i = 0; i < N; ++i) {
        int a = i * 2;
        
    shared_block:
        /* This label creates shared basic block */
        int b = a + i;
        result += b;
        
        /* Loop B (will be 'other' in some analysis) */
        for (int j = 0; j < 3; ++j) {
            int c = b * j;
            result ^= c;
            
            if (j == 1 && i < N/2) {
                /* Jump to shared block in Loop A */
                goto shared_block;
            }
            
            asm volatile ("" : : "r"(c));
        }
        
        asm volatile ("" : : "r"(a), "r"(b));
    }
    
    dummy = result;
    return result & 0xFF;
}

/* Function 4: Mixed loop types - do-while inside for */
NOINLINE int test_mixed_loops(int N) {
    int result = 0;
    volatile int dummy;
    
    /* Outer for loop */
    for (int i = 0; i < N; ++i) {
        int counter = 0;
        
        /* Inner do-while loop */
        do {
            int a = i + counter;
            int b = a * counter;
            int c = b >> 1;
            result += c;
            
            /* Create register pressure */
            int d = c ^ i;
            int e = d * 3;
            int f = e - counter;
            
            asm volatile ("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f));
            counter++;
        } while (counter < 5);
        
        /* While loop after do-while */
        int w = 0;
        while (w < 3) {
            result ^= (i * w);
            asm volatile ("" : : "r"(w));
            w++;
        }
    }
    
    dummy = result;
    return result & 0xFF;
}

/* Function 5: Sibling loops with no overlap */
NOINLINE int test_disjoint_loops(int N) {
    int result = 0;
    volatile int dummy;
    
    /* First independent loop */
    for (int i = 0; i < N/2; ++i) {
        int a = i * 3;
        int b = a + 1;
        result += b;
        asm volatile ("" : : "r"(a), "r"(b));
    }
    
    /* Second independent loop (no block sharing) */
    for (int j = 0; j < N/3; ++j) {
        int x = j * 5;
        int y = x - 2;
        result ^= y;
        asm volatile ("" : : "r"(x), "r"(y));
    }
    
    dummy = result;
    return result & 0xFF;
}

/* Main function with volatile inputs to prevent constant propagation */
int main(int argc, char *argv[]) {
    volatile int seed = argc;
    int N1 = (seed % 50) + 20;  /* Prevent trivial loops */
    int N2 = (seed % 40) + 30;
    int N3 = (seed % 30) + 25;
    int N4 = (seed % 35) + 15;
    int N5 = (seed % 45) + 10;
    
    int result = 0;
    
    /* Call all test functions */
    result ^= test_perfect_nesting(N1);
    result ^= test_loop_subset_of_other(N2);
    result ^= test_partial_overlap(N3);
    result ^= test_mixed_loops(N4);
    result ^= test_disjoint_loops(N5);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result & 0xFF);
    
    return 0;
}
