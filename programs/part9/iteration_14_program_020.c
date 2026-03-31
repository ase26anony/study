/* 
 * Test program for hw-doloop.cc coverage
 * Designed to trigger bitmap intersection logic in discover_loop_hierarchy
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a -c test_hwloop.c
 * Or for generic target: gcc -O3 -funroll-loops -fpeel-loops -fprofile-arcs -ftest-coverage test_hwloop.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function compilation */
#define NOINLINE __attribute__((noinline, cold))

/* Function 1: Perfect nesting - other is subset of loop */
NOINLINE int perfect_nesting(int N) {
    volatile int result = 0;
    int i, j;
    
    /* Outer loop - this will be 'loop' */
    for (i = 0; i < N; ++i) {
        /* No code here to ensure other is subset */
        
        /* Inner loop - this will be 'other' (subset of loop) */
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
        
        /* No code here either to maintain subset relationship */
    }
    
    return result & 0xFF;
}

/* Function 2: Loop is subset of other */
NOINLINE int loop_subset_of_other(int N) {
    volatile int result = 0;
    int i, j, k;
    
    /* Outer loop - this will be 'other' */
    for (i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (j = 0; j < 3; ++j) {
            int a = i * j;
            result += a;
            asm volatile("" : : "r"(a));
        }
        
        /* Second inner loop - this will be 'loop' (subset of other) */
        for (k = 0; k < i; ++k) {
            /* Register pressure */
            int x = i + k;
            int y = i * k;
            int z = y - x;
            result ^= (x * y) >> (z & 3);
            asm volatile("" : : "r"(x), "r"(y), "r"(z));
        }
    }
    
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops with goto */
NOINLINE int partial_overlap_goto(int N) {
    volatile int result = 0;
    int i, j;
    
    /* Loop A - will be 'loop' */
    for (i = 0; i < N; ++i) {
        int a = i * 2;
        result += a;
        
    shared_label:
        /* This block is shared between loops */
        int b = a + i;
        result ^= b;
        
        /* Loop B - will be 'other' */
        for (j = 0; j < 5; ++j) {
            int c = i + j;
            result += c;
            
            if (c > 10) {
                /* Jump into Loop A's body */
                goto shared_label;
            }
            
            asm volatile("" : : "r"(c));
        }
        
        asm volatile("" : : "r"(a), "r"(b));
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types with complex nesting */
NOINLINE int mixed_loop_types(int N) {
    volatile int result = 0;
    int i = 0;
    
    /* while loop */
    while (i < N) {
        int j = 0;
        
        /* do-while loop inside while */
        do {
            int a = i * j;
            int b = a << 2;
            int c = b - a;
            result ^= c;
            asm volatile("" : : "r"(a), "r"(b), "r"(c));
            j++;
        } while (j < 5);
        
        /* for loop after do-while */
        for (int k = 0; k < i; k++) {
            int x = i + k;
            int y = x * 3;
            result += y;
            asm volatile("" : : "r"(x), "r"(y));
        }
        
        i++;
    }
    
    return result & 0xFF;
}

/* Function 5: Sibling loops with break to shared label */
NOINLINE int sibling_loops_break(int N) {
    volatile int result = 0;
    int i, j;
    
    /* First loop */
    for (i = 0; i < N; ++i) {
        int a = i * 3;
        result += a;
        
        if (a > 100) {
            goto shared_exit;
        }
    }
    
    /* Second loop - shares exit block via break */
    for (j = 0; j < N * 2; ++j) {
        int b = j * 2;
        result ^= b;
        
        if (b > 50) {
            break;
        }
        
        asm volatile("" : : "r"(b));
    }
    
shared_exit:
    /* Shared exit block */
    int final = result * 2;
    asm volatile("" : : "r"(final));
    return final & 0xFF;
}

/* Main function to drive all test cases */
int main(int argc, char **argv) {
    volatile int seed = argc;
    int total = 0;
    
    /* Use volatile to prevent constant propagation */
    int N1 = (seed % 50) + 10;
    int N2 = (seed % 40) + 15;
    int N3 = (seed % 30) + 20;
    int N4 = (seed % 25) + 25;
    int N5 = (seed % 20) + 30;
    
    /* Call all functions to ensure they're compiled and executed */
    total ^= perfect_nesting(N1);
    total ^= loop_subset_of_other(N2);
    total ^= partial_overlap_goto(N3);
    total ^= mixed_loop_types(N4);
    total ^= sibling_loops_break(N5);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
