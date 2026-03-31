/* test_mcf_coverage.c
 * 
 * This program creates a complex register allocation scenario designed to
 * trigger the min-cost flow solver's debug dumping code, specifically
 * the uncovered lines in dump_fixup_edge that handle NEW_EXIT and NEW_ENTRY
 * node labels.
 *
 * Compile with: gcc -O3 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 * Or for ARM: gcc -O3 -march=armv7-a -mtune=cortex-a8 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 */

/* Force inclusion of MCF debugging */
#ifndef MCF_DEBUG
#define MCF_DEBUG 1
#endif

#include <stdio.h>
#include <stdlib.h>

/* Complex function with many overlapping live ranges to maximize register pressure */
__attribute__((noinline))
int test_ira_conflict(int iterations) {
    /* Declare many variables that will have overlapping live ranges */
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5;
    volatile int f = 6, g = 7, h = 8, i = 9, j = 10;
    volatile int k = 11, l = 12, m = 13, n = 14, o = 15;
    volatile int p = 16, q = 17, r = 18, s = 19, t = 20;
    
    int result = 0;
    
    /* Nested loops with many live variables across loop boundaries */
    for (int outer = 0; outer < iterations; outer++) {
        /* All variables are live at this point */
        a = b + c;
        b = c + d;
        c = d + e;
        d = e + f;
        e = f + g;
        
        /* Inner loop with complex dependencies */
        for (int inner = 0; inner < 10; inner++) {
            /* More computations keeping many variables live */
            f = g + h + inner;
            g = h + i + outer;
            h = i + j + a;
            i = j + k + b;
            j = k + l + c;
            
            /* Use volatile asm to clobber many registers */
            asm volatile (
                "mov r0, %0\n\t"
                "mov r1, %1\n\t"
                "mov r2, %2\n\t"
                "mov r3, %3\n\t"
                :
                : "r" (f), "r" (g), "r" (h), "r" (i)
                : "r0", "r1", "r2", "r3", "memory"
            );
            
            /* More computations */
            k = l + m + d;
            l = m + n + e;
            m = n + o + f;
            n = o + p + g;
            o = p + q + h;
            
            /* Another asm clobber */
            asm volatile (
                "add %0, %1, %2\n\t"
                : "=r" (p)
                : "r" (q), "r" (r)
                : "cc"
            );
            
            p = q + r + i;
            q = r + s + j;
            r = s + t + k;
            s = t + a + l;
            t = a + b + m;
            
            /* Accumulate result */
            result += a + b + c + d + e + f + g + h + i + j +
                     k + l + m + n + o + p + q + r + s + t;
        }
        
        /* Force spill/reload by using all variables after loop */
        asm volatile (
            "mov r0, %0\n\t"
            "mov r1, %1\n\t"
            "mov r2, %2\n\t"
            "mov r3, %3\n\t"
            "mov r4, %4\n\t"
            "mov r5, %5\n\t"
            :
            : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e), "r" (f)
            : "r0", "r1", "r2", "r3", "r4", "r5", "memory"
        );
    }
    
    /* Final computation using all variables */
    result = a + b + c + d + e + f + g + h + i + j +
            k + l + m + n + o + p + q + r + s + t;
    
    return result;
}

