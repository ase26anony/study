/* test_hwloop_coverage.c
 * 
 * This test is designed for targets with hardware loop support.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * 
 * The program creates complex loop structures with specific block relationships
 * to exercise the bitmap intersection logic in hw-doloop.cc:
 * - Perfectly nested loops (subset relationship)
 * - Partially overlapping loops
 * - Sibling loops with one being subset of another
 * - Complex control flow with gotos and labels
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
    
    /* Outer loop - this will be 'loop' in the analysis */
    for (i = 0; i < n; ++i) {
        /* No code here to ensure inner loop is perfect subset */
        
        /* Inner loop - this will be 'other' in the analysis */
        for (j = 0; j < i; ++j) {
            /* Create register pressure */
            int a = i + j;
            int b = i * j;
            int c = b - a;
            int d = (a * b) >> (c & 3);
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            
            result ^= d;
        }
        
        /* No code here either to maintain subset relationship */
    }
    
    return result & 0xFF;
}

/* Function 2: Loop with inner sibling loops - loop is subset of other
 * This should trigger: other->loops.safe_push(loop)
 */
NOINLINE int test_sibling_subsets(int n) {
    int result = 0;
    int i, j, k;
    
    /* Outer loop - this will be 'other' in the analysis */
    for (i = 0; i < n; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (j = 0; j < 5; ++j) {
            int a = i * j;
            int b = a + global_seed;
            result += b;
            asm volatile("" : : "r"(a), "r"(b));
        }
        
        /* Second inner loop - this will be 'loop' in the analysis */
        for (k = 0; k < i; ++k) {
            /* Create more register pressure */
            int x = i * k;
            int y = x ^ result;
            int z = y << (k & 3);
            
            asm volatile("" : : "r"(x), "r"(y), "r"(z));
            result ^= z;
        }
        
        /* More code in outer loop after inner loops */
        result += i * 3;
    }
    
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops via goto
 * This should trigger the first condition (bitmap_intersect_p = true)
 * but not the subset conditions
 */
NOINLINE int test_partial_overlap(int n) {
    int result = 0;
    int i = 0, j = 0;
    
    /* Loop A */
    for (i = 0; i < n; ++i) {
        int a = i * 2;
        result += a;
        
        if (i == n/2) {
            /* Jump into the middle of another loop */
            goto inside_loop_b;
        }
        
        /* Loop B - partially overlaps with Loop A via goto */
        for (j = 0; j < n; ++j) {
            inside_loop_b:
            int b = j * 3;
            int c = b + i;
            
            asm volatile("" : : "r"(b), "r"(c));
            result ^= c;
            
            if (j > n/2 && i < n/2) {
                /* Jump back out */
                goto after_loop_b;
            }
        }
        after_loop_b:
        
        /* More computation */
        result -= i;
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types with complex control flow */
NOINLINE int test_mixed_loops(int n) {
    int result = 0;
    int i = 0;
    
    /* while loop */
    while (i < n) {
        int j = 0;
        
        /* do-while loop inside while */
        do {
            int a = i * j;
            int b = a ^ global_seed;
            int c = b << (j & 3);
            
            asm volatile("" : : "r"(a), "r"(b), "r"(c));
            result += c;
            
            j++;
        } while (j < 5);
        
        /* Another for loop as sibling */
        for (int k = 0; k < i; k++) {
            int x = k * result;
            int y = x >> 2;
            
            asm volatile("" : : "r"(x), "r"(y));
            result ^= y;
            
            if (k == 3) {
                /* Early exit creates more complex CFG */
                break;
            }
        }
        
        i++;
    }
    
    return result & 0xFF;
}

/* Function 5: Deep nesting with multiple levels */
NOINLINE int test_deep_nesting(int n) {
    int result = 0;
    
    /* Level 1 */
    for (int i = 0; i < n; i++) {
        /* Level 2 */
        for (int j = 0; j < i; j++) {
            /* Level 3 */
            for (int k = 0; k < j; k++) {
                /* Heavy computation for register pressure */
                int a = i * j * k;
                int b = a + global_seed;
                int c = b ^ result;
                int d = c << (k & 3);
                int e = d >> 1;
                int f = e * 7;
                
                asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f));
                result += f;
            }
            
            /* Code between nested loops */
            result ^= j * 11;
        }
        
        /* More outer loop code */
        if (i % 2 == 0) {
            result += i * 13;
        } else {
            result -= i * 17;
        }
    }
    
    return result & 0xFF;
}

/* Main function to drive all tests */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile and command line to prevent constant propagation */
    volatile int base = argc;
    int n1 = (base % 50) + 20;
    int n2 = (base % 40) + 30;
    int n3 = (base % 30) + 10;
    int n4 = (base % 20) + 15;
    int n5 = (base % 10) + 25;
    
    /* Run all test functions */
    total ^= test_perfect_nesting(n1);
    total ^= test_sibling_subsets(n2);
    total ^= test_partial_overlap(n3);
    total ^= test_mixed_loops(n4);
    total ^= test_deep_nesting(n5);
    
    /* Ensure result is used to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
