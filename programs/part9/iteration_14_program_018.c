/* test_hwloop_coverage.c
 * 
 * This test is designed for targets with hardware loop support.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * 
 * The program creates complex loop structures with specific block relationships
 * to exercise the bitmap intersection logic in hw-doloop.cc:
 * - Loops with intersecting block bitmaps
 * - Perfectly nested loops (subset relationship)
 * - Partially overlapping loops
 * - Sibling loops with one being subset of another
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function compilation */
#define NOINLINE __attribute__((noinline, cold))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Function 1: Perfectly nested loops - other is subset of loop
 * This should trigger: loop->loops.safe_push(other)
 */
NOINLINE int test_perfect_nesting(int n) {
    int result = 0;
    int i, j;
    
    /* Outer loop - this will be 'loop' in the bitmap logic */
    for (i = 0; i < n; ++i) {
        /* No code here to ensure inner loop is perfect subset */
        
        /* Inner loop - this will be 'other' in the bitmap logic */
        for (j = 0; j < i; ++j) {
            /* Create register pressure */
            int a = i + j;
            int b = i * 2 - j;
            int c = b - a;
            int d = (a * b) >> (c & 3);
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            
            result ^= d;
        }
        
        /* No code here either to maintain perfect nesting */
    }
    
    return result & 0xFF;
}

/* Function 2: loop is subset of other
 * This should trigger: other->loops.safe_push(loop)
 */
NOINLINE int test_loop_subset_of_other(int n) {
    int result = 0;
    int i, j, k;
    
    /* Outer loop - this will be 'other' in the bitmap logic */
    for (i = 0; i < n; ++i) {
        /* First inner loop - creates blocks in 'other' that are not in 'loop' */
        for (j = 0; j < 3; ++j) {
            int a = i * j;
            int b = a + global_seed;
            result += b;
            asm volatile("" : : "r"(a), "r"(b));
        }
        
        /* Second inner loop - this will be 'loop' in the bitmap logic */
        for (k = 0; k < i; ++k) {
            /* Create complex operations for register pressure */
            int x = i ^ k;
            int y = x * 3;
            int z = y - (k << 2);
            int w = z | (x & 0xF);
            
            asm volatile("" : : "r"(x), "r"(y), "r"(z), "r"(w));
            result ^= w;
        }
        
        /* More code in outer loop after 'loop' */
        result += i * 7;
    }
    
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops via goto
 * Creates loops that intersect but neither is subset
 */
NOINLINE int test_partial_overlap(int n) {
    int result = 0;
    int i = 0, j = 0;
    
    /* Loop A */
    for (i = 0; i < n; ++i) {
        int a = i * 2;
        
        if (i == n/2) {
            /* Jump into Loop B's body */
            goto overlap_point;
        }
        
        result += a;
        
        /* Loop B - partially overlaps with Loop A via goto */
        for (j = 0; j < 5; ++j) {
            overlap_point:
            int b = i + j * 3;
            int c = b ^ a;
            
            asm volatile("" : : "r"(b), "r"(c));
            result ^= c;
            
            /* Break back to Loop A */
            if (j == 2 && i > n/2) {
                break;
            }
        }
        
        result -= i;
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types with complex relationships */
NOINLINE int test_mixed_loops(int n) {
    int result = 0;
    int i = 0;
    
    /* while loop */
    while (i < n) {
        int j = 0;
        
        /* do-while nested inside while */
        do {
            int a = i * j;
            int b = a + global_seed;
            int c = b ^ (i & j);
            
            asm volatile("" : : "r"(a), "r"(b), "r"(c));
            result += c;
            
            j++;
        } while (j < 3);
        
        /* Another for loop as sibling */
        for (int k = 0; k < 2; k++) {
            int x = (i << k) | (result & 0xF);
            asm volatile("" : : "r"(x));
            result ^= x;
        }
        
        i++;
    }
    
    /* Follow-up for loop that might intersect with previous loops */
    for (int m = 0; m < n/2; m++) {
        int y = result * m;
        asm volatile("" : : "r"(y));
        result -= y;
    }
    
    return result & 0xFF;
}

/* Function 5: Complex nested structure with multiple relationships */
NOINLINE int test_complex_nesting(int n) {
    int result = 0;
    
    /* Level 1 */
    for (int a = 0; a < n; a++) {
        /* Level 2 - first inner */
        for (int b = 0; b < a; b++) {
            /* Level 3 - perfectly nested in both above */
            for (int c = 0; c < 2; c++) {
                int x = a * b * c;
                asm volatile("" : : "r"(x));
                result += x;
            }
        }
        
        /* Level 2 - second inner (sibling to first) */
        int d = 0;
        while (d < 3) {
            int y = a << d;
            asm volatile("" : : "r"(y));
            result ^= y;
            d++;
        }
    }
    
    return result & 0xFF;
}

int main(int argc, char **argv) {
    int total = 0;
    
    /* Use volatile/argc to prevent constant propagation */
    volatile int seed = argc;
    int N = (seed % 100) + 10;
    
    printf("Running hardware loop coverage tests with N=%d\n", N);
    
    /* Call all test functions to ensure they're compiled and executed */
    total ^= test_perfect_nesting(N);
    total ^= test_loop_subset_of_other(N);
    total ^= test_partial_overlap(N);
    total ^= test_mixed_loops(N);
    total ^= test_complex_nesting(N);
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
