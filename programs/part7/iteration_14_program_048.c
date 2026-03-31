/* 
 * Test program for hardware loop optimization coverage.
 * Designed to trigger bitmap intersection logic in hw-doloop.cc lines 429-436.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline, cold))
#define OPTIMIZE __attribute__((optimize("O2")))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Function 1: Perfect nesting - other is subset of loop */
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
            int a = i * j;
            int b = a ^ j;
            int c = b - i;
            int d = c * a;
            int e = d >> 2;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e));
            
            result ^= (a + b - c) * d;
        }
        
        /* No code here either to maintain subset relationship */
    }
    
    return result & 0xFF;
}

/* Function 2: Loop is subset of other */
NOINLINE OPTIMIZE
int test_loop_subset_of_other(int n) {
    int result = 0;
    int i, j, k;
    
    /* Outer loop (will be 'other' in the analysis) */
    for (i = 0; i < n; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (j = 0; j < 5; ++j) {
            int a = i * j;
            int b = a ^ 0x55;
            result += a - b;
            asm volatile("" : : "r"(a), "r"(b));
        }
        
        /* Second inner loop (will be 'loop' in the analysis) */
        for (k = 0; k < i; ++k) {
            /* This loop is subset of the outer loop */
            int x = i * k;
            int y = x ^ k;
            int z = y - i;
            result ^= (x * y) >> (z & 3);
            asm volatile("" : : "r"(x), "r"(y), "r"(z));
        }
    }
    
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops using goto */
NOINLINE OPTIMIZE
int test_partial_overlap_goto(int n) {
    int result = 0;
    int i = 0, j = 0;
    
    /* Loop A (will be 'loop' in some analysis) */
    for (i = 0; i < n; ++i) {
        int a = i * 3;
        
    loop_a_body:
        result += a ^ i;
        
        /* Loop B (will be 'other' in some analysis) */
        for (j = 0; j < 3; ++j) {
            int b = i * j;
            result -= b;
            
            if (j == 1 && i % 2 == 0) {
                /* Jump into loop A's body, creating intersection */
                goto loop_a_body;
            }
            
            asm volatile("" : : "r"(b));
        }
        
        int c = result * i;
        asm volatile("" : : "r"(c));
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types with complex relationships */
NOINLINE OPTIMIZE
int test_mixed_loops(int n) {
    int result = 0;
    int i = 0;
    
    /* while loop */
    while (i < n) {
        int j = 0;
        
        /* do-while loop inside while */
        do {
            int a = i * j;
            int b = a ^ 0xAA;
            int c = b << 2;
            result += c - a;
            asm volatile("" : : "r"(a), "r"(b), "r"(c));
            j++;
        } while (j < 5);
        
        /* for loop after do-while */
        for (int k = 0; k < i; ++k) {
            int x = result * k;
            int y = x ^ result;
            result = y - k;
            asm volatile("" : : "r"(x), "r"(y));
            
            /* Nested if with label for potential goto */
            if (k == i/2) {
                result ^= 0xFF;
            }
        }
        
        i++;
    }
    
    return result & 0xFF;
}

/* Function 5: Sibling loops with shared basic block via switch */
NOINLINE OPTIMIZE
int test_sibling_loops_switch(int n) {
    int result = 0;
    int mode = n % 3;
    
    /* First loop */
    for (int i = 0; i < n; ++i) {
        switch (mode) {
            case 0:
                result += i * 2;
                break;
            case 1:
                result -= i;
                break;
            default:
                result ^= i;
                break;
        }
        
        int a = result * i;
        asm volatile("" : : "r"(a));
    }
    
    /* Second loop that shares the switch basic block pattern */
    for (int j = n; j > 0; --j) {
        switch (mode) {
            case 0:
                result += j * 3;
                break;
            case 1:
                result -= j * 2;
                break;
            default:
                result ^= j * 5;
                break;
        }
        
        int b = result ^ j;
        asm volatile("" : : "r"(b));
    }
    
    return result & 0xFF;
}

/* Main function to drive all tests */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile and argc to prevent constant propagation */
    volatile int seed = argc;
    int N = (seed % 100) + 10;
    
    /* Call all test functions with different parameters */
    total ^= test_perfect_nesting(N);
    total ^= test_loop_subset_of_other(N + 1);
    total ^= test_partial_overlap_goto(N + 2);
    total ^= test_mixed_loops(N + 3);
    total ^= test_sibling_loops_switch(N + 4);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
