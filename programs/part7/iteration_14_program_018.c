/* 
 * Test program for hardware loop optimization coverage.
 * Designed to trigger bitmap intersection logic in hw-doloop.cc lines 429-436.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * Target: ARMv8-A with hardware loop support (or similar architecture)
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function compilation */
#define NOINLINE __attribute__((noinline, cold))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Function 1: Perfect nesting - other is subset of loop (Condition 2) */
NOINLINE int test_perfect_nesting(int n) {
    int result = 0;
    int i, j;
    
    /* Outer loop (will be 'loop' in hierarchy) */
    for (i = 0; i < n; ++i) {
        /* No code here ensures loop blocks are superset of inner */
        
        /* Inner loop (will be 'other' in hierarchy) - perfect subset */
        for (j = 0; j < i + 1; ++j) {
            /* Create register pressure */
            int a = i * j;
            int b = a ^ j;
            int c = b - i;
            int d = c * a;
            result ^= (d >> 2) & 0xFF;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
        }
        
        /* No code here either - ensures perfect nesting */
    }
    
    return result;
}

/* Function 2: Reverse subset - loop is subset of other (Condition 3) */
NOINLINE int test_reverse_subset(int n) {
    int result = 0;
    int i, j, k;
    
    /* Outer loop (will be 'other' in hierarchy) */
    for (i = 0; i < n; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (j = 0; j < 3; ++j) {
            int a = i + j;
            int b = a * 2;
            result += b & 0xF;
            asm volatile("" : : "r"(a), "r"(b));
        }
        
        /* Second inner loop (will be 'loop' in hierarchy) - subset of outer */
        for (k = 0; k < i; ++k) {
            /* Complex body for register pressure */
            int x = k * 3;
            int y = x ^ result;
            int z = y - k;
            result ^= (z * x) & 0xFF;
            
            asm volatile("" : : "r"(x), "r"(y), "r"(z));
        }
    }
    
    return result;
}

/* Function 3: Partial overlap with goto (Condition 1) */
NOINLINE int test_partial_overlap(int n) {
    int result = 0;
    int i = 0, j = 0;
    
    /* First loop (will be 'loop' in hierarchy) */
    for (i = 0; i < n; ++i) {
        int a = i * 2;
        result += a;
        
    loop_body:
        /* This label creates shared basic block */
        int b = result ^ i;
        asm volatile("" : : "r"(a), "r"(b));
    }
    
    /* Second loop (will be 'other' in hierarchy) with goto into first loop */
    for (j = 0; j < n; ++j) {
        int c = j * 3;
        result ^= c;
        
        if (j == n/2) {
            /* Jump into the first loop's body - creates intersection */
            goto loop_body;
        }
        
        int d = result - j;
        asm volatile("" : : "r"(c), "r"(d));
    }
    
    return result;
}

/* Function 4: Mixed loop types with do-while and while */
NOINLINE int test_mixed_loops(int n) {
    int result = 0;
    int i = 0;
    
    /* for loop */
    for (i = 0; i < n; ++i) {
        /* do-while nested inside for */
        int j = 0;
        do {
            int a = i * j;
            int b = a + result;
            result = b ^ 0x55;
            asm volatile("" : : "r"(a), "r"(b));
            j++;
        } while (j < 3);
    }
    
    /* while loop after for loop (sibling relationship) */
    int k = n;
    while (k > 0) {
        int c = k * result;
        result = c & 0xFF;
        asm volatile("" : : "r"(c));
        k--;
    }
    
    return result;
}

/* Function 5: Complex nested structure with multiple relationships */
NOINLINE int test_complex_nesting(int n) {
    int result = 0;
    
    /* Level 1: Outer loop */
    for (int i = 0; i < n; ++i) {
        /* Level 2: First middle loop */
        for (int j = 0; j < i + 2; ++j) {
            /* Level 3: Innermost loop A */
            for (int k = 0; k < 2; ++k) {
                int a = i * j * k;
                result += a;
                asm volatile("" : : "r"(a));
            }
        }
        
        /* Level 2: Second middle loop (sibling to first) */
        for (int m = 0; m < 3; ++m) {
            /* Level 3: Innermost loop B */
            int count = 0;
            while (count < 2) {
                int b = i * m * count;
                result ^= b;
                asm volatile("" : : "r"(b));
                count++;
            }
        }
    }
    
    return result;
}

int main(int argc, char **argv) {
    int total = 0;
    
    /* Use volatile to prevent constant propagation */
    volatile int seed = argc > 1 ? atoi(argv[1]) : global_seed;
    int N = (seed % 100) + 10;
    
    /* Call all test functions to ensure they're compiled and executed */
    total ^= test_perfect_nesting(N);
    total ^= test_reverse_subset(N);
    total ^= test_partial_overlap(N);
    total ^= test_mixed_loops(N);
    total ^= test_complex_nesting(N);
    
    /* Generate side effect to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
