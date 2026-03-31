/* 
 * Test program for hardware loop bitmap intersection coverage
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a -c test_hwloop.c
 * Or for generic target: gcc -O3 -funroll-loops -fpeel-loops -fprofile-arcs -ftest-coverage test_hwloop.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function compilation */
#define NOINLINE __attribute__((noinline, cold))

/* Target-specific hint for hardware loops */
#ifdef __ARM_ARCH
#define HWLOOP_TARGET __attribute__((target("arch=armv8-a+lse")))
#else
#define HWLOOP_TARGET
#endif

/* Function 1: Perfect nesting - other is subset of loop (Condition 2) */
NOINLINE HWLOOP_TARGET
int perfect_nesting(int N) {
    volatile int result = 0;
    int i, j;
    
    /* Outer loop - this will be 'loop' */
    for (i = 0; i < N; ++i) {
        /* No code here to ensure other is subset of loop */
        
        /* Inner loop - this will be 'other' (subset of loop) */
        for (j = 0; j < 5; ++j) {
            /* Create register pressure */
            int a = i + j;
            int b = i * 2 + j;
            int c = b - a;
            int d = (a * b) >> (c & 3);
            
            /* Prevent optimization */
            asm volatile ("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            
            result ^= d;
        }
        
        /* No code here either to maintain subset relationship */
    }
    
    return result & 0xFF;
}

/* Function 2: Loop is subset of other (Condition 3) */
NOINLINE HWLOOP_TARGET
int loop_subset_of_other(int N) {
    volatile int result = 0;
    int i, j, k;
    
    /* Outer loop - this will be 'other' */
    for (i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (j = 0; j < 3; ++j) {
            int a = i * j;
            int b = a + 1;
            result += b;
            asm volatile ("" : : "r"(a), "r"(b));
        }
        
        /* Second inner loop - this will be 'loop' (subset of other) */
        for (k = 0; k < 4; ++k) {
            /* Create register pressure */
            int x = i + k;
            int y = x * 2;
            int z = y - x;
            int w = (x * y) >> (z & 3);
            
            asm volatile ("" : : "r"(x), "r"(y), "r"(z), "r"(w));
            result ^= w;
        }
        
        /* More code in 'other' after 'loop' */
        int final = i * 2;
        asm volatile ("" : : "r"(final));
        result += final;
    }
    
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops with goto (Condition 1) */
NOINLINE HWLOOP_TARGET
int overlapping_with_goto(int N) {
    volatile int result = 0;
    int i, j;
    
    /* First loop - this will be 'loop' */
    for (i = 0; i < N; ++i) {
        int a = i * 3;
        asm volatile ("" : : "r"(a));
        result += a;
        
    shared_block:
        /* This label creates shared basic block */
        int shared = i * 5;
        asm volatile ("" : : "r"(shared));
        result ^= shared;
    }
    
    /* Second loop - this will be 'other' */
    for (j = 0; j < N/2; ++j) {
        int b = j * 7;
        asm volatile ("" : : "r"(b));
        result += b;
        
        /* Jump into first loop's body to create intersection */
        if (j == N/4) {
            goto shared_block;
        }
        
        int c = b * 2;
        asm volatile ("" : : "r"(c));
        result ^= c;
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types with complex relationships */
NOINLINE HWLOOP_TARGET
int mixed_loop_types(int N) {
    volatile int result = 0;
    int i = 0;
    
    /* do-while inside for loop */
    for (i = 0; i < N; ++i) {
        int j = 0;
        
        /* do-while loop - creates different CFG structure */
        do {
            int a = i + j;
            int b = a * 2;
            int c = b - i;
            
            asm volatile ("" : : "r"(a), "r"(b), "r"(c));
            result += (a * b) >> (c & 3);
            
            j++;
        } while (j < 3);
        
        /* while loop after do-while */
        int k = 0;
        while (k < 2) {
            int d = i * k;
            asm volatile ("" : : "r"(d));
            result ^= d;
            k++;
        }
    }
    
    /* Another for loop that shares some blocks via switch */
    for (int m = 0; m < N/2; ++m) {
        switch (m % 3) {
            case 0:
                result += m * 2;
                break;
            case 1:
                result ^= m * 3;
                /* Fall through to share block */
            case 2:
                result -= m;
                break;
        }
        asm volatile ("" : : "r"(m));
    }
    
    return result & 0xFF;
}

/* Function 5: Sibling loops with break to shared label */
NOINLINE HWLOOP_TARGET
int sibling_loops_with_break(int N) {
    volatile int result = 0;
    int i, j;
    
    /* First sibling loop */
    for (i = 0; i < N; ++i) {
        if (i == N/2) {
            break;  /* Break to create CFG edge */
        }
        
        int a = i * 11;
        asm volatile ("" : : "r"(a));
        result += a;
    }
    
shared_label:
    /* Shared block after break */
    int shared = i * 13;
    asm volatile ("" : : "r"(shared));
    result ^= shared;
    
    /* Second sibling loop that can jump to shared label */
    for (j = 0; j < N; ++j) {
        int b = j * 17;
        asm volatile ("" : : "r"(b));
        result += b;
        
        if (j == N/3) {
            goto shared_label;
        }
    }
    
    return result & 0xFF;
}

/* Main function with volatile inputs to prevent constant propagation */
int main(int argc, char *argv[]) {
    volatile int seed = argc;
    int total = 0;
    
    /* Use volatile to prevent optimization */
    int N1 = (seed % 100) + 10;
    int N2 = (seed % 50) + 20;
    int N3 = (seed % 30) + 15;
    int N4 = (seed % 40) + 25;
    int N5 = (seed % 60) + 5;
    
    /* Call all test functions */
    total ^= perfect_nesting(N1);
    total ^= loop_subset_of_other(N2);
    total ^= overlapping_with_goto(N3);
    total ^= mixed_loop_types(N4);
    total ^= sibling_loops_with_break(N5);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
