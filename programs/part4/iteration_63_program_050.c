/* test_mcf_coverage.c
 * 
 * This program is designed to trigger the uncovered lines in GCC's
 * mcf.cc dump_fixup_edge function, specifically the branches that
 * print "NEW_EXIT" and "NEW_ENTRY" labels.
 *
 * Compile with: gcc -O3 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 * Or for ARM:   gcc -O3 -march=armv7-a -mtune=cortex-a8 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 */

#include <stdio.h>
#include <stdlib.h>

/* Force many overlapping live ranges with complex control flow */
__attribute__((noinline))
static void test_ira_conflict(int iterations) {
    /* Declare many variables that will have overlapping live ranges */
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5;
    volatile int f = 6, g = 7, h = 8, i = 9, j = 10;
    volatile int k = 11, l = 12, m = 13, n = 14, o = 15;
    volatile int p = 16, q = 17, r = 18, s = 19, t = 20;
    
    /* Complex nested loops to create overlapping live ranges */
    for (int outer = 0; outer < iterations; outer++) {
        /* Many variables live across the inner loop */
        int sum1 = a + b + c + d + e;
        int sum2 = f + g + h + i + j;
        
        /* Inner loop with more live variables */
        for (int inner = 0; inner < 100; inner++) {
            /* Force all variables to be used and kept alive */
            int temp1 = sum1 + k + l + m;
            int temp2 = sum2 + n + o + p;
            
            /* Complex computation keeping many values live */
            a = b + temp1;
            b = c + temp2;
            c = d + a;
            d = e + b;
            e = f + c;
            
            f = g + d;
            g = h + e;
            h = i + f;
            i = j + g;
            j = k + h;
            
            /* Use inline asm to clobber many registers */
            asm volatile (
                "mov r0, %0\n\t"
                "mov r1, %1\n\t"
                "mov r2, %2\n\t"
                "mov r3, %3\n\t"
                :
                : "r" (temp1), "r" (temp2), "r" (sum1), "r" (sum2)
                : "r0", "r1", "r2", "r3", "cc", "memory"
            );
            
            /* More computations to extend live ranges */
            k = l + i;
            l = m + j;
            m = n + k;
            n = o + l;
            o = p + m;
            
            p = q + n;
            q = r + o;
            r = s + p;
            s = t + q;
            t = a + r;
        }
        
        /* Conditional branches to create different control flow paths */
        if (outer % 3 == 0) {
            /* Use more variables in this path */
            int mix = a * b * c * d;
            asm volatile ("" : : "r" (mix) : "memory");
        } else if (outer % 3 == 1) {
            /* Different variable combination */
            int mix = e * f * g * h;
            asm volatile ("" : : "r" (mix) : "memory");
        } else {
            /* Yet another combination */
            int mix = i * j * k * l;
            asm volatile ("" : : "r" (mix) : "memory");
        }
    }
    
    /* Final use to prevent optimization */
    asm volatile ("" : : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
                       "r" (f), "r" (g), "r" (h), "r" (i), "r" (j),
                       "r" (k), "r" (l), "r" (m), "r" (n), "r" (o),
                       "r" (p), "r" (q), "r" (r), "r" (s), "r" (t) : "memory");
}

/* Second test function with different variable usage patterns */
__attribute__((noinline))
static void test_ira_conflict2(int seed) {
    /* Create a data flow pattern that might trigger fixup edges */
    int v1 = seed, v2 = seed * 2, v3 = seed * 3, v4 = seed * 4;
    int v5 = seed * 5, v6 = seed * 6, v7 = seed * 7, v8 = seed * 8;
    int v9 = seed * 9, v10 = seed * 10, v11 = seed * 11, v12 = seed * 12;
    
    /* Unrolled loop to create many parallel live ranges */
    for (int i = 0; i < 50; i++) {
        /* Chain of dependencies forcing sequential evaluation */
        v1 = v2 + v12;
        v2 = v3 + v1;
        v3 = v4 + v2;
        v4 = v5 + v3;
        v5 = v6 + v4;
        v6 = v7 + v5;
        v7 = v8 + v6;
        v8 = v9 + v7;
        v9 = v10 + v8;
        v10 = v11 + v9;
        v11 = v12 + v10;
        v12 = v1 + v11;
        
        /* Force spilling by using many variables in asm */
        asm volatile (
            "add %0, %0, %1\n\t"
            "add %2, %2, %3\n\t"
            "add %4, %4, %5\n\t"
            "add %6, %6, %7\n\t"
            : "+r" (v1), "+r" (v2), "+r" (v3), "+r" (v4),
              "+r" (v5), "+r" (v6), "+r" (v7), "+r" (v8)
            :
            : "cc"
        );
    }
    
    /* Return all values to prevent dead code elimination */
    asm volatile ("" : : "r" (v1), "r" (v2), "r" (v3), "r" (v4),
                       "r" (v5), "r" (v6), "r" (v7), "r" (v8),
                       "r" (v9), "r" (v10), "r" (v11), "r" (v12) : "memory");
}

/* Third test: Function with switch statement creating complex CFG */
__attribute__((noinline))
static void test_ira_conflict3(int mode) {
    int x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5;
    int x6 = 6, x7 = 7, x8 = 8, x9 = 9, x10 = 10;
    
    switch (mode % 5) {
        case 0:
            x1 = x2 + x3;
            x2 = x4 + x5;
            x3 = x6 + x7;
            /* Fall through */
        case 1:
            x4 = x8 + x9;
            x5 = x10 + x1;
            x6 = x2 + x3;
            break;
        case 2:
            x7 = x4 + x5;
            x8 = x6 + x7;
            x9 = x8 + x9;
            break;
        case 3:
            x10 = x1 + x2;
            x1 = x3 + x4;
            x2 = x5 + x6;
            x3 = x7 + x8;
            break;
        case 4:
            x4 = x9 + x10;
            x5 = x1 + x2;
            x6 = x3 + x4;
            x7 = x5 + x6;
            x8 = x7 + x8;
            x9 = x9 + x10;
            break;
    }
    
    /* Use results */
    asm volatile ("" : : "r" (x1), "r" (x2), "r" (x3), "r" (x4), "r" (x5),
                       "r" (x6), "r" (x7), "r" (x8), "r" (x9), "r" (x10) : "memory");
}

/* Main function that exercises different scenarios */
int main(int argc, char **argv) {
    int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    int seed = (argc > 2) ? atoi(argv[2]) : 42;
    
    /* Call test functions multiple times with different parameters
     * to explore different register allocation scenarios */
    for (int run = 0; run < 3; run++) {
        test_ira_conflict(iterations + run);
        test_ira_conflict2(seed + run);
        test_ira_conflict3(run);
    }
    
    return 0;
}
