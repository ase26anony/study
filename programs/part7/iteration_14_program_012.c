/* 
 * Test program for hw-doloop.cc coverage
 * Designed for targets with hardware loop support (ARM, RISC-V with Ziloop, PowerPC)
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a -c test_hwloop.c
 * Or for generic testing: gcc -O3 -funroll-loops -fpeel-loops -fprofile-arcs -ftest-coverage test_hwloop.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline, cold))

/* Function 1: Perfect nesting - other is subset of loop (Condition 2) */
NOINLINE int perfect_nesting(int N) {
    volatile int result = 0;
    int a, b, c;
    
    /* Outer loop - this will be 'loop' in the analysis */
    for (int i = 0; i < N; ++i) {
        /* No code here ensures loop has no blocks outside other */
        
        /* Inner loop - this will be 'other' in the analysis */
        /* other's blocks are subset of loop's blocks */
        for (int j = 0; j < i; ++j) {
            /* Create register pressure */
            a = i * j;
            b = a + j;
            c = b - i;
            result ^= (a * b) >> (c & 7);
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c));
        }
        
        /* No code here either to maintain subset relationship */
    }
    
    return result & 255;
}

/* Function 2: Reverse nesting - loop is subset of other (Condition 3) */
NOINLINE int reverse_nesting(int N) {
    volatile int result = 0;
    int x, y, z;
    
    /* Outer loop - this will be 'other' in the analysis */
    for (int i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 3; ++j) {
            x = i * j;
            y = x + 12345;
            result ^= y;
            asm volatile("" : : "r"(x), "r"(y));
        }
        
        /* Second inner loop - this will be 'loop' in the analysis */
        /* loop's blocks are subset of other's blocks */
        for (int k = 0; k < i; ++k) {
            x = i * k;
            y = x + k;
            z = y - i;
            result ^= (x * y) >> (z & 7);
            asm volatile("" : : "r"(x), "r"(y), "r"(z));
        }
    }
    
    return result & 255;
}

/* Function 3: Partial overlap with goto (Condition 1) */
NOINLINE int partial_overlap(int N) {
    volatile int result = 0;
    int p, q, r;
    
    /* Loop A - will be 'loop' in some analysis */
    for (int i = 0; i < N; ++i) {
        p = i * 2;
        
    loop_a_body:
        q = p + i;
        result ^= q;
        
        /* Loop B - will be 'other' in some analysis */
        /* Shares block via goto to loop_a_body */
        for (int j = 0; j < 5; ++j) {
            r = i * j;
            result += r;
            
            if (j == 3 && i % 2 == 0) {
                /* Jump into loop A's body, creating intersection */
                goto loop_a_body;
            }
            
            asm volatile("" : : "r"(r));
        }
        
        asm volatile("" : : "r"(p), "r"(q));
    }
    
    return result & 255;
}

/* Function 4: Mixed loop types with complex relationships */
NOINLINE int mixed_loops(int N) {
    volatile int result = 0;
    int a, b, c, d;
    
    /* while loop */
    int w = 0;
    while (w < N) {
        /* do-while nested inside while */
        int dw = 0;
        do {
            a = w * dw;
            b = a + 123;
            result ^= b;
            asm volatile("" : : "r"(a), "r"(b));
            dw++;
        } while (dw < 3);
        
        /* for loop after do-while */
        for (int i = 0; i < w; i++) {
            c = w * i;
            d = c - 456;
            result += d;
            asm volatile("" : : "r"(c), "r"(d));
            
            /* Another nested for creating subset relationship */
            for (int j = 0; j < 2; j++) {
                result ^= (c * j) & 255;
            }
        }
        
        w++;
    }
    
    return result & 255;
}

/* Function 5: Sibling loops with shared header block */
NOINLINE int sibling_loops(int N) {
    volatile int result = 0;
    int tmp1, tmp2;
    
    /* Two sequential loops that might be analyzed as overlapping */
    for (int i = 0; i < N; ++i) {
        tmp1 = i * 3;
        result += tmp1;
    }
    
    /* Second loop that might share some CFG structure */
    for (int j = 0; j < N; ++j) {
        tmp2 = j * 5;
        result ^= tmp2;
        
        /* Small inner loop to create hierarchy */
        for (int k = 0; k < 2; ++k) {
            asm volatile("" : : "r"(tmp2));
        }
    }
    
    return result & 255;
}

/* Main function with volatile inputs to prevent constant propagation */
int main(int argc, char *argv[]) {
    volatile int seed = argc;
    int N1 = (seed % 100) + 10;
    int N2 = (seed % 50) + 5;
    int N3 = (seed % 30) + 3;
    int N4 = (seed % 20) + 2;
    int N5 = (seed % 10) + 1;
    
    int result = 0;
    
    /* Call all functions to ensure they're compiled and executed */
    result ^= perfect_nesting(N1);
    result ^= reverse_nesting(N2);
    result ^= partial_overlap(N3);
    result ^= mixed_loops(N4);
    result ^= sibling_loops(N5);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result & 255);
    
    return 0;
}
