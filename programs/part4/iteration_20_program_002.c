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
#define NORETURN __attribute__((noreturn))
#define COLD __attribute__((cold))

/* Pattern A: Integer arithmetic chain with 30+ variables in a loop */
NOINLINE static long pattern_a(int iterations, int seed) {
    /* 30+ integer variables to create register pressure */
    int v1 = seed + 1;
    int v2 = v1 * 2;
    int v3 = v2 - seed;
    int v4 = v3 ^ v1;
    int v5 = v4 | v2;
    int v6 = v5 & v3;
    int v7 = v6 + v4;
    int v8 = v7 * v5;
    int v9 = v8 - v6;
    int v10 = v9 ^ v7;
    int v11 = v10 | v8;
    int v12 = v11 & v9;
    int v13 = v12 + v10;
    int v14 = v13 * v11;
    int v15 = v14 - v12;
    int v16 = v15 ^ v13;
    int v17 = v16 | v14;
    int v18 = v17 & v15;
    int v19 = v18 + v16;
    int v20 = v19 * v17;
    int v21 = v20 - v18;
    int v22 = v21 ^ v19;
    int v23 = v22 | v20;
    int v24 = v23 & v21;
    int v25 = v24 + v22;
    int v26 = v25 * v23;
    int v27 = v26 - v24;
    int v28 = v27 ^ v25;
    int v29 = v28 | v26;
    int v30 = v29 & v27;
    
    long result = 0;
    for (int i = 0; i < iterations; i++) {
        /* Interdependent operations to prevent optimization */
        v1 = (v30 + i) & 0xFF;
        v2 = v1 * v29;
        v3 = v2 - v28;
        v4 = v3 ^ v27;
        v5 = v4 | v26;
        v6 = v5 & v25;
        v7 = v6 + v24;
        v8 = v7 * v23;
        v9 = v8 - v22;
        v10 = v9 ^ v21;
        v11 = v10 | v20;
        v12 = v11 & v19;
        v13 = v12 + v18;
        v14 = v13 * v17;
        v15 = v14 - v16;
        v16 = v15 ^ v15;  /* Self-operation creates complexity */
        v17 = v16 | v14;
        v18 = v17 & v13;
        v19 = v18 + v12;
        v20 = v19 * v11;
        v21 = v20 - v10;
        v22 = v21 ^ v9;
        v23 = v22 | v8;
        v24 = v23 & v7;
        v25 = v24 + v6;
        v26 = v25 * v5;
        v27 = v26 - v4;
        v28 = v27 ^ v3;
        v29 = v28 | v2;
        v30 = v29 & v1;
        
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                  v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                  v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
    }
    
    /* Use asm to prevent dead code elimination */
    asm volatile ("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4), "r"(v5),
                       "r"(v6), "r"(v7), "r"(v8), "r"(v9), "r"(v10),
                       "r"(v11), "r"(v12), "r"(v13), "r"(v14), "r"(v15),
                       "r"(v16), "r"(v17), "r"(v18), "r"(v19), "r"(v20),
                       "r"(v21), "r"(v22), "r"(v23), "r"(v24), "r"(v25),
                       "r"(v26), "r"(v27), "r"(v28), "r"(v29), "r"(v30));
    
    return result;
}

