/* test_mcf_coverage.c
 * 
 * This test program is designed to trigger GCC's Min-Cost Flow pass
 * during register allocation, specifically to cover the special node
 * printing logic in mcf.cc's print_node function.
 * 
 * Compile with: gcc -O3 -fprofile-arcs -ftest-coverage -fdump-rtl-all test_mcf_coverage.c -o test_mcf
 * Run with: ./test_mcf
 * Generate coverage: gcov test_mcf_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units in same file */
#define NOINLINE __attribute__((noinline))
#define COLD __attribute__((cold))

/* Pattern A: Integer arithmetic chain with 30+ variables in a loop */
NOINLINE static long pattern_a(int iterations, int seed) {
    /* 30+ integer variables to create register pressure */
    int v1 = seed + 1;
    int v2 = v1 * 2;
    int v3 = v2 - seed;
    int v4 = v3 * v1;
    int v5 = v4 / (seed ? seed : 1);
    int v6 = v5 ^ v2;
    int v7 = v6 | v3;
    int v8 = v7 & v4;
    int v9 = v8 << 2;
    int v10 = v9 >> 1;
    int v11 = v10 + v1;
    int v12 = v11 - v2;
    int v13 = v12 * v3;
    int v14 = v13 / (v4 ? v4 : 1);
    int v15 = v14 ^ v5;
    int v16 = v15 | v6;
    int v17 = v16 & v7;
    int v18 = v17 << 3;
    int v19 = v18 >> 2;
    int v20 = v19 + v8;
    int v21 = v20 - v9;
    int v22 = v21 * v10;
    int v23 = v22 / (v11 ? v11 : 1);
    int v24 = v23 ^ v12;
    int v25 = v24 | v13;
    int v26 = v25 & v14;
    int v27 = v26 << 1;
    int v28 = v27 >> 1;
    int v29 = v28 + v15;
    int v30 = v29 - v16;
    int v31 = v30 * v17;
    int v32 = v31 / (v18 ? v18 : 1);
    int v33 = v32 ^ v19;
    int v34 = v33 | v20;
    int v35 = v34 & v21;
    
    long result = 0;
    for (int i = 0; i < iterations; i++) {
        /* Interdependent operations to prevent optimization */
        v1 = (v1 + v2) ^ v35;
        v2 = (v2 - v3) | v34;
        v3 = (v3 * v4) & v33;
        v4 = (v4 ^ v5) + v32;
        v5 = (v5 | v6) - v31;
        v6 = (v6 & v7) * v30;
        v7 = (v7 << 1) / (v29 ? v29 : 1);
        v8 = (v8 >> 2) ^ v28;
        v9 = (v9 + v10) | v27;
        v10 = (v10 - v11) & v26;
        v11 = (v11 * v12) + v25;
        v12 = (v12 / (v13 ? v13 : 1)) - v24;
        v13 = (v13 ^ v14) * v23;
        v14 = (v14 | v15) / (v22 ? v22 : 1);
        v15 = (v15 & v16) ^ v21;
        
        /* Use asm to prevent dead code elimination */
        asm volatile("" : "+r"(v1), "+r"(v2), "+r"(v3), "+r"(v4), "+r"(v5));
        asm volatile("" : "+r"(v6), "+r"(v7), "+r"(v8), "+r"(v9), "+r"(v10));
        
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    }
    
    return result + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
           v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30 +
           v31 + v32 + v33 + v34 + v35;
}

/* Pattern B: Floating-point intensive computation */
NOINLINE static double pattern_b(int iterations, double seed) {
    /* 20+ double variables to pressure FP registers */
    double d1 = seed + 1.0;
    double d2 = d1 * 1.5;
    double d3 = d2 - 0.5;
    double d4 = d3 * d1;
    double d5 = d4 / (d2 + 0.001);
    double d6 = d5 + d3;
    double d7 = d6 - d4;
    double d8 = d7 * d5;
    double d9 = d8 / (d6 + 0.001);
    double d10 = d9 + d7;
    double d11 = d10 - d8;
    double d12 = d11 * d9;
    double d13 = d12 / (d10 + 0.001);
    double d14 = d13 + d11;
    double d15 = d14 - d12;
    double d16 = d15 * d13;
    double d17 = d16 / (d14 + 0.001);
    double d18 = d17 + d15;
    double d19 = d18 - d16;
    double d20 = d19 * d17;
    
    /* Complex loop with trigonometric functions */
    for (int i = 0; i < iterations; i++) {
        d1 = d1 * 0.99 + d20 * 0.01;
        d2 = d2 * 0.98 + d19 * 0.02;
        d3 = d3 * 0.97 + d18 * 0.03;
        d4 = d4 * 0.96 + d17 * 0.04;
        d5 = d5 * 0.95 + d16 * 0.05;
        
        /* Use all variables in computation */
        d6 = (d1 + d2 + d3 + d4 + d5) / 5.0;
        d7 = (d6 * d1 - d2) / (d3 + 0.001);
        d8 = (d7 * d4 + d5) / (d6 + 0.001);
        d9 = (d8 * d2 - d3) / (d4 + 0.001);
        d10 = (d9 * d5 + d1) / (d2 + 0.001);
        
        /* Prevent optimization */
        asm volatile("" : "+x"(d1), "+x"(d2), "+x"(d3), "+x"(d4), "+x"(d5));
    }
    
    return d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10 +
           d11 + d12 + d13 + d14 + d15 + d16 + d17 + d18 + d19 + d20;
}

