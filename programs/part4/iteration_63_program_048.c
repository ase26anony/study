/* test_mcf_coverage.c
 * 
 * This program is designed to trigger the uncovered lines in GCC's
 * min-cost flow solver (mcf.cc) by creating register allocation
 * scenarios that require fixup graph transformations.
 * 
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 * 
 * The -DMCF_DEBUG flag is crucial to enable debug dumping code paths.
 */

#include <stdio.h>
#include <stdlib.h>

/* Force many overlapping live ranges to create complex conflict graph */
#define FORCE_REGISTER_PRESSURE 1

/* Function with many overlapping live variables to create register pressure */
int test_ira_conflict(int iterations) {
    /* Declare many integer variables that will have overlapping live ranges */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int result = 0;
    
    /* Initialize all variables with different values */
    a = 1; b = 2; c = 3; d = 4; e = 5;
    f = 6; g = 7; h = 8; i = 9; j = 10;
    k = 11; l = 12; m = 13; n = 14; o = 15;
    p = 16; q = 17; r = 18; s = 19; t = 20;
    
    /* Nested loops to create complex liveness intervals */
    for (int outer = 0; outer < iterations; outer++) {
        /* Many variables live across the inner loop */
        int temp1 = a + b;
        int temp2 = c + d;
        
        for (int inner = 0; inner < 100; inner++) {
            /* Force all variables to be used in computation */
            a = b + c + inner;
            b = c + d + inner;
            c = d + e + inner;
            d = e + f + inner;
            e = f + g + inner;
            f = g + h + inner;
            g = h + i + inner;
            h = i + j + inner;
            i = j + k + inner;
            j = k + l + inner;
            k = l + m + inner;
            l = m + n + inner;
            m = n + o + inner;
            n = o + p + inner;
            o = p + q + inner;
            p = q + r + inner;
            q = r + s + inner;
            r = s + t + inner;
            s = t + a + inner;
            t = a + b + inner;
            
            /* Use volatile asm to clobber registers and increase pressure */
            asm volatile (
                "nop\n\t"
                "nop\n\t"
                "nop\n\t"
                : 
                : 
                : "memory"
            );
        }
        
        /* More computations to extend live ranges */
        result += a + b + c + d + e + f + g + h + i + j +
                  k + l + m + n + o + p + q + r + s + t;
    }
    
    /* Final computation using all variables */
    return result + a + b + c + d + e + f + g + h + i + j +
                   k + l + m + n + o + p + q + r + s + t;
}

/* Another function with different register pressure pattern */
int test_ira_conflict2(int seed) {
    /* Variables with complex dependency chain */
    int v1 = seed;
    int v2 = v1 * 2;
    int v3 = v2 + v1;
    int v4 = v3 * v2;
    int v5 = v4 - v3;
    int v6 = v5 / (v1 + 1);
    int v7 = v6 | v5;
    int v8 = v7 & v6;
    int v9 = v8 ^ v7;
    int v10 = v9 << 2;
    int v11 = v10 >> 1;
    int v12 = v11 % 7;
    int v13 = v12 * 3;
    int v14 = v13 + 11;
    int v15 = v14 - 5;
    
    /* Loop with all variables live */
    for (int i = 0; i < 50; i++) {
        /* Rotate values through variables */
        int tmp = v1;
        v1 = v2; v2 = v3; v3 = v4; v4 = v5; v5 = v6;
        v6 = v7; v7 = v8; v8 = v9; v9 = v10; v10 = v11;
        v11 = v12; v12 = v13; v13 = v14; v14 = v15; v15 = tmp;
        
        /* Inline asm that clobbers many registers */
        asm volatile (
            "mov r0, r0\n\t"
            "mov r1, r1\n\t"
            "mov r2, r2\n\t"
            : 
            : 
            : "r0", "r1", "r2", "memory"
        );
    }
    
    /* Force all variables to contribute to result */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15;
}

/* Function with switch statement to create control flow complexity */
int test_ira_conflict3(int mode) {
    int x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5;
    int x6 = 6, x7 = 7, x8 = 8, x9 = 9, x10 = 10;
    
    switch (mode % 5) {
        case 0:
            x1 = x2 + x3;
            x2 = x4 * x5;
            /* fall through */
        case 1:
            x3 = x6 - x7;
            x4 = x8 / (x9 + 1);
            break;
        case 2:
            x5 = x10 & x1;
            x6 = x2 | x3;
            /* fall through */
        case 3:
            x7 = x4 ^ x5;
            x8 = x6 << 1;
            break;
        case 4:
            x9 = x7 >> 2;
            x10 = x8 % 3;
            break;
    }
    
    /* All variables live at this point */
    asm volatile ("" : : : "memory");
    
    return x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9 + x10;
}

/* Function that creates imbalance in register usage */
void test_ira_imbalance(void) {
    /* Create many short-lived temporaries in expression */
    int sum = 0;
    
    for (int i = 0; i < 100; i++) {
        /* Complex expression with many intermediate values */
        sum += (((i * 2) + 3) * 4 - 5) / 6 + 
               ((i % 7) << 2) | 0xFF +
               (i & 0xF) ^ 0xA;
    }
    
    /* Force spilling */
    volatile int sink = sum;
    (void)sink;
}

/* Main function to exercise all test cases */
int main(int argc, char **argv) {
    int result = 0;
    
    /* Call test functions with different parameters to explore
       different register allocation scenarios */
    result += test_ira_conflict(argc > 1 ? atoi(argv[1]) : 10);
    result += test_ira_conflict2(result);
    
    for (int i = 0; i < 20; i++) {
        result += test_ira_conflict3(i);
    }
    
    test_ira_imbalance();
    
    printf("Result: %d\n", result);
    return result & 0xFF;  /* Return non-zero to indicate execution */
}
