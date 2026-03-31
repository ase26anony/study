/* test_hwloop.c
 * 
 * This test is designed for targets with hardware loop support.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * 
 * The program creates complex loop structures with specific block relationships
 * to exercise the bitmap intersection logic in hw-doloop.cc:
 * - Perfectly nested loops (subset relationship)
 * - Partially overlapping loops
 * - Sibling loops with one being a subset of outer loop
 * - Various loop types (for, while, do-while)
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline, cold))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Function 1: Perfectly nested loops - other is subset of loop */
NOINLINE int perfect_nesting(int n) {
    int result = 0;
    int i, j;
    
    /* Outer loop - this will be 'loop' in the hierarchy */
    for (i = 0; i < n; ++i) {
        /* No code here to ensure inner loop is perfect subset */
        
        /* Inner loop - this will be 'other' (subset of outer) */
        for (j = 0; j < i; ++j) {
            /* Create register pressure */
            int a = i + j;
            int b = i * j;
            int c = b - a;
            int d = (a * b) >> (c & 3);
            result ^= d;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
        }
        
        /* No code here either - inner loop is perfect subset */
    }
    
    return result & 0xFF;
}

/* Function 2: Loop with inner sibling loops - loop is subset of other */
NOINLINE int sibling_subsets(int n) {
    int result = 0;
    int i, j, k;
    
    /* Outer loop - this will be 'other' */
    for (i = 0; i < n; ++i) {
        /* First inner loop - creates blocks in 'other' not in second loop */
        for (j = 0; j < 5; ++j) {
            int a = i * j;
            result += a;
            asm volatile("" : : "r"(a));
        }
        
        /* Second inner loop - this will be 'loop' (subset of outer) */
        for (k = 0; k < i; ++k) {
            int b = i + k;
            int c = b * 3;
            result ^= c;
            asm volatile("" : : "r"(b), "r"(c));
        }
    }
    
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops with goto */
NOINLINE int overlapping_with_goto(int n) {
    int result = 0;
    int i = 0, j = 0;
    
    /* Loop A - will be 'loop' */
    for (i = 0; i < n; ++i) {
        int a = i * 2;
        result += a;
        
    shared_label:
        /* This block is shared between both loops */
        int shared = a + result;
        asm volatile("" : : "r"(shared));
        
        /* Loop B - will be 'other' (partially overlaps with loop A) */
        while (j < n) {
            int b = j * 3;
            result ^= b;
            
            /* Jump into loop A to create intersection */
            if (j == n/2) {
                goto shared_label;
            }
            
            j++;
            
            /* Complex body for register pressure */
            int x = b + i;
            int y = x * result;
            int z = y >> 2;
            result += z;
            asm volatile("" : : "r"(x), "r"(y), "r"(z));
        }
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types with complex nesting */
NOINLINE int mixed_loop_types(int n) {
    int result = 0;
    int i = 0;
    
    /* do-while as outer loop */
    do {
        /* for loop inside do-while */
        for (int j = 0; j < 3; ++j) {
            int a = i * j;
            
            /* while loop inside for loop */
            int k = 0;
            while (k < 2) {
                int b = a + k;
                result ^= b;
                
                /* Create many variables for register pressure */
                int c = b * 2;
                int d = c - a;
                int e = d >> 1;
                int f = e * result;
                result += f & 0xF;
                
                asm volatile("" : : "r"(b), "r"(c), "r"(d), "r"(e), "r"(f));
                k++;
            }
        }
        
        i++;
    } while (i < n);
    
    return result & 0xFF;
}

/* Function 5: Complex hierarchy with multiple levels */
NOINLINE int multi_level_hierarchy(int n) {
    int result = 0;
    
    /* Level 1: Outer loop */
    for (int i = 0; i < n; ++i) {
        /* Level 2: Middle loop 1 */
        for (int j = 0; j < i; ++j) {
            /* Level 3: Inner loop 1 */
            int k = 0;
            while (k < 5) {
                int a = i + j + k;
                result += a;
                
                /* More register pressure */
                int b = a * 2;
                int c = b - i;
                int d = c ^ j;
                result ^= d;
                
                asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
                k++;
            }
        }
        
        /* Level 2: Middle loop 2 (sibling) */
        for (int m = 0; m < 3; ++m) {
            int e = i * m;
            result += e;
            
            /* Small do-while inside */
            int p = 0;
            do {
                int f = e + p;
                result ^= f;
                p++;
            } while (p < 2);
        }
    }
    
    return result & 0xFF;
}

/* Main function with volatile inputs to prevent optimization */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile and argc to create variable loop bounds */
    volatile int seed = argc;
    int n1 = (seed % 50) + 10;
    int n2 = (seed % 40) + 15;
    int n3 = (seed % 30) + 20;
    int n4 = (seed % 20) + 25;
    int n5 = (seed % 10) + 30;
    
    /* Call all functions to ensure they're compiled and executed */
    total ^= perfect_nesting(n1);
    total ^= sibling_subsets(n2);
    total ^= overlapping_with_goto(n3);
    total ^= mixed_loop_types(n4);
    total ^= multi_level_hierarchy(n5);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