/* Pattern B: Floating-point intensive computation */
NOINLINE static double pattern_b(int iterations, double seed) {
    /* 20+ double variables to pressure FP registers */
    double d1 = seed * 1.1;
    double d2 = d1 + 2.2;
    double d3 = d2 - 3.3;
    double d4 = d3 * 4.4;
    double d5 = d4 / 5.5;
    double d6 = d5 + d1;
    double d7 = d6 * d2;
    double d8 = d7 - d3;
    double d9 = d8 / d4;
    double d10 = d9 + d5;
    double d11 = d10 * d6;
    double d12 = d11 - d7;
    double d13 = d12 / d8;
    double d14 = d13 + d9;
    double d15 = d14 * d10;
    double d16 = d15 - d11;
    double d17 = d16 / d12;
    double d18 = d17 + d13;
    double d19 = d18 * d14;
    double d20 = d19 - d15;
    
    double result = 0.0;
    for (int i = 0; i < iterations; i++) {
        /* Complex FP operations */
        d1 = (d20 + i) * 1.234567;
        d2 = d1 / (i + 1.0);
        d3 = d2 * d19;
        d4 = d3 - d18;
        d5 = d4 / d17;
        d6 = d5 + d16;
        d7 = d6 * d15;
        d8 = d7 - d14;
        d9 = d8 / d13;
        d10 = d9 + d12;
        d11 = d10 * d11;  /* Self-multiplication */
        d12 = d11 - d10;
        d13 = d12 / d9;
        d14 = d13 + d8;
        d15 = d14 * d7;
        d16 = d15 - d6;
        d17 = d16 / d5;
        d18 = d17 + d4;
        d19 = d18 * d3;
        d20 = d19 - d2;
        
        result += d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10 +
                  d11 + d12 + d13 + d14 + d15 + d16 + d17 + d18 + d19 + d20;
    }
    
    /* Prevent optimization */
    asm volatile ("" : : "w"(d1), "w"(d2), "w"(d3), "w"(d4), "w"(d5),
                       "w"(d6), "w"(d7), "w"(d8), "w"(d9), "w"(d10),
                       "w"(d11), "w"(d12), "w"(d13), "w"(d14), "w"(d15),
                       "w"(d16), "w"(d17), "w"(d18), "w"(d19), "w"(d20));
    
    return result;
}

/* Pattern C: Complex control flow with switch statement */
NOINLINE static int pattern_c(int value, int iterations) {
    int result = value;
    
    for (int i = 0; i < iterations; i++) {
        /* Switch with many cases creates complex CFG */
        switch ((result + i) % 32) {
            case 0:  result = result * 2 + 1; break;
            case 1:  result = result ^ 0xAAAA; break;
            case 2:  result = result | 0x5555; break;
            case 3:  result = result & 0xFFFF; break;
            case 4:  result = result + result; break;
            case 5:  result = result - 1234; break;
            case 6:  result = result * 3; break;
            case 7:  result = result / 2; break;
            case 8:  result = result % 1000; break;
            case 9:  result = result << 2; break;
            case 10: result = result >> 1; break;
            case 11: result = ~result; break;
            case 12: result = result ^ result; /* Becomes 0 */ break;
            case 13: result = 42; break;
            case 14: result = result * result; break;
            case 15: result = result + 0xDEAD; break;
            case 16: result = result | 0xBEEF; break;
            case 17: result = result & 0xCAFE; break;
            case 18: result = result ^ 0xF00D; break;
            case 19: result = result + 0xBAD; break;
            case 20: result = result - 0xC0DE; break;
            case 21: result = result * 7; break;
            case 22: result = result / 3; break;
            case 23: result = result % 777; break;
            case 24: result = result << 3; break;
            case 25: result = result >> 2; break;
            case 26: result = result + i; break;
            case 27: result = result - i; break;
            case 28: result = result * i; if (i == 0) result = 1; break;
            case 29: result = result ^ i; break;
            case 30: result = result | i; break;
            case 31: result = result & i; break;
            default: result = 0; break;
        }
        
        /* Nested loop with break/continue for additional CFG complexity */
        for (int j = 0; j < 5; j++) {
            if (j == 2) continue;
            if (j == 4) break;
            result += j;
        }
    }
    
    return result;
}

/* Pattern D: Vector extensions for SIMD register pressure */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