/* Second test function with different register pressure pattern */
__attribute__((noinline))
int test_ira_conflict2(int seed) {
    /* Create many pseudo-registers with complex dependency graph */
    int v1 = seed * 1;
    int v2 = seed * 2;
    int v3 = seed * 3;
    int v4 = seed * 4;
    int v5 = seed * 5;
    int v6 = seed * 6;
    int v7 = seed * 7;
    int v8 = seed * 8;
    int v9 = seed * 9;
    int v10 = seed * 10;
    int v11 = seed * 11;
    int v12 = seed * 12;
    int v13 = seed * 13;
    int v14 = seed * 14;
    int v15 = seed * 15;
    int v16 = seed * 16;
    int v17 = seed * 17;
    int v18 = seed * 18;
    int v19 = seed * 19;
    int v20 = seed * 20;
    
    /* Create a complex web of dependencies */
    for (int i = 0; i < 100; i++) {
        v1 = v2 + v3;
        v2 = v3 + v4;
        v3 = v4 + v5;
        v4 = v5 + v6;
        v5 = v6 + v7;
        v6 = v7 + v8;
        v7 = v8 + v9;
        v8 = v9 + v10;
        v9 = v10 + v11;
        v10 = v11 + v12;
        v11 = v12 + v13;
        v12 = v13 + v14;
        v13 = v14 + v15;
        v14 = v15 + v16;
        v15 = v16 + v17;
        v16 = v17 + v18;
        v17 = v18 + v19;
        v18 = v19 + v20;
        v19 = v20 + v1;
        v20 = v1 + v2;
        
        /* Force register pressure with inline asm that uses many registers */
        asm volatile (
            "/* Clobber many registers */\n\t"
            "mov r0, %0\n\t"
            "mov r1, %1\n\t"
            "mov r2, %2\n\t"
            "mov r3, %3\n\t"
            "mov r4, %4\n\t"
            "mov r5, %5\n\t"
            "mov r6, %6\n\t"
            "mov r7, %7\n\t"
            :
            : "r" (v1), "r" (v2), "r" (v3), "r" (v4),
              "r" (v5), "r" (v6), "r" (v7), "r" (v8)
            : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "memory"
        );
    }
    
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
}

/* Third test: Function with switch statement creating complex CFG */
__attribute__((noinline))
int test_ira_conflict3(int mode) {
    int x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5;
    int x6 = 6, x7 = 7, x8 = 8, x9 = 9, x10 = 10;
    
    /* Switch creates complex control flow graph */
    switch (mode % 8) {
        case 0:
            x1 = x2 + x3;
            x2 = x3 + x4;
            /* fall through */
        case 1:
            x3 = x4 + x5;
            x4 = x5 + x6;
            break;
        case 2:
            x5 = x6 + x7;
            x6 = x7 + x8;
            /* fall through */
        case 3:
            x7 = x8 + x9;
            x8 = x9 + x10;
            x9 = x10 + x1;
            break;
        case 4:
            x10 = x1 + x2;
            x1 = x2 + x3;
            x2 = x3 + x4;
            x3 = x4 + x5;
            /* fall through */
        case 5:
            x4 = x5 + x6;
            x5 = x6 + x7;
            x6 = x7 + x8;
            break;
        case 6:
        case 7:
            x7 = x8 + x9;
            x8 = x9 + x10;
            x9 = x10 + x1;
            x10 = x1 + x2;
            x1 = x2 + x3;
            break;
    }
    
    /* Loop with all variables live */
    for (int i = 0; i < 50; i++) {
        x1 = x2 + x3 + i;
        x2 = x3 + x4 + i;
        x3 = x4 + x5 + i;
        x4 = x5 + x6 + i;
        x5 = x6 + x7 + i;
        x6 = x7 + x8 + i;
        x7 = x8 + x9 + i;
        x8 = x9 + x10 + i;
        x9 = x10 + x1 + i;
        x10 = x1 + x2 + i;
    }
    
    return x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9 + x10;
}

/* Main function that exercises all test cases */
int main(int argc, char **argv) {
    int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    int total = 0;
    
    printf("Starting IRA/MCF coverage test...\n");
    
    /* Call test functions multiple times with different parameters
     * to explore different register allocation scenarios */
    for (int run = 0; run < 5; run++) {
        total += test_ira_conflict(iterations + run);
        total += test_ira_conflict2(iterations + run * 10);
        total += test_ira_conflict3(iterations + run * 5);
    }
    
    printf("Total result: %d\n", total);
    return total != 0 ? 0 : 1;
}
