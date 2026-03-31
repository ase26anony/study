/* test_mcf_coverage.c
 * 
 * This test program is designed to stress GCC's Min-Cost Flow register allocator
 * to trigger coverage of special node indices in the print_node function.
 * 
 * Compile with: gcc -O3 -fprofile-arcs -ftest-coverage -fdump-rtl-all test_mcf_coverage.c -o test_mcf
 * Run with: ./test_mcf
 * Generate coverage: gcov test_mcf_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure each function is compiled independently */
#define NOINLINE __attribute__((noinline))
#define COLD __attribute__((cold))

/* Pattern A: Integer arithmetic chain with 30+ variables in a loop */
NOINLINE static long pattern_a(int iterations, int seed) {
    /* 30+ integer variables to create register pressure */
    int v1 = seed + 1;
    int v2 = v1 * 2;
    int v3 = v2 + v1;
    int v4 = v3 - v2;
    int v5 = v4 * v3;
    int v6 = v5 / (v1 ? v1 : 1);
    int v7 = v6 ^ v5;
    int v8 = v7 | v6;
    int v9 = v8 & v7;
    int v10 = v9 << 2;
    int v11 = v10 >> 1;
    int v12 = v11 + v10;
    int v13 = v12 - v11;
    int v14 = v13 * v12;
    int v15 = v14 % (v13 ? v13 : 1);
    int v16 = v15 ^ v14;
    int v17 = v16 | v15;
    int v18 = v17 & v16;
    int v19 = v18 << 3;
    int v20 = v19 >> 2;
    int v21 = v20 + v19;
    int v22 = v21 - v20;
    int v23 = v22 * v21;
    int v24 = v23 % (v22 ? v22 : 1);
    int v25 = v24 ^ v23;
    int v26 = v25 | v24;
    int v27 = v26 & v25;
    int v28 = v27 << 1;
    int v29 = v28 >> 1;
    int v30 = v29 + v28;
    
    /* Complex loop with interdependencies */
    long sum = 0;
    for (int i = 0; i < iterations; i++) {
        /* Create complex data flow to force MCF analysis */
        v1 = (v30 + i) & 0xFF;
        v2 = v1 ^ v30;
        v3 = v2 + v1;
        v4 = v3 - v2;
        v5 = v4 * v3;
        v6 = v5 ^ v4;
        v7 = v6 | v5;
        v8 = v7 & v6;
        v9 = v8 + v7;
        v10 = v9 - v8;
        v11 = v10 * v9;
        v12 = v11 ^ v10;
        v13 = v12 | v11;
        v14 = v13 & v12;
        v15 = v14 + v13;
        v16 = v15 - v14;
        v17 = v16 * v15;
        v18 = v17 ^ v16;
        v19 = v18 | v17;
        v20 = v19 & v18;
        v21 = v20 + v19;
        v22 = v21 - v20;
        v23 = v22 * v21;
        v24 = v23 ^ v22;
        v25 = v24 | v23;
        v26 = v25 & v24;
        v27 = v26 + v25;
        v28 = v27 - v26;
        v29 = v28 * v27;
        v30 = v29 ^ v28;
        
        sum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
               v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
               v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
        
        /* Complex control flow with break/continue */
        if (i % 7 == 0) continue;
        if (i % 13 == 0) {
            v1 = v30;
            continue;
        }
        if (i % 17 == 0) {
            v2 = v29;
            break;
        }
    }
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4), "r"(v5),
                     "r"(v6), "r"(v7), "r"(v8), "r"(v9), "r"(v10),
                     "r"(v11), "r"(v12), "r"(v13), "r"(v14), "r"(v15),
                     "r"(v16), "r"(v17), "r"(v18), "r"(v19), "r"(v20),
                     "r"(v21), "r"(v22), "r"(v23), "r"(v24), "r"(v25),
                     "r"(v26), "r"(v27), "r"(v28), "r"(v29), "r"(v30));
    
    return sum;
}