/* Pattern C: Complex control flow with switch statement */
NOINLINE static int pattern_c(int value, int iterations) {
    int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Switch with many cases creates complex CFG */
        switch ((value + i) % 23) {
            case 0:  result += value * 1; break;
            case 1:  result += value * 2; break;
            case 2:  result += value * 3; break;
            case 3:  result += value * 4; break;
            case 4:  result += value * 5; break;
            case 5:  result += value * 6; break;
            case 6:  result += value * 7; break;
            case 7:  result += value * 8; break;
            case 8:  result += value * 9; break;
            case 9:  result += value * 10; break;
            case 10: result += value * 11; break;
            case 11: result += value * 12; break;
            case 12: result += value * 13; break;
            case 13: result += value * 14; break;
            case 14: result += value * 15; break;
            case 15: result += value * 16; break;
            case 16: result += value * 17; break;
            case 17: result += value * 18; break;
            case 18: result += value * 19; break;
            case 19: result += value * 20; break;
            case 20: result += value * 21; break;
            case 21: result += value * 22; break;
            case 22: result += value * 23; break;
        }
        
        /* Nested loops with break/continue */
        for (int j = 0; j < 5; j++) {
            if (j == 2) continue;
            for (int k = 0; k < 3; k++) {
                if (k == 1) break;
                result += j * k;
            }
        }
    }
    
    return result;
}

/* Pattern D: Vector extensions for SIMD register pressure */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

NOINLINE static long pattern_d(int iterations) {
#ifdef __GNUC__
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = {9, 10, 11, 12};
    v4si vec4 = {13, 14, 15, 16};
    v4si vec5 = {17, 18, 19, 20};
    v4si vec6 = {21, 22, 23, 24};
    v4si vec7 = {25, 26, 27, 28};
    v4si vec8 = {29, 30, 31, 32};
    
    v4si result_vec = {0, 0, 0, 0};
    
    for (int i = 0; i < iterations; i++) {
        vec1 = vec1 + vec2;
        vec2 = vec2 * vec3;
        vec3 = vec3 - vec4;
        vec4 = vec4 & vec5;
        vec5 = vec5 | vec6;
        vec6 = vec6 ^ vec7;
        vec7 = vec7 + vec8;
        vec8 = vec8 * vec1;
        
        result_vec = result_vec + vec1 + vec2 + vec3 + vec4 + 
                    vec5 + vec6 + vec7 + vec8;
        
        /* Prevent optimization */
        asm volatile("" : "+x"(vec1), "+x"(vec2), "+x"(vec3), "+x"(vec4));
    }
    
    long result = result_vec[0] + result_vec[1] + result_vec[2] + result_vec[3];
    return result;
#else
    return iterations;
#endif
}

/* Pattern E: Explicit register variables to conflict with allocator */
NOINLINE static int pattern_e(int iterations) {
    /* Try to use specific registers */
    register int r1 asm ("r12") = 1;
    register int r2 asm ("r13") = 2;
    register int r3 asm ("r14") = 3;
    register int r4 asm ("r15") = 4;
    
    int var1 = 10, var2 = 20, var3 = 30, var4 = 40;
    int var5 = 50, var6 = 60, var7 = 70, var8 = 80;
    int var9 = 90, var10 = 100, var11 = 110, var12 = 120;
    
    for (int i = 0; i < iterations; i++) {
        /* Mix register variables with regular ones */
        r1 = r1 + var1 + i;
        r2 = r2 * var2 - i;
        r3 = r3 ^ var3 | i;
        r4 = r4 & var4 + i;
        
        var1 = var1 + r1;
        var2 = var2 - r2;
        var3 = var3 * r3;
        var4 = var4 / (r4 ? r4 : 1);
        
        /* Use all variables to keep them alive */
        var5 = var1 + var2;
        var6 = var3 - var4;
        var7 = var5 * var6;
        var8 = var7 / (var5 ? var5 : 1);
        var9 = var8 ^ var1;
        var10 = var9 | var2;
        var11 = var10 & var3;
        var12 = var11 << 2;
        
        /* Force register spills */
        asm volatile("" : "+r"(r1), "+r"(r2), "+r"(r3), "+r"(r4));
        asm volatile("" : "+r"(var1), "+r"(var2), "+r"(var3), "+r"(var4));
    }
    
    return r1 + r2 + r3 + r4 + var1 + var2 + var3 + var4 + 
           var5 + var6 + var7 + var8 + var9 + var10 + var11 + var12;
}

/* Main function that calls all patterns */
COLD int main(int argc, char **argv) {
    int iterations = 1000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 100) iterations = 100;
        if (iterations > 10000) iterations = 10000;
    }
    
    long total = 0;
    
    /* Call each pattern multiple times with different inputs */
    for (int i = 0; i < 5; i++) {
        total += pattern_a(iterations / 10, i * 7);
        total += (long)pattern_b(iterations / 20, i * 3.14);
        total += pattern_c(i * 11, iterations / 50);
        total += pattern_d(iterations / 30);
        total += pattern_e(iterations / 40);
    }
    
    /* Use CPU feature detection to engage target-specific optimizations */
#ifdef __GNUC__
    if (__builtin_cpu_supports("avx2")) {
        total += 1000;
    }
    if (__builtin_cpu_supports("sse4.2")) {
        total += 500;
    }
#endif
    
    /* Print result to prevent complete optimization */
    printf("Result: %ld\n", total);
    
    return (total > 0) ? 0 : 1;
}
