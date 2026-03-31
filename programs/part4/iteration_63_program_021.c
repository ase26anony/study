/* test_mcf_coverage.c
 * 
 * This program is designed to trigger the uncovered lines in GCC's
 * Min-Cost Flow solver (mcf.cc) by creating register allocation
 * scenarios that require complex fixup graph construction.
 * 
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 * 
 * The -DMCF_DEBUG flag is crucial to enable the debug dumping code
 * that contains the target block in dump_fixup_edge.
 */

#include <stdio.h>
#include <stdlib.h>

/* Force many overlapping live ranges with complex control flow */
__attribute__((noinline))
static void test_ira_conflict_high_pressure(void) {
    /* Declare many variables that will have overlapping live ranges */
    volatile int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    
    /* Initialize with different values to prevent constant propagation */
    a = 1; b = 2; c = 3; d = 4; e = 5; f = 6; g = 7; h = 8; i = 9; j = 10;
    k = 11; l = 12; m = 13; n = 14; o = 15; p = 16; q = 17; r = 18; s = 19; t = 20;
    
    /* Nested loops with many live variables across iterations */
    for (int outer = 0; outer < 100; outer++) {
        /* All variables are live here */
        int temp1 = a + b + c + d;
        int temp2 = e + f + g + h;
        
        /* Inner loop with more live variables */
        for (int inner = 0; inner < 50; inner++) {
            /* Complex computation keeping many variables live */
            int sum1 = i + j + k + l;
            int sum2 = m + n + o + p;
            int sum3 = q + r + s + t;
            
            /* Use volatile asm to clobber registers and increase pressure */
            asm volatile (
                "/* Clobber many registers */"
                :
                : "r"(sum1), "r"(sum2), "r"(sum3), "r"(temp1), "r"(temp2)
                : "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                  "r8", "r9", "r10", "r11", "r12", "r14"
            );
            
            /* Modify variables to prevent dead code elimination */
            a += inner;
            b += sum1;
            c += sum2;
            d += sum3;
        }
        
        /* More computations with overlapping live ranges */
        e = temp1 * 2;
        f = temp2 * 3;
        g = a + b + c;
        h = d + e + f;
        
        /* Another asm statement that uses many registers */
        asm volatile (
            "/* Use many input registers */"
            : "=r"(i), "=r"(j), "=r"(k), "=r"(l)
            : "r"(g), "r"(h), "r"(e), "r"(f), "0"(i), "1"(j), "2"(k), "3"(l)
            : "cc"
        );
    }
    
    /* Final use to prevent optimization */
    volatile int result = a + b + c + d + e + f + g + h + i + j + 
                         k + l + m + n + o + p + q + r + s + t;
    (void)result;
}

/* Alternative test with different variable count to affect graph size */
__attribute__((noinline))
static void test_ira_conflict_medium_pressure(void) {
    /* Different number of variables to potentially create different
     * fixup graph sizes and indices */
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    volatile int w1, w2, w3, w4, w5;
    
    v1 = 1; v2 = 2; v3 = 3; v4 = 4; v5 = 5;
    v6 = 6; v7 = 7; v8 = 8; v9 = 9; v10 = 10;
    w1 = 11; w2 = 12; w3 = 13; w4 = 14; w5 = 15;
    
    /* Complex control flow with conditionals */
    for (int i = 0; i < 1000; i++) {
        if (i % 3 == 0) {
            v1 = v2 + v3;
            v4 = v5 * v6;
            w1 = w2 - w3;
            
            asm volatile (
                "/* Medium pressure asm */"
                : "+r"(v1), "+r"(v4), "+r"(w1)
                : "r"(v2), "r"(v3), "r"(v5), "r"(v6), "r"(w2), "r"(w3)
                : "cc", "memory"
            );
        } else if (i % 3 == 1) {
            v7 = v8 + v9;
            v10 = w4 * w5;
            
            asm volatile (
                "/* Different register usage pattern */"
                : "=r"(v7), "=r"(v10)
                : "r"(v8), "r"(v9), "r"(w4), "r"(w5), "0"(v7), "1"(v10)
                : "cc"
            );
        } else {
            /* All variables live in this branch */
            int temp = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                      w1 + w2 + w3 + w4 + w5;
            
            asm volatile (
                "/* Use result */"
                : 
                : "r"(temp)
                : "memory"
            );
        }
    }
}

/* Test with artificial register pressure via many function arguments */
__attribute__((noinline))
static int test_many_args(int a1, int a2, int a3, int a4, int a5,
                          int a6, int a7, int a8, int a9, int a10,
                          int a11, int a12, int a13, int a14, int a15) {
    /* All arguments are live at function entry */
    volatile int sum = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
                      a11 + a12 + a13 + a14 + a15;
    
    /* Create more local variables */
    int b1 = sum * 2;
    int b2 = sum * 3;
    int b3 = sum * 4;
    int b4 = sum * 5;
    int b5 = sum * 6;
    
    /* Loop with all variables live */
    for (int i = 0; i < 100; i++) {
        b1 += a1 + i;
        b2 += a2 + i * 2;
        b3 += a3 + i * 3;
        b4 += a4 + i * 4;
        b5 += a5 + i * 5;
        
        /* Force spilling with large asm clobber list */
        asm volatile (
            "/* Clobber everything */"
            : "+r"(b1), "+r"(b2), "+r"(b3), "+r"(b4), "+r"(b5)
            : "r"(a6), "r"(a7), "r"(a8), "r"(a9), "r"(a10),
              "r"(a11), "r"(a12), "r"(a13), "r"(a14), "r"(a15)
            : "cc", "memory",
              "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
              "r8", "r9", "r10", "r11", "r12", "r14", "r15"
        );
    }
    
    return b1 + b2 + b3 + b4 + b5;
}

/* Main function that exercises different scenarios */
int main(void) {
    printf("Testing IRA conflict scenarios to trigger MCF debug output...\n");
    
    /* Call different test functions to explore various graph configurations */
    for (int iteration = 0; iteration < 10; iteration++) {
        test_ira_conflict_high_pressure();
        test_ira_conflict_medium_pressure();
        
        /* Test with many arguments */
        int result = test_many_args(
            iteration, iteration+1, iteration+2, iteration+3, iteration+4,
            iteration+5, iteration+6, iteration+7, iteration+8, iteration+9,
            iteration+10, iteration+11, iteration+12, iteration+13, iteration+14
        );
        
        /* Use result to prevent dead code elimination */
        volatile int dummy = result;
        (void)dummy;
    }
    
    printf("Test completed.\n");
    return 0;
}
