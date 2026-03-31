/* test_hwloop.c
 * This test is designed for targets with hardware loop support.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a -c test_hwloop.c -o test_hwloop.o
 * Or for generic targets: gcc -O3 -funroll-loops -fpeel-loops -fprofile-arcs -ftest-coverage test_hwloop.c -o test_hwloop_executable
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline, cold))

/* Force hardware loop optimization on supported targets */
#ifdef __ARM_ARCH
#define TARGET_HWLOOP __attribute__((target("arch=armv8-a+lse")))
#else
#define TARGET_HWLOOP
#endif

/* Use volatile assembly to prevent optimization */
#define KEEP(i) asm volatile("" : : "r"(i))

/* Function 1: Perfectly nested loops - other is subset of loop */
NOINLINE TARGET_HWLOOP
int perfect_nesting(int N) {
    int result = 0;
    volatile int v = N; /* Prevent constant propagation */
    int limit = v;
    
    /* Outer loop - this will be 'loop' */
    for (int i = 0; i < limit; ++i) {
        /* No code here to ensure other is subset */
        
        /* Inner loop - this will be 'other' (subset of loop) */
        for (int j = 0; j < i + 1; ++j) {
            /* Create register pressure */
            int a = i * j;
            int b = a + j;
            int c = b - i;
            int d = c * 3;
            int e = d >> 2;
            result ^= e;
            KEEP(result);
        }
        
        /* No code here either to maintain subset property */
    }
    
    return result & 0xFF;
}

/* Function 2: Loop is subset of other */
NOINLINE TARGET_HWLOOP
int loop_subset_of_other(int N) {
    int result = 0;
    volatile int v = N;
    int limit = v;
    
    /* Outer loop - this will be 'other' */
    for (int i = 0; i < limit; ++i) {
        /* First inner loop - creates blocks in other not in loop */
        for (int j = 0; j < 2; ++j) {
            int a = i + j;
            int b = a * 7;
            result += b;
            KEEP(result);
        }
        
        /* Second inner loop - this will be 'loop' (subset of other) */
        for (int k = 0; k < i + 3; ++k) {
            /* More register pressure */
            int x = i * k;
            int y = x ^ k;
            int z = y + result;
            result = z & 0xFFFF;
            KEEP(result);
        }
        
        /* More code in other after loop */
        result ^= (i << 3);
    }
    
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops with goto */
NOINLINE TARGET_HWLOOP
int overlapping_with_goto(int N) {
    int result = 0;
    volatile int v = N;
    int limit = v;
    
    /* Loop A - will be 'loop' */
    for (int i = 0; i < limit; ++i) {
        int a = i * 3;
        
    shared_block:
        result += a;
        KEEP(result);
        
        /* Loop B - will be 'other', shares block via goto */
        for (int j = 0; j < 5; ++j) {
            if (j == 3 && i < limit/2) {
                goto shared_block; /* Creates intersection */
            }
            result ^= j;
            KEEP(result);
        }
        
        result -= i;
    }
    
    return result & 0xFF;
}

/* Function 4: Complex nested structure with mixed loop types */
NOINLINE TARGET_HWLOOP
int mixed_loop_types(int N) {
    int result = 0;
    volatile int v = N;
    int limit = v;
    
    /* Outer for loop */
    for (int i = 0; i < limit; ++i) {
        /* Inner do-while loop */
        int dw = 0;
        do {
            result += dw * i;
            dw++;
            KEEP(result);
        } while (dw < 3);
        
        /* Inner while loop */
        int w = 0;
        while (w < i % 5) {
            result ^= (w << i);
            w++;
            KEEP(result);
        }
        
        /* Another for loop inside */
        for (int k = 0; k < 2; ++k) {
            int x = i * k + result;
            int y = x % 256;
            int z = y | 0x1F;
            result = z;
            KEEP(result);
        }
    }
    
    return result & 0xFF;
}

/* Function 5: Sibling loops with break to shared label */
NOINLINE TARGET_HWLOOP
int sibling_loops_with_break(int N) {
    int result = 0;
    volatile int v = N;
    int limit = v;
    
    /* First loop */
    for (int i = 0; i < limit; ++i) {
        if (i % 2 == 0) {
            for (int j = 0; j < 4; ++j) {
                result += j * i;
                if (j == 2 && i > limit/2) {
                    goto shared_exit; /* Jump to shared block */
                }
                KEEP(result);
            }
        }
    }
    
shared_exit:
    result += 0xAA;
    
    /* Second loop that also can reach shared_exit */
    for (int k = 0; k < limit/2; ++k) {
        result ^= k * 3;
        if (k == limit/4) {
            goto shared_exit; /* Another path to shared block */
        }
        KEEP(result);
    }
    
    return result & 0xFF;
}

/* Function 6: Disjoint loops (should not trigger intersection) */
NOINLINE TARGET_HWLOOP
int disjoint_loops(int N) {
    int result = 0;
    volatile int v = N;
    int limit = v;
    
    /* First completely separate loop */
    for (int i = 0; i < limit; ++i) {
        result += i * 2;
        KEEP(result);
    }
    
    /* Unrelated code between loops */
    result ^= 0x55;
    int temp = result * 3;
    
    /* Second completely separate loop */
    for (int j = 0; j < limit/2; ++j) {
        result -= j + temp;
        KEEP(result);
    }
    
    return result & 0xFF;
}

/* Main function with volatile inputs to prevent optimization */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile and command line to prevent constant propagation */
    volatile int seed = argc;
    int N1 = (seed % 100) + 10;
    int N2 = (seed % 50) + 20;
    int N3 = (seed % 30) + 15;
    int N4 = (seed % 40) + 5;
    int N5 = (seed % 60) + 8;
    int N6 = (seed % 70) + 12;
    
    /* Call all functions to ensure they're compiled and executed */
    total ^= perfect_nesting(N1);
    total ^= loop_subset_of_other(N2);
    total ^= overlapping_with_goto(N3);
    total ^= mixed_loop_types(N4);
    total ^= sibling_loops_with_break(N5);
    total ^= disjoint_loops(N6);
    
    /* Generate side effect to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return total & 1;
}
