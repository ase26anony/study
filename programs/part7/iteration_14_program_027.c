/* 
 * Test program for hardware loop bitmap intersection coverage.
 * Designed to trigger specific conditions in hw-doloop.cc lines 429-436.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function compilation */
#define NOINLINE __attribute__((noinline, cold))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Function 1: Perfect nesting - other is subset of loop (Condition 2) */
NOINLINE int test_perfect_nesting(int N) {
    int result = 0;
    int a, b, c;
    
    /* Outer loop - this will be 'loop' in the analysis */
    for (int i = 0; i < N; ++i) {
        /* No code here to ensure inner loop is proper subset */
        
        /* Inner loop - this will be 'other' in the analysis */
        for (int j = 0; j < (N - i); ++j) {
            /* Create register pressure */
            a = i + j;
            b = a * 2;
            c = b - i;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c));
            
            result ^= (a * b) >> (c & 7);
        }
        
        /* No code here either - inner loop is proper subset */
    }
    
    return result & 0xFF;
}

/* Function 2: Loop is subset of other (Condition 3) */
NOINLINE int test_loop_subset_of_other(int N) {
    int result = 0;
    int x, y, z;
    
    /* Outer loop - this will be 'other' in the analysis */
    for (int i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 3; ++j) {
            x = i * j;
            y = x + 1;
            asm volatile("" : : "r"(x), "r"(y));
            result += x - y;
        }
        
        /* Second inner loop - this will be 'loop' in the analysis */
        for (int k = 0; k < (N - i); ++k) {
            /* Create more complex operations */
            x = i + k;
            y = x * 3;
            z = y - k;
            
            asm volatile("" : : "r"(x), "r"(y), "r"(z));
            result ^= (z * x) | y;
        }
    }
    
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops with goto (Condition 1) */
NOINLINE int test_partial_overlap(int N) {
    int result = 0;
    int a, b, c;
    
    /* Loop A - will be 'loop' in analysis */
    for (int i = 0; i < N; ++i) {
        a = i * 2;
        
    shared_label:
        b = a + i;
        c = b * 3;
        
        asm volatile("" : : "r"(a), "r"(b), "r"(c));
        result += c - a;
        
        /* Loop B - will be 'other' in analysis */
        for (int j = 0; j < 5; ++j) {
            if (j == 3 && result > 100) {
                /* Jump into Loop A's body, creating intersection */
                goto shared_label;
            }
            
            a = j * i;
            b = a + result;
            asm volatile("" : : "r"(a), "r"(b));
            result ^= b;
        }
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types with do-while and while */
NOINLINE int test_mixed_loops(int N) {
    int result = 0;
    int i = 0;
    int a, b, c;
    
    /* for loop */
    for (i = 0; i < N; ++i) {
        a = i * 2;
        
        /* do-while loop inside for */
        int j = 0;
        do {
            b = a + j;
            c = b * 3;
            asm volatile("" : : "r"(b), "r"(c));
            result += c;
            j++;
        } while (j < 4);
    }
    
    /* while loop after for */
    int k = 0;
    while (k < N) {
        a = k * 3;
        b = result - a;
        asm volatile("" : : "r"(a), "r"(b));
        result ^= b;
        k++;
    }
    
    return result & 0xFF;
}

/* Function 5: Complex nested structure with sibling loops */
NOINLINE int test_sibling_loops(int N) {
    int result = 0;
    int x, y, z;
    
    /* Outer loop */
    for (int i = 0; i < N; ++i) {
        /* Sibling loop A */
        for (int j = 0; j < i; ++j) {
            x = i * j;
            y = x + 1;
            asm volatile("" : : "r"(x), "r"(y));
            result += x * y;
        }
        
        /* Some intermediate code */
        x = i * 3;
        asm volatile("" : : "r"(x));
        
        /* Sibling loop B */
        for (int k = 0; k < (N - i); ++k) {
            y = i + k;
            z = y * 2;
            asm volatile("" : : "r"(y), "r"(z));
            result ^= z - y;
        }
    }
    
    return result & 0xFF;
}

/* Main function with volatile inputs to prevent optimization */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile and command line to prevent constant propagation */
    volatile int seed = argc;
    int N1 = (seed % 50) + 10;
    int N2 = (seed % 40) + 15;
    int N3 = (seed % 30) + 20;
    int N4 = (seed % 60) + 5;
    int N5 = (seed % 45) + 8;
    
    /* Call all test functions */
    total += test_perfect_nesting(N1);
    total += test_loop_subset_of_other(N2);
    total += test_partial_overlap(N3);
    total += test_mixed_loops(N4);
    total += test_sibling_loops(N5);
    
    /* Generate side effect to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
