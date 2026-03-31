/* test_hwloop.c
 * This test is designed for targets with hardware loop support.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a -c test_hwloop.c
 * Or for generic target: gcc -O3 -funroll-loops -fpeel-loops -fprofile-arcs -ftest-coverage test_hwloop.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline, cold))

/* Function 1: Perfect nesting - other is subset of loop */
NOINLINE int perfect_nesting(int N) {
    volatile int result = 0;
    int i, j;
    
    /* Outer loop (will be 'loop' in hierarchy) */
    for (i = 0; i < N; ++i) {
        /* No code here to ensure other is subset */
        
        /* Inner loop (will be 'other' in hierarchy) */
        for (j = 0; j < 5; ++j) {
            /* Create register pressure */
            int a = i + j;
            int b = a * 2;
            int c = b - a;
            int d = (a * b) >> (c & 3);
            asm volatile ("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            result ^= d;
        }
        
        /* No code here either */
    }
    
    return result & 0xFF;
}

/* Function 2: Reverse nesting - loop is subset of other */
NOINLINE int reverse_nesting(int N) {
    volatile int result = 0;
    int i, j, k;
    
    /* Outer loop (will be 'other' in hierarchy) */
    for (i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (j = 0; j < 3; ++j) {
            int a = i * j;
            asm volatile ("" : : "r"(a));
            result += a;
        }
        
        /* Second inner loop (will be 'loop' in hierarchy) */
        for (k = 0; k < 4; ++k) {
            /* Create register pressure */
            int a = i + k;
            int b = a * 3;
            int c = b - a;
            int d = (a * b) >> (c & 3);
            asm volatile ("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            result ^= d;
        }
    }
    
    return result & 0xFF;
}

/* Function 3: Partial overlap with goto - loops intersect but neither is subset */
NOINLINE int partial_overlap(int N) {
    volatile int result = 0;
    int i, j;
    
    /* First loop (could be 'loop' or 'other') */
    for (i = 0; i < N; ++i) {
        int a = i * 2;
        asm volatile ("" : : "r"(a));
        result += a;
        
        if (i == N/2) {
            /* Jump into second loop's body */
            goto inside_second;
        }
    }
    
    /* Second loop */
    for (j = 0; j < N/2; ++j) {
    inside_second:
        int b = j * 3;
        int c = b - j;
        asm volatile ("" : : "r"(b), "r"(c));
        result ^= (b * c);
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types - do-while inside for */
NOINLINE int mixed_loops(int N) {
    volatile int result = 0;
    int i = 0;
    
    /* for loop */
    for (i = 0; i < N; ++i) {
        int j = 0;
        
        /* do-while loop inside */
        do {
            int a = i + j;
            int b = a * 2;
            asm volatile ("" : : "r"(a), "r"(b));
            result += a * b;
            j++;
        } while (j < 3);
    }
    
    /* while loop after for */
    int k = 0;
    while (k < N/2) {
        int c = k * 5;
        asm volatile ("" : : "r"(c));
        result ^= c;
        k++;
    }
    
    return result & 0xFF;
}

/* Function 5: Complex nested structure with sibling loops */
NOINLINE int complex_siblings(int N) {
    volatile int result = 0;
    int i, j, k;
    
    /* Outer loop */
    for (i = 0; i < N; ++i) {
        /* First sibling inner loop */
        for (j = 0; j < 2; ++j) {
            int a = i * j;
            asm volatile ("" : : "r"(a));
            result += a;
        }
        
        /* Second sibling inner loop */
        for (k = 0; k < 3; ++k) {
            int b = i + k;
            int c = b * 2;
            asm volatile ("" : : "r"(b), "r"(c));
            result ^= b * c;
        }
        
        /* Third loop that shares some blocks via conditional */
        if (i % 2 == 0) {
            for (int m = 0; m < 2; ++m) {
                int d = i * m;
                asm volatile ("" : : "r"(d));
                result += d;
            }
        }
    }
    
    return result & 0xFF;
}

int main(int argc, char **argv) {
    volatile int seed = argc;
    int N = (seed % 100) + 10;  /* Prevent constant propagation */
    int total = 0;
    
    /* Call all functions to ensure they're compiled and executed */
    total += perfect_nesting(N);
    total += reverse_nesting(N);
    total += partial_overlap(N);
    total += mixed_loops(N);
    total += complex_siblings(N);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
