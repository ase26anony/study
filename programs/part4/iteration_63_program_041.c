/* test_mcf_coverage.c
 * 
 * This program is designed to trigger the uncovered lines in GCC's
 * Min-Cost Flow solver (mcf.cc) by creating register allocation
 * scenarios that force IRA to build complex fixup graphs with
 * NEW_EXIT and NEW_ENTRY nodes.
 *
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c
 * Or with: gcc -O3 -funroll-loops -fira-algorithm=CB -DMCF_DEBUG -c test_mcf_coverage.c
 */

/* Force inclusion of IRA debugging */
#ifndef MCF_DEBUG
#define MCF_DEBUG 1
#endif

#include <stdio.h>
#include <stdlib.h>

/* Function with many overlapping live ranges to create complex conflict graph */
__attribute__((noinline))
static void test_ira_conflict(int iterations) {
    /* Declare many variables that will have overlapping live ranges */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int w1, w2, w3, w4, w5, w6, w7, w8, w9, w10;
    int x1, x2, x3, x4, x5, x6, x7, x8, x9, x10;
    int y1, y2, y3, y4, y5, y6, y7, y8, y9, y10;
    int z1, z2, z3, z4, z5, z6, z7, z8, z9, z10;
    
    /* Initialize all variables to create definitions */
    v1 = iterations * 1;  v2 = iterations * 2;  v3 = iterations * 3;
    v4 = iterations * 4;  v5 = iterations * 5;  v6 = iterations * 6;
    v7 = iterations * 7;  v8 = iterations * 8;  v9 = iterations * 9;
    v10 = iterations * 10;
    
    w1 = v1 + 1;  w2 = v2 + 1;  w3 = v3 + 1;  w4 = v4 + 1;  w5 = v5 + 1;
    w6 = v6 + 1;  w7 = v7 + 1;  w8 = v8 + 1;  w9 = v9 + 1;  w10 = v10 + 1;
    
    x1 = w1 * 2;  x2 = w2 * 2;  x3 = w3 * 2;  x4 = w4 * 2;  x5 = w5 * 2;
    x6 = w6 * 2;  x7 = w7 * 2;  x8 = w8 * 2;  x9 = w9 * 2;  x10 = w10 * 2;
    
    y1 = x1 / 3;  y2 = x2 / 3;  y3 = x3 / 3;  y4 = x4 / 3;  y5 = x5 / 3;
    y6 = x6 / 3;  y7 = x7 / 3;  y8 = x8 / 3;  y9 = x9 / 3;  y10 = x10 / 3;
    
    /* Nested loops to create complex liveness patterns */
    for (int i = 0; i < iterations; i++) {
        /* Many variables live across loop iterations */
        z1 = v1 + i;  z2 = v2 + i;  z3 = v3 + i;  z4 = v4 + i;  z5 = v5 + i;
        z6 = v6 + i;  z7 = v7 + i;  z8 = v8 + i;  z9 = v9 + i;  z10 = v10 + i;
        
        /* Inner loop with more live variables */
        for (int j = 0; j < 3; j++) {
            /* Use all variables to keep them live */
            v1 = z1 + j;  v2 = z2 + j;  v3 = z3 + j;  v4 = z4 + j;  v5 = z5 + j;
            v6 = z6 + j;  v7 = z7 + j;  v8 = z8 + j;  v9 = z9 + j;  v10 = z10 + j;
            
            w1 = v1 * j;  w2 = v2 * j;  w3 = v3 * j;  w4 = v4 * j;  w5 = v5 * j;
            w6 = v6 * j;  w7 = v7 * j;  w8 = v8 * j;  w9 = v9 * j;  w10 = v10 * j;
            
            /* Volatile asm to clobber registers and increase pressure */
            asm volatile (
                "/* Clobber many registers */"
                :
                : 
                : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                  "r8", "r9", "r10", "r11", "r12", "r14", "memory"
            );
            
            /* More computations to create register pressure */
            x1 = w1 + y1;  x2 = w2 + y2;  x3 = w3 + y3;  x4 = w4 + y4;  x5 = w5 + y5;
            x6 = w6 + y6;  x7 = w7 + y7;  x8 = w8 + y8;  x9 = w9 + y9;  x10 = w10 + y10;
            
            y1 = x1 - z1;  y2 = x2 - z2;  y3 = x3 - z3;  y4 = x4 - z4;  y5 = x5 - z5;
            y6 = x6 - z6;  y7 = x7 - z7;  y8 = x8 - z8;  y9 = x9 - z9;  y10 = x10 - z10;
        }
        
        /* Conditional to create different control flow paths */
        if (i % 2 == 0) {
            /* Use different subsets of variables */
            v1 = y1 + x1;  v3 = y3 + x3;  v5 = y5 + x5;  v7 = y7 + x7;  v9 = y9 + x9;
        } else {
            v2 = y2 + x2;  v4 = y4 + x4;  v6 = y6 + x6;  v8 = y8 + x8;  v10 = y10 + x10;
        }
    }
    
    /* Final use of all variables to prevent optimization */
    asm volatile (
        "/* Final use of variables %0, %1, %2, %3, %4, %5, %6, %7, %8, %9 */"
        :
        : "r" (v1), "r" (v2), "r" (v3), "r" (v4), "r" (v5),
          "r" (v6), "r" (v7), "r" (v8), "r" (v9), "r" (v10)
    );
}