/* Pattern B: Floating-point intensive computation */
NOINLINE static double pattern_b(int iterations, double seed) {
    /* 20+ double variables to pressure FP registers */
    double d1 = seed + 1.0;
    double d2 = d1 * 1.5;
    double d3 = d2 + d1;
    double d4 = d3 - d2;
    double d5 = d4 * d3;
    double d6 = d5 / (d4 != 0.0 ? d4 : 1.0);
    double d7 = d6 + d5;
    double d8 = d7 - d6;
    double d9 = d8 * d7;
    double d10 = d9 / (d8 != 0.0 ? d8 : 1.0);
    double d11 = d10 + d9;
    double d12 = d11 - d10;
    double d13 = d12 * d11;
    double d14 = d13 / (d12 != 0.0 ? d12 : 1.0);
    double d15 = d14 + d13;
    double d16 = d15 - d14;
    double d17 = d16 * d15;
    double d18 = d17 / (d16 != 0.0 ? d16 : 1.0);
    double d19 = d18 + d17;
    double d20 = d19 - d18;
    
    double sum = 0.0;
    for (int i = 0; i < iterations; i++) {
        /* Complex FP operations */
        d1 = d20 * 0.99 + i;
        d2 = d1 * 1.01;
        d3 = d2 + d1;
        d4 = d3 - d2;
        d5 = d4 * d3;
        d6 = d5 / (d4 != 0.0 ? d4 : 1.0);
        d7 = d6 + d5;
        d8 = d7 - d6;
        d9 = d8 * d7;
        d10 = d9 / (d8 != 0.0 ? d8 : 1.0);
        d11 = d10 + d9;
        d12 = d11 - d10;
        d13 = d12 * d11;
        d14 = d13 / (d12 != 0.0 ? d12 : 1.0);
        d15 = d14 + d13;
        d16 = d15 - d14;
        d17 = d16 * d15;
        d18 = d17 / (d16 != 0.0 ? d16 : 1.0);
        d19 = d18 + d17;
        d20 = d19 - d18;
        
        sum += d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10 +
               d11 + d12 + d13 + d14 + d15 + d16 + d17 + d18 + d19 + d20;
        
        /* Nested loops with break */
        for (int j = 0; j < 3; j++) {
            if (j == 1) {
                d1 = d20;
                continue;
            }
            if (j == 2 && i % 5 == 0) {
                break;
            }
        }
    }
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(d1), "r"(d2), "r"(d3), "r"(d4), "r"(d5),
                     "r"(d6), "r"(d7), "r"(d8), "r"(d9), "r"(d10),
                     "r"(d11), "r"(d12), "r"(d13), "r"(d14), "r"(d15),
                     "r"(d16), "r"(d17), "r"(d18), "r"(d19), "r"(d20));
    
    return sum;
}

/* Pattern C: Complex control flow with switch statement */
NOINLINE static int pattern_c(int value, int iterations) {
    int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Switch with many cases creates complex CFG */
        switch ((value + i) % 23) {
            case 0:  result += i * 2; break;
            case 1:  result += i ^ 0x55; break;
            case 2:  result += i << 1; break;
            case 3:  result += i >> 1; break;
            case 4:  result += i & 0xF; break;
            case 5:  result += i | 0xA; break;
            case 6:  result += ~i; break;
            case 7:  result += i * 3; break;
            case 8:  result += i / 2; break;
            case 9:  result += i % 7; break;
            case 10: result += i + 0x100; break;
            case 11: result += i - 0x50; break;
            case 12: result += i * i; break;
            case 13: result += -i; break;
            case 14: result += i ^ i; break;
            case 15: result += i & (i - 1); break;
            case 16: result += i | (i << 8); break;
            case 17: result += (i << 4) | (i >> 4); break;
            case 18: result += i * 5; break;
            case 19: result += i / 3; break;
            case 20: result += i % 11; break;
            case 21: result += i + 0x200; break;
            case 22: result += i - 0x100; break;
            default: result += 1; break;
        }
        
        /* Additional variables in each basic block */
        int temp1 = result * 2;
        int temp2 = temp1 + i;
        int temp3 = temp2 ^ result;
        int temp4 = temp3 & 0xFF;
        int temp5 = temp4 | 0x80;
        
        result = temp5;
        
        /* Loop with continue conditions */
        if (i % 4 == 0) continue;
        if (i % 6 == 0) {
            int extra = result * 3;
            result = extra;
            continue;
        }
    }
    
    return result;
}

/* Pattern D: Vector extensions for SIMD register pressure */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