NOINLINE static v4si pattern_d(v4si init, int iterations) {
#ifdef __GNUC__
    v4si v1 = init + (v4si){1, 2, 3, 4};
    v4si v2 = v1 * (v4si){2, 2, 2, 2};
    v4si v3 = v2 - (v4si){1, 1, 1, 1};
    v4si v4 = v3 ^ (v4si){0xFF, 0xFF, 0xFF, 0xFF};
    v4si v5 = v4 | (v4si){0xAA, 0xAA, 0xAA, 0xAA};
    v4si v6 = v5 & (v4si){0x55, 0x55, 0x55, 0x55};
    v4si v7 = v6 + v1;
    v4si v8 = v7 * v2;
    v4si v9 = v8 - v3;
    v4si v10 = v9 ^ v4;
    
    v4si result = {0, 0, 0, 0};
    
    for (int i = 0; i < iterations; i++) {
        v1 = v10 + (v4si){i, i+1, i+2, i+3};
        v2 = v1 * (v4si){i, i, i, i};
        v3 = v2 - v9;
        v4 = v3 ^ v8;
        v5 = v4 | v7;
        v6 = v5 & v6;  /* Self-operation */
        v7 = v6 + v5;
        v8 = v7 * v4;
        v9 = v8 - v3;
        v10 = v9 ^ v2;
        
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    }
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "x"(v1), "x"(v2), "x"(v3), "x"(v4), "x"(v5),
                       "x"(v6), "x"(v7), "x"(v8), "x"(v9), "x"(v10));
    
    return result;
#else
    return init;
#endif
}

/* Pattern E: Explicit register variables to conflict with allocator */
NOINLINE static long pattern_e(int iterations) {
    /* Try to use specific registers */
    register long r1 asm ("r12") = 1;
    register long r2 asm ("r13") = 2;
    register long r3 asm ("r14") = 3;
    register long r4 asm ("r15") = 4;
    register long r5 asm ("rbx") = 5;
    
    /* Additional non-register variables to create pressure */
    long v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    long v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    long v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    
    long result = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Mix register and non-register variables */
        r1 = r2 + r3;
        r2 = r3 * r4;
        r3 = r4 - r5;
        r4 = r5 ^ r1;
        r5 = r1 | r2;
        
        v6 = r1 + v20;
        v7 = r2 * v19;
        v8 = r3 - v18;
        v9 = r4 ^ v17;
        v10 = r5 | v16;
        
        v11 = v6 + v15;
        v12 = v7 * v14;
        v13 = v8 - v13;  /* Self-operation */
        v14 = v9 ^ v12;
        v15 = v10 | v11;
        
        v16 = v11 + v10;
        v17 = v12 * v9;
        v18 = v13 - v8;
        v19 = v14 ^ v7;
        v20 = v15 | v6;
        
        result += r1 + r2 + r3 + r4 + r5 + v6 + v7 + v8 + v9 + v10 +
                  v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    }
    
    /* Force register variables to be used */
    asm volatile ("" : : "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5));
    
    return result;
}

/* Main function that calls all patterns */
COLD int main(int argc, char *argv[]) {
    int iterations = 1000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 10) iterations = 10;
        if (iterations > 100000) iterations = 100000;
    }
    
    long total = 0;
    
    /* Call each pattern multiple times with different inputs */
    for (int i = 0; i < 5; i++) {
        total += pattern_a(iterations / 10, i * 12345);
        total += (long)pattern_b(iterations / 20, i * 1.2345);
        total += pattern_c(i * 54321, iterations / 50);
        
#ifdef __GNUC__
        v4si vec = {i, i+1, i+2, i+3};
        v4si vec_result = pattern_d(vec, iterations / 100);
        total += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
#endif
        
        total += pattern_e(iterations / 30);
    }
    
    /* Use CPU features to engage target-specific optimizations */
#ifdef __GNUC__
    if (__builtin_cpu_supports("avx2")) {
        total += 0x1000;
    }
    if (__builtin_cpu_supports("sse4.2")) {
        total += 0x2000;
    }
#endif
    
    /* Print result to prevent optimization of entire program */
    printf("Result: %ld (iterations: %d)\n", total, iterations);
    
    return (total > 0) ? 0 : 1;
}