/* Second test function with different variable count to affect graph size */
__attribute__((noinline))
static void test_ira_conflict_2(int iterations) {
    /* Different number of variables to potentially create different
       fixup graph sizes and indices */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15;
    int b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15;
    
    a1 = iterations;  a2 = iterations * 2;  a3 = iterations * 3;
    a4 = iterations * 4;  a5 = iterations * 5;  a6 = iterations * 6;
    a7 = iterations * 7;  a8 = iterations * 8;  a9 = iterations * 9;
    a10 = iterations * 10; a11 = iterations * 11; a12 = iterations * 12;
    a13 = iterations * 13; a14 = iterations * 14; a15 = iterations * 15;
    
    /* Complex loop with many live ranges */
    for (int i = 0; i < iterations; i++) {
        b1 = a1 + i;  b2 = a2 + i;  b3 = a3 + i;  b4 = a4 + i;  b5 = a5 + i;
        b6 = a6 + i;  b7 = a7 + i;  b8 = a8 + i;  b9 = a9 + i;  b10 = a10 + i;
        b11 = a11 + i; b12 = a12 + i; b13 = a13 + i; b14 = a14 + i; b15 = a15 + i;
        
        /* Multiple uses in nested scope */
        {
            int t1 = b1 * b2;
            int t2 = b3 * b4;
            int t3 = b5 * b6;
            int t4 = b7 * b8;
            int t5 = b9 * b10;
            
            a1 = t1 + t2;
            a2 = t3 + t4;
            a3 = t5 + b11;
            a4 = b12 + b13;
            a5 = b14 + b15;
            
            /* Another volatile asm to clobber registers */
            asm volatile (
                "/* Clobber more registers */"
                :
                :
                : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                  "r8", "r9", "r10", "r11", "r12", "r14", "cc", "memory"
            );
        }
        
        /* Switch to create complex control flow */
        switch (i % 4) {
            case 0: a6 = b1 + b2; break;
            case 1: a7 = b3 + b4; break;
            case 2: a8 = b5 + b6; break;
            case 3: a9 = b7 + b8; break;
        }
    }
}

/* Third test: Function with parameter passing to create copy edges */
__attribute__((noinline))
static int test_ira_param_passing(int p1, int p2, int p3, int p4, int p5,
                                  int p6, int p7, int p8, int p9, int p10) {
    /* Many parameters create initial register pressure */
    int sum = p1 + p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9 + p10;
    
    /* Local variables that interact with parameters */
    int l1 = sum * p1;
    int l2 = sum * p2;
    int l3 = sum * p3;
    int l4 = sum * p4;
    int l5 = sum * p5;
    int l6 = sum * p6;
    int l7 = sum * p7;
    int l8 = sum * p8;
    int l9 = sum * p9;
    int l10 = sum * p10;
    
    /* Loop that uses all variables */
    for (int i = 0; i < 100; i++) {
        p1 = l1 + i;  p2 = l2 + i;  p3 = l3 + i;  p4 = l4 + i;  p5 = l5 + i;
        p6 = l6 + i;  p7 = l7 + i;  p8 = l8 + i;  p9 = l9 + i;  p10 = l10 + i;
        
        l1 = p1 * p2;  l2 = p3 * p4;  l3 = p5 * p6;  l4 = p7 * p8;  l5 = p9 * p10;
        l6 = p1 + p3;  l7 = p2 + p4;  l8 = p5 + p7;  l9 = p6 + p8;  l10 = p9 + p10;
        
        /* Memory barrier to force spills */
        asm volatile ("" ::: "memory");
    }
    
    return l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8 + l9 + l10;
}

/* Main function that calls test cases with different parameters
   to explore different conflict graph configurations */
int main(int argc, char **argv) {
    int iterations = 100;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Call first test function - many variables, nested loops */
    test_ira_conflict(iterations);
    
    /* Call second test function - different variable count */
    test_ira_conflict_2(iterations / 2);
    
    /* Call third test function - parameter passing stress */
    int result = test_ira_param_passing(
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10
    );
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}
