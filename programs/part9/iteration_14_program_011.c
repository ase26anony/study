/* test_hwloop.c
 * This test is designed for targets with hardware loop support.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a test_hwloop.c -o test_hwloop
 * Or for generic targets: gcc -O3 -funroll-loops -fpeel-loops -fprofile-arcs -ftest-coverage test_hwloop.c -o test_hwloop
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline, cold))

/* Function 1: Perfect nesting - other is subset of loop (Condition 2) */
NOINLINE int perfect_nesting(int N) {
    int result = 0;
    volatile int sink;
    
    /* Outer loop - this will be 'loop' */
    for (int i = 0; i < N; ++i) {
        /* Inner loop - this will be 'other' (subset of outer) */
        for (int j = 0; j < (N - i); ++j) {
            /* Create register pressure */
            int a = i * j;
            int b = a + i;
            int c = b - j;
            int d = c * a;
            int e = d >> 2;
            result ^= e;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(result));
        }
        
        /* Small amount of code in outer loop but not in inner */
        result += i;
    }
    
    sink = result;
    return sink & 0xFF;
}

/* Function 2: Reverse subset - loop is subset of other (Condition 3) */
NOINLINE int reverse_subset(int N) {
    int result = 0;
    volatile int sink;
    
    /* Outer loop - this will be 'other' */
    for (int i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' but not in 'loop' */
        for (int j = 0; j < 3; ++j) {
            result += i * j;
            asm volatile("" : : "r"(result));
        }
        
        /* Second inner loop - this will be 'loop' (subset of outer) */
        for (int k = 0; k < (N - i); ++k) {
            /* Register pressure */
            int a = i + k;
            int b = a * k;
            int c = b - i;
            int d = c ^ result;
            result = d;
            
            asm volatile("" : : "r"(result));
        }
        
        /* More code in outer loop after inner loops */
        result ^= i;
    }
    
    sink = result;
    return sink & 0xFF;
}

/* Function 3: Partial overlap with goto - triggers bitmap intersection (Condition 1) */
NOINLINE int partial_overlap_goto(int N) {
    int result = 0;
    volatile int sink;
    
    /* Loop A - will be 'loop' */
    for (int i = 0; i < N; ++i) {
        int a = i * 2;
        int b = a + 1;
        
    shared_block:
        /* This block will be shared between both loops via goto */
        result ^= (a * b) >> 2;
        asm volatile("" : : "r"(result));
        
        /* Loop B - will be 'other' */
        for (int j = 0; j < 5; ++j) {
            if (j == 3 && i < N/2) {
                /* Jump into Loop A's body, creating intersection */
                goto shared_block;
            }
            
            int c = j * i;
            int d = c + result;
            result = d ^ j;
            asm volatile("" : : "r"(result));
        }
        
        result += i;
    }
    
    sink = result;
    return sink & 0xFF;
}

/* Function 4: Mixed loop types with complex nesting */
NOINLINE int mixed_loop_types(int N) {
    int result = 0;
    volatile int sink;
    
    /* for loop */
    for (int i = 0; i < N; ++i) {
        /* do-while loop inside for */
        int j = 0;
        do {
            int a = i * j;
            int b = a + result;
            int c = b ^ (i + j);
            result = c;
            asm volatile("" : : "r"(result));
            j++;
        } while (j < 4);
        
        /* while loop after do-while */
        int k = 0;
        while (k < 3) {
            result ^= (i * k);
            asm volatile("" : : "r"(result));
            k++;
        }
    }
    
    sink = result;
    return sink & 0xFF;
}

/* Function 5: Sibling loops with shared header block */
NOINLINE int sibling_loops(int N) {
    int result = 0;
    volatile int sink;
    
    /* Two sequential loops that might share some CFG structure */
    for (int i = 0; i < N; ++i) {
        /* First loop */
        for (int j = 0; j < 2; ++j) {
            result += i * j;
            asm volatile("" : : "r"(result));
        }
    }
    
    /* Second loop with similar but not identical structure */
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < 3; ++j) {
            result ^= i * j;
            asm volatile("" : : "r"(result));
        }
    }
    
    sink = result;
    return sink & 0xFF;
}

/* Main function with volatile inputs to prevent constant propagation */
int main(int argc, char *argv[]) {
    volatile int seed = argc;
    int total = 0;
    
    /* Use command line or volatile to get loop bounds */
    int N1 = (seed % 50) + 10;
    int N2 = (seed % 40) + 15;
    int N3 = (seed % 30) + 20;
    int N4 = (seed % 25) + 25;
    int N5 = (seed % 20) + 30;
    
    /* Call all functions to ensure they're compiled and executed */
    total ^= perfect_nesting(N1);
    total ^= reverse_subset(N2);
    total ^= partial_overlap_goto(N3);
    total ^= mixed_loop_types(N4);
    total ^= sibling_loops(N5);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
