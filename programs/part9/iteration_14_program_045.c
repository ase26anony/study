/* test_hwloop_coverage.c
 * 
 * This test is designed for targets with hardware loop support.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * 
 * The program creates complex loop structures with specific block relationships
 * to exercise the bitmap intersection logic in hw-doloop.cc:
 * - Perfectly nested loops (subset relationship)
 * - Partially overlapping loops
 * - Sibling loops with one being subset of outer
 * - Disjoint loops for comparison
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline, cold))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Function 1: Perfectly nested loops - other is subset of loop
 * This should trigger: loop->loops.safe_push(other)
 */
NOINLINE int test_perfect_nesting(int n) {
    int result = 0;
    int i, j;
    
    /* Outer loop - this will be 'loop' */
    for (i = 0; i < n; ++i) {
        /* No code here to ensure inner loop is perfect subset */
        
        /* Inner loop - this will be 'other' (subset of outer) */
        for (j = 0; j < i + 1; ++j) {
            /* Create register pressure */
            int a = i * j;
            int b = (i << 3) ^ j;
            int c = a + b - (i & j);
            int d = c * 7;
            int e = d ^ (a | b);
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e));
            
            result ^= (a * b + c) ^ d;
        }
        
        /* No code here either to maintain subset relationship */
    }
    
    return result & 0xFF;
}

/* Function 2: Loop where 'loop' is subset of 'other'
 * This should trigger: other->loops.safe_push(loop)
 */
NOINLINE int test_loop_subset_of_other(int n) {
    int result = 0;
    int i, j, k;
    
    /* Outer loop - this will be 'other' */
    for (i = 0; i < n; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (j = 0; j < 3; ++j) {
            int temp = i * j * 7;
            result += temp;
            asm volatile("" : : "r"(temp));
        }
        
        /* Some intermediate code in outer loop */
        int intermediate = i * 13;
        asm volatile("" : : "r"(intermediate));
        
        /* Second inner loop - this will be 'loop' (subset of 'other') */
        for (k = 0; k < i % 5 + 2; ++k) {
            /* Register pressure */
            int a = i * k;
            int b = k << 2;
            int c = a ^ b;
            int d = c * 11;
            
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            result ^= (a + b) * c - d;
        }
        
        /* More code in outer loop after 'loop' */
        result += intermediate * 3;
    }
    
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops via goto
 * Creates loops that intersect but neither is subset
 */
NOINLINE int test_partial_overlap(int n) {
    int result = 0;
    int i = 0, j = 0;
    
    /* First loop - will be 'loop' */
    for (i = 0; i < n; ++i) {
        int a = i * 3;
        
    loop_body:
        /* Shared block - both loops will include this via goto */
        int shared = a + i;
        asm volatile("" : : "r"(shared));
        result ^= shared;
        
        /* Second loop - will be 'other' */
        if (j < n / 2) {
            for (j = 0; j < n / 2; ++j) {
                int b = j * 5;
                asm volatile("" : : "r"(b));
                result += b;
                
                /* Jump into first loop's body */
                if (j == i % 3) {
                    goto loop_body;
                }
            }
        }
        
        /* Continue with first loop */
        result += a * 7;
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types (do-while inside for) */
NOINLINE int test_mixed_loops(int n) {
    int result = 0;
    int i;
    
    /* Outer for loop */
    for (i = 0; i < n; ++i) {
        int counter = 0;
        
        /* Inner do-while loop */
        do {
            /* Register pressure */
            int a = counter * i;
            int b = a << 1;
            int c = b ^ 0x55;
            int d = c + i;
            int e = d * 3;
            int f = e ^ counter;
            
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f));
            
            result += (a * b + c * d) ^ (e | f);
            counter++;
        } while (counter < (i % 4 + 1));
        
        /* While loop after do-while */
        int w = 0;
        while (w < 2) {
            result ^= (i * w) << w;
            asm volatile("" : : "r"(w));
            w++;
        }
    }
    
    return result & 0xFF;
}

/* Function 5: Complex nested structure with sibling loops */
NOINLINE int test_complex_nesting(int n) {
    int result = 0;
    int i, j, k;
    
    /* Level 1: Outer loop */
    for (i = 0; i < n; ++i) {
        /* Level 2: First middle loop */
        for (j = 0; j < (i % 3 + 2); ++j) {
            /* Level 3: Innermost loop A */
            for (k = 0; k < 2; ++k) {
                int val = (i * j * k) ^ 0xAA;
                asm volatile("" : : "r"(val));
                result += val;
            }
            
            /* Some code between inner loops */
            int mid = j * 17;
            asm volatile("" : : "r"(mid));
            
            /* Level 3: Innermost loop B (sibling of A) */
            for (k = 0; k < 3; ++k) {
                int val = (i + j + k) * 11;
                asm volatile("" : : "r"(val));
                result ^= val;
            }
        }
        
        /* Level 2: Second middle loop (sibling of first) */
        for (j = 0; j < (i % 2 + 1); ++j) {
            int val = i * j * 19;
            asm volatile("" : : "r"(val));
            result -= val;
        }
    }
    
    return result & 0xFF;
}

/* Function 6: Disjoint loops for baseline comparison */
NOINLINE int test_disjoint_loops(int n) {
    int result = 0;
    int i;
    
    /* First completely independent loop */
    for (i = 0; i < n; ++i) {
        int a = i * 2;
        asm volatile("" : : "r"(a));
        result += a;
    }
    
    /* Some unrelated code */
    result ^= 0x55;
    
    /* Second independent loop */
    for (i = n - 1; i >= 0; --i) {
        int b = i * 3;
        asm volatile("" : : "r"(b));
        result ^= b;
    }
    
    return result & 0xFF;
}

int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile and argc to prevent constant propagation */
    volatile int seed = argc;
    int base_n = (seed % 50) + 20;  /* Range: 20-69 */
    
    /* Call all test functions with different sizes */
    total ^= test_perfect_nesting(base_n);
    total ^= test_loop_subset_of_other(base_n + 5);
    total ^= test_partial_overlap(base_n + 10);
    total ^= test_mixed_loops(base_n + 15);
    total ^= test_complex_nesting(base_n + 20);
    total ^= test_disjoint_loops(base_n + 25);
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    /* Generate coverage data */
    return (total & 0xFF) == 0 ? 0 : 1;
}
