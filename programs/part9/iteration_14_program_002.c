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

/* Function 1: Perfectly nested loops - other is subset of loop */
NOINLINE int perfect_nesting(int n) {
    int result = 0;
    int i, j;
    
    /* Outer loop - this will be 'loop' in the analysis */
    for (i = 0; i < n; ++i) {
        /* No code here to ensure inner loop blocks are subset of outer */
        
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

/* Function 2: Loop with subset relationship reversed - loop is subset of other */
NOINLINE int reversed_subset(int n) {
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
            /* This loop's blocks are subset of outer loop's blocks */
            int x = i * k;
            int y = x - global_seed;
            int z = (x * y) >> 2;
            
            asm volatile("" : : "r"(x), "r"(y), "r"(z));
            result ^= z;
        }
    }
    
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops using goto */
NOINLINE int overlapping_with_goto(int n) {
    int result = 0;
    int i, j;
    
    /* First loop - will be 'loop' */
    for (i = 0; i < n; ++i) {
        int a = i * 3;
        result += a;
        
    shared_label:
        /* This block will be shared between both loops */
        int shared = a + global_seed;
        asm volatile("" : : "r"(shared));
        
        /* Second loop - will be 'other' */
        for (j = 0; j < 3; ++j) {
            int b = i * j + shared;
            result ^= b;
            
            /* Jump to shared block in first loop */
            if (j == 1 && i % 2 == 0) {
                goto shared_label;
            }
        }
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types (for, while, do-while) */
NOINLINE int mixed_loop_types(int n) {
    int result = 0;
    int i = 0;
    
    /* Outer for loop */
    for (i = 0; i < n; ++i) {
        int j = 0;
        
        /* Inner while loop */
        while (j < 5) {
            int a = i * j;
            int b = a + global_seed;
            result += b;
            asm volatile("" : : "r"(a), "r"(b));
            j++;
        }
        
        /* Another inner do-while loop */
        int k = 0;
        do {
            int x = i * k;
            int y = x - global_seed;
            result ^= y;
            asm volatile("" : : "r"(x), "r"(y));
            k++;
        } while (k < 3);
    }
    
    return result & 0xFF;
}

/* Function 5: Complex nested structure with break to shared block */
NOINLINE int complex_break_structure(int n) {
    int result = 0;
    int i, j;
    
    /* Outer loop */
    for (i = 0; i < n; ++i) {
        /* Middle loop - will share blocks via break */
        for (j = 0; j < 10; ++j) {
            int a = i * j;
            
            /* Break to shared code in outer loop */
            if (a > 50) {
                result += a;
                break;  /* This creates shared exit block */
            }
            
            int b = a + global_seed;
            result ^= b;
            asm volatile("" : : "r"(a), "r"(b));
        }
        
        /* Shared block after break */
        int shared = i * global_seed;
        asm volatile("" : : "r"(shared));
        result += shared;
        
        /* Innermost loop */
        for (int k = 0; k < 3; ++k) {
            int c = shared * k;
            result ^= c;
            asm volatile("" : : "r"(c));
        }
    }
    
    return result & 0xFF;
}

/* Function 6: Sibling loops with one loop containing the other's header */
NOINLINE int sibling_loops_partial_overlap(int n) {
    int result = 0;
    int i;
    
    /* First loop */
    i = 0;
    while (i < n) {
        int a = i * 2;
        result += a;
        
        /* This increment block might be shared */
        i++;
        
        if (i % 3 == 0) {
            /* Jump into second loop's body */
            goto second_loop_body;
        }
    }
    
    /* Second loop */
    for (int j = 0; j < n; j++) {
        int b = j * 3;
        
    second_loop_body:
        /* Shared block */
        int shared = b + global_seed;
        result ^= shared;
        asm volatile("" : : "r"(shared));
    }
    
    return result & 0xFF;
}

int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile to prevent constant propagation */
    volatile int seed = argc;
    int N = (seed % 100) + 10;
    
    printf("Running hardware loop coverage test with N=%d\n", N);
    
    /* Call all test functions to ensure they're compiled and executed */
    total ^= perfect_nesting(N);
    total ^= reversed_subset(N);
    total ^= overlapping_with_goto(N);
    total ^= mixed_loop_types(N);
    total ^= complex_break_structure(N);
    total ^= sibling_loops_partial_overlap(N);
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
