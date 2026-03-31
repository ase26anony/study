/* 
 * Test program for hardware loop optimization coverage.
 * Designed to trigger bitmap intersection logic in hw-doloop.cc lines 429-436.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * For RISC-V: -march=rv64gc_ziloop
 * For PowerPC: -mcpu=power8
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function compilation */
#define NOINLINE __attribute__((noinline, cold))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Function 1: Perfect nesting - other is subset of loop */
NOINLINE int test_perfect_nesting(int n) {
    int result = 0;
    int a, b, c;
    
    /* Outer loop - this will be 'loop' */
    for (int i = 0; i < n; ++i) {
        /* No code here to ensure inner loop is subset */
        
        /* Inner loop - this will be 'other' (subset of outer) */
        for (int j = 0; j < i; ++j) {
            /* Create register pressure */
            a = i * j;
            b = a + j;
            c = b - i;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c));
            
            result ^= (a * b) >> (c & 7);
        }
        
        /* No code here either to maintain subset relationship */
    }
    
    return result & 0xFF;
}

/* Function 2: Reverse nesting - loop is subset of other */
NOINLINE int test_reverse_nesting(int n) {
    int result = 0;
    int x, y, z;
    
    /* Outer loop - this will be 'other' */
    for (int i = 0; i < n; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 3; ++j) {
            x = i * j;
            y = x + global_seed;
            asm volatile("" : : "r"(x), "r"(y));
            result += x - y;
        }
        
        /* Second inner loop - this will be 'loop' (subset of outer) */
        for (int k = 0; k < i; ++k) {
            /* Create more register pressure */
            x = i * k;
            y = k * k;
            z = x ^ y;
            
            asm volatile("" : : "r"(x), "r"(y), "r"(z));
            result ^= (z * x) | y;
        }
    }
    
    return result & 0xFF;
}

/* Function 3: Partial overlap with goto */
NOINLINE int test_partial_overlap(int n) {
    int result = 0;
    int tmp1, tmp2, tmp3;
    
    /* Loop A - will be 'loop' */
    for (int i = 0; i < n; ++i) {
        tmp1 = i * 3;
        tmp2 = tmp1 + global_seed;
        
    shared_label:
        /* This block will be shared */
        tmp3 = tmp1 ^ tmp2;
        result += tmp3;
        
        /* Loop B - will be 'other' */
        for (int j = 0; j < 5; ++j) {
            if (j == 3 && i < n/2) {
                /* Jump into Loop A's body */
                goto shared_label;
            }
            
            tmp1 = j * i;
            tmp2 = tmp1 - j;
            asm volatile("" : : "r"(tmp1), "r"(tmp2));
            result ^= tmp1 * tmp2;
        }
        
        /* More operations in Loop A */
        tmp1 = result * i;
        asm volatile("" : : "r"(tmp1));
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types */
NOINLINE int test_mixed_loops(int n) {
    int result = 0;
    int a, b, c, d;
    
    /* for loop */
    for (int i = 0; i < n; ++i) {
        a = i * 2;
        b = a + global_seed;
        
        /* do-while loop inside */
        int k = 0;
        do {
            c = k * a;
            d = b - k;
            asm volatile("" : : "r"(c), "r"(d));
            result += c ^ d;
            k++;
        } while (k < 4);
        
        /* while loop after */
        int m = 0;
        while (m < 3) {
            c = m * i;
            asm volatile("" : : "r"(c));
            result -= c;
            m++;
        }
    }
    
    return result & 0xFF;
}

/* Function 5: Complex nested structure */
NOINLINE int test_complex_nesting(int n) {
    int result = 0;
    int v1, v2, v3, v4, v5;
    
    /* Level 1 */
    for (int i = 0; i < n; ++i) {
        v1 = i * 7;
        
        /* Level 2 - first */
        for (int j = 0; j < i; ++j) {
            v2 = j * 11;
            
            /* Level 3 */
            for (int k = 0; k < 2; ++k) {
                v3 = k * 13;
                v4 = v1 + v2 + v3;
                asm volatile("" : : "r"(v3), "r"(v4));
                result += v4;
            }
            
            /* Another Level 3 */
            int m = 0;
            while (m < 3) {
                v5 = m * 17;
                asm volatile("" : : "r"(v5));
                result ^= v5;
                m++;
            }
        }
        
        /* Level 2 - second (creates sibling relationship) */
        for (int p = 0; p < 5; ++p) {
            v1 = p * 19;
            v2 = v1 ^ result;
            asm volatile("" : : "r"(v1), "r"(v2));
            result -= v2;
        }
    }
    
    return result & 0xFF;
}

/* Main function with volatile inputs */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile and argc to prevent constant propagation */
    volatile int seed = argc;
    int N = (seed % 100) + 10;
    
    /* Call all test functions */
    total += test_perfect_nesting(N);
    total += test_reverse_nesting(N + 5);
    total += test_partial_overlap(N + 10);
    total += test_mixed_loops(N + 15);
    total += test_complex_nesting(N + 20);
    
    /* Generate side effect to prevent elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