NOINLINE static v4si pattern_d(v4si input, int iterations) {
#ifdef __GNUC__
    v4si v1 = input + (v4si){1, 2, 3, 4};
    v4si v2 = v1 * (v4si){2, 2, 2, 2};
    v4si v3 = v2 + v1;
    v4si v4 = v3 - v2;
    v4si v5 = v4 * v3;
    v4si v6 = v5 + v4;
    v4si v7 = v6 - v5;
    v4si v8 = v7 * v6;
    v4si v9 = v8 + v7;
    v4si v10 = v9 - v8;
    
    v4si result = {0, 0, 0, 0};
    
    for (int i = 0; i < iterations; i++) {
        v1 = v10 + (v4si){i, i+1, i+2, i+3};
        v2 = v1 * (v4si){i%5+1, i%7+1, i%11+1, i%13+1};
        v3 = v2 + v1;
        v4 = v3 - v2;
        v5 = v4 * v3;
        v6 = v5 + v4;
        v7 = v6 - v5;
        v8 = v7 * v6;
        v9 = v8 + v7;
        v10 = v9 - v8;
        
        result = result + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        
        /* Conditional vector operations */
        if (i % 3 == 0) {
            v1 = v10;
        }
        if (i % 5 == 0) {
            v2 = v9;
        }
    }
    
    /* Prevent dead code elimination */
    asm volatile("" : : "x"(v1), "x"(v2), "x"(v3), "x"(v4), "x"(v5),
                     "x"(v6), "x"(v7), "x"(v8), "x"(v9), "x"(v10));
    
    return result;
#else
    return input;
#endif
}

/* Pattern E: Explicit register variables to conflict with allocator */
NOINLINE static int pattern_e(int iterations) {
    /* Explicit register variables that may conflict with allocator's choices */
    register int r1 asm ("r12") = 1;
    register int r2 asm ("r13") = 2;
    register int r3 asm ("r14") = 3;
    register int r4 asm ("r15") = 4;
    
    int v1 = r1, v2 = r2, v3 = r3, v4 = r4;
    int v5 = v1 + v2;
    int v6 = v3 + v4;
    int v7 = v5 * v6;
    int v8 = v7 - v5;
    int v9 = v8 / (v6 ? v6 : 1);
    int v10 = v9 ^ v8;
    
    int sum = 0;
    for (int i = 0; i < iterations; i++) {
        /* Mix explicit register vars with regular vars */
        r1 = v10 + i;
        r2 = r1 * 2;
        r3 = r2 + r1;
        r4 = r3 - r2;
        
        v1 = r4;
        v2 = v1 * r3;
        v3 = v2 + v1;
        v4 = v3 - v2;
        v5 = v4 * v3;
        v6 = v5 / (v4 ? v4 : 1);
        v7 = v6 ^ v5;
        v8 = v7 | v6;
        v9 = v8 & v7;
        v10 = v9 << 1;
        
        sum += r1 + r2 + r3 + r4 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        
        /* Computed goto (GCC extension) for complex control flow */
        void *labels[] = {&&L0, &&L1, &&L2, &&L3, &&L4};
        goto *labels[i % 5];
        
        L0: r1 += 1; continue;
        L1: r2 += 2; continue;
        L2: r3 += 3; continue;
        L3: r4 += 4; continue;
        L4: v10 += 5; continue;
    }
    
    /* Force use of all variables */
    asm volatile("" : : "r"(r1), "r"(r2), "r"(r3), "r"(r4),
                     "r"(v1), "r"(v2), "r"(v3), "r"(v4),
                     "r"(v5), "r"(v6), "r"(v7), "r"(v8),
                     "r"(v9), "r"(v10));
    
    return sum;
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
    
    /* Call Pattern A multiple times with different seeds */
    for (int i = 0; i < 5; i++) {
        total += pattern_a(iterations / 5, i * 12345);
    }
    
    /* Call Pattern B */
    double fp_result = pattern_b(iterations / 10, 3.14159);
    total += (long)fp_result;
    
    /* Call Pattern C with different values */
    for (int i = 0; i < 7; i++) {
        total += pattern_c(i * 17, iterations / 20);
    }
    
    /* Call Pattern D if supported */
#ifdef __GNUC__
    v4si vec_input = {1, 2, 3, 4};
    v4si vec_result = pattern_d(vec_input, iterations / 50);
    total += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
#endif
    
    /* Call Pattern E */
    total += pattern_e(iterations / 25);
    
    /* Use __builtin_cpu_supports to engage target-specific optimizations */
#ifdef __GNUC__
    if (__builtin_cpu_supports("avx2")) {
        total += 0x1000;
    }
    if (__builtin_cpu_supports("sse4.2")) {
        total += 0x2000;
    }
#endif
    
    /* Print result to prevent optimization */
    printf("Result: %ld\n", total);
    
    return 0;
}
