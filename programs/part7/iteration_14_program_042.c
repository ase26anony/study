/* 
 * Test program for hardware loop optimization coverage
 * Target: ARMv8-A with hardware loop support
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a -c hwloop_test.c
 * Run with: ./hwloop_test
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline, cold))
#define OPTIMIZE __attribute__((optimize("O2")))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Function 1: Perfectly nested loops - other is subset of loop */
NOINLINE OPTIMIZE
int test_perfect_nesting(int n) {
    int result = 0;
    int i, j;
    
    /* Outer loop (will be 'loop' in the analysis) */
    for (i = 0; i < n; ++i) {
        /* No code here to ensure other is subset of loop */
        
        /* Inner loop (will be 'other' in the analysis) */
        for (j = 0; j < i; ++j) {
            /* Create register pressure */
            int a = i * 3;
            int b = j * 7;
            int c = a - b;
            int d = (a * b) >> (c & 3);
            result ^= d;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
        }
        
        /* No code here either to maintain subset relationship */
    }
    
    return result & 0xFF;
}

/* Function 2: Loop is subset of other (sibling loops) */
NOINLINE OPTIMIZE
int test_loop_subset_of_other(int n) {
    int result = 0;
    int i, j, k;
    
    /* Outer loop (will be 'other' in the analysis) */
    for (i = 0; i < n; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (j = 0; j < 5; ++j) {
            int a = i + j;
            int b = a * 2;
            result += b;
            asm volatile("" : : "r"(a), "r"(b));
        }
        
        /* Second inner loop (will be 'loop' in the analysis) */
        for (k = 0; k < i; ++k) {
            /* This loop is subset of 'other' */
            int x = i * k;
            int y = x ^ k;
            int z = y >> (k & 3);
            result ^= z;
            asm volatile("" : : "r"(x), "r"(y), "r"(z));
        }
    }
    
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops with goto */
NOINLINE OPTIMIZE
int test_partial_overlap(int n) {
    int result = 0;
    int i = 0, j = 0;
    
    /* Loop A (will be 'loop' in the analysis) */
    for (i = 0; i < n; ++i) {
        int a = i * 2;
        
    loop_a_body:
        result += a;
        asm volatile("" : : "r"(a));
        
        /* Loop B (will be 'other' in the analysis) */
        while (j < n) {
            int b = j * 3;
            result ^= b;
            
            /* Jump into loop A's body to create intersection */
            if (j == i / 2) {
                goto loop_a_body;  /* Creates block intersection */
            }
            
            j++;
            asm volatile("" : : "r"(b));
        }
        
        /* Reset j for next iteration */
        j = i + 1;
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types with do-while inside for */
NOINLINE OPTIMIZE
int test_mixed_loops(int n) {
    int result = 0;
    int i = 0;
    
    /* Outer for loop */
    for (i = 0; i < n; ++i) {
        int counter = 0;
        
        /* Inner do-while loop */
        do {
            int a = i + counter;
            int b = a * counter;
            int c = b ^ result;
            result = c;
            
            asm volatile("" : : "r"(a), "r"(b), "r"(c));
            counter++;
        } while (counter < 3);
        
        /* Additional operations to create more blocks */
        if (i % 2 == 0) {
            result += i;
        } else {
            result -= i;
        }
    }
    
    return result & 0xFF;
}

/* Function 5: Complex nested structure with sibling loops */
NOINLINE OPTIMIZE
int test_complex_nesting(int n) {
    int result = 0;
    int i, j, k;
    
    /* Level 1: Outer loop */
    for (i = 0; i < n; ++i) {
        /* Level 2: First middle loop */
        for (j = 0; j < i; ++j) {
            /* Level 3: Innermost loop A */
            for (k = 0; k < 2; ++k) {
                int a = i * j * k;
                result += a;
                asm volatile("" : : "r"(a));
            }
        }
        
        /* Level 2: Second middle loop (sibling to first) */
        for (j = n - 1; j >= 0; --j) {
            /* Level 3: Innermost loop B */
            int m = 0;
            while (m < 2) {
                int b = i ^ j ^ m;
                result ^= b;
                asm volatile("" : : "r"(b));
                m++;
            }
        }
    }
    
    return result & 0xFF;
}

/* Function 6: Disjoint loops (should not trigger intersection) */
NOINLINE OPTIMIZE
int test_disjoint_loops(int n) {
    int result = 0;
    int i;
    
    /* First independent loop */
    for (i = 0; i < n; ++i) {
        result += i * 2;
        asm volatile("" : : "r"(i));
    }
    
    /* Completely separate code section */
    if (result > 100) {
        result = 100;
    }
    
    /* Second independent loop */
    for (i = n; i > 0; --i) {
        result -= i;
        asm volatile("" : : "r"(i));
    }
    
    return result & 0xFF;
}

/* Main function with volatile inputs to prevent optimization */
int main(int argc, char *argv[]) {
    int total = 0;
    int i;
    
    /* Use volatile and argc to create non-constant loop bounds */
    volatile int seed = argc;
    int N1 = (seed % 50) + 10;
    int N2 = (seed % 40) + 15;
    int N3 = (seed % 30) + 20;
    int N4 = (seed % 25) + 25;
    int N5 = (seed % 20) + 30;
    int N6 = (seed % 15) + 35;
    
    /* Call all test functions to ensure they're compiled and executed */
    total ^= test_perfect_nesting(N1);
    total ^= test_loop_subset_of_other(N2);
    total ^= test_partial_overlap(N3);
    total ^= test_mixed_loops(N4);
    total ^= test_complex_nesting(N5);
    total ^= test_disjoint_loops(N6);
    
    /* Additional loop in main to create more opportunities */
    for (i = 0; i < 100; ++i) {
        total += (i * global_seed) & 0xFF;
        asm volatile("" : : "r"(i));
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return total & 0xFF;
}
