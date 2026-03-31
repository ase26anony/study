/* test_mcf_coverage.c
 * 
 * This test is designed to trigger the uncovered lines in GCC's min-cost flow
 * solver (mcf.cc) by creating register allocation scenarios that require
 * complex fixup graph transformations.
 * 
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 * Or with: gcc -O3 -funroll-loops -fira-algorithm=CB -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 */

/* Force inclusion of IRA and MCF debugging */
#ifdef MCF_DEBUG
/* This ensures debug paths are compiled in */
#endif

#include <stdio.h>
#include <stdlib.h>

/* Function with many overlapping live ranges to create complex conflict graph */
__attribute__((noinline))
static int test_ira_conflict(int iterations) {
    /* Declare many variables that will have overlapping live ranges */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int result = 0;
    
    /* Initialize all variables with different values to prevent optimization */
    a = iterations * 1;
    b = iterations * 2;
    c = iterations * 3;
    d = iterations * 4;
    e = iterations * 5;
    f = iterations * 6;
    g = iterations * 7;
    h = iterations * 8;
    i = iterations * 9;
    j = iterations * 10;
    k = iterations * 11;
    l = iterations * 12;
    m = iterations * 13;
    n = iterations * 14;
    o = iterations * 15;
    p = iterations * 16;
    q = iterations * 17;
    r = iterations * 18;
    s = iterations * 19;
    t = iterations * 20;
    
    /* Nested loops to create complex liveness patterns */
    for (int outer = 0; outer < iterations; outer++) {
        /* Many variables live across inner loop */
        int temp1 = a + b + c + d;
        int temp2 = e + f + g + h;
        
        /* Volatile asm to clobber registers and increase pressure */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            : 
            : 
            : "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
              "r8", "r9", "r10", "r11", "r12"
        );
        
        for (int inner = 0; inner < 10; inner++) {
            /* Complex computation keeping many variables live */
            a = b + c - d;
            b = c + d - e;
            c = d + e - f;
            d = e + f - g;
            e = f + g - h;
            f = g + h - i;
            g = h + i - j;
            h = i + j - k;
            
            /* More volatile asm to force register spills */
            asm volatile (
                "nop\n\t"
                "nop\n\t"
                : 
                : 
                : "memory"
            );
            
            /* Use remaining variables */
            result += i + j + k + l + m + n + o + p + q + r + s + t;
            
            /* Rotate values to create data dependencies */
            int tmp = a;
            a = b; b = c; c = d; d = e; e = f; f = g; g = h; h = tmp;
        }
        
        /* Use all variables in complex expressions */
        result += (a * b) / (c + 1);
        result += (d * e) / (f + 1);
        result += (g * h) / (i + 1);
        result += (j * k) / (l + 1);
        result += (m * n) / (o + 1);
        result += (p * q) / (r + 1);
        result += (s * t) / (a + 1);
    }
    
    /* Final complex expression using all variables */
    return result + a + b + c + d + e + f + g + h + i + j + 
           k + l + m + n + o + p + q + r + s + t;
}

/* Second test function with different register pressure pattern */
__attribute__((noinline))
static int test_spill_scenario(int seed) {
    /* Variables that will be heavily used and spilled */
    int v1 = seed * 2;
    int v2 = seed * 3;
    int v3 = seed * 5;
    int v4 = seed * 7;
    int v5 = seed * 11;
    int v6 = seed * 13;
    int v7 = seed * 17;
    int v8 = seed * 19;
    int v9 = seed * 23;
    int v10 = seed * 29;
    
    /* Large switch to create many basic blocks with different live ranges */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        switch (i % 10) {
            case 0: sum += v1 + v2; v1++; break;
            case 1: sum += v3 + v4; v2++; break;
            case 2: sum += v5 + v6; v3++; break;
            case 3: sum += v7 + v8; v4++; break;
            case 4: sum += v9 + v10; v5++; break;
            case 5: sum += v1 * v2; v6++; break;
            case 6: sum += v3 * v4; v7++; break;
            case 7: sum += v5 * v6; v8++; break;
            case 8: sum += v7 * v8; v9++; break;
            case 9: sum += v9 * v10; v10++; break;
        }
        
        /* Force register pressure with inline asm */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            : 
            : 
            : "memory", "r0", "r1", "r2", "r3", "r4"
        );
    }
    
    return sum;
}

/* Third test: Function with parameter passing to create copy edges */
__attribute__((noinline))
static int test_param_passing(int p1, int p2, int p3, int p4, int p5,
                              int p6, int p7, int p8, int p9, int p10) {
    /* Use all parameters in complex ways */
    int sum = p1 + p2 + p3 + p4 + p5;
    
    for (int i = 0; i < 50; i++) {
        /* Create many temporary values */
        int t1 = p1 * p2;
        int t2 = p3 * p4;
        int t3 = p5 * p6;
        int t4 = p7 * p8;
        int t5 = p9 * p10;
        
        sum += t1 + t2 + t3 + t4 + t5;
        
        /* Rotate parameters */
        int tmp = p1;
        p1 = p2; p2 = p3; p3 = p4; p4 = p5; p5 = p6;
        p6 = p7; p7 = p8; p8 = p9; p9 = p10; p10 = tmp;
        
        /* Force spills with large clobber list */
        asm volatile (
            "nop\n\t"
            : 
            : 
            : "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
              "r8", "r9", "r10", "r11", "r12", "r14"
        );
    }
    
    return sum;
}

/* Main function that exercises all test scenarios */
int main(int argc, char **argv) {
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    int result = 0;
    
    printf("Starting MCF coverage test...\n");
    
    /* Test 1: Complex conflict graph with many live ranges */
    result += test_ira_conflict(iterations);
    
    /* Test 2: Spill-heavy scenario */
    result += test_spill_scenario(iterations);
    
    /* Test 3: Parameter passing stress test */
    result += test_param_passing(
        iterations, iterations+1, iterations+2, iterations+3, iterations+4,
        iterations+5, iterations+6, iterations+7, iterations+8, iterations+9
    );
    
    /* Additional loop to increase compilation complexity */
    for (int i = 0; i < 10; i++) {
        result += test_ira_conflict(i);
    }
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
