/* test_mcf_coverage.c
 * 
 * This test program is designed to stress GCC's Min-Cost Flow register allocator
 * to trigger coverage of special node indices in the print_node function.
 * 
 * Compile with: gcc -O3 -fprofile-arcs -ftest-coverage -fdump-rtl-all test_mcf_coverage.c -o test_mcf
 * Run with: ./test_mcf
 * Generate coverage with: gcov test_mcf_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units for each pressure function */
#define NOINLINE __attribute__((noinline))
#define COLD __attribute__((cold))

/* Pattern A: Integer arithmetic chain with 30+ variables in a loop
 * Forces spill decisions through high register pressure */
NOINLINE static long pattern_a_int_pressure(int iterations, int seed) {
    /* Declare 30+ integer variables to create register pressure */
    int v1 = seed + 1;
    int v2 = seed * 2;
    int v3 = seed ^ 0x55;
    int v4 = seed << 1;
    int v5 = seed >> 1;
    int v6 = v1 + v2;
    int v7 = v3 - v4;
    int v8 = v5 * v6;
    int v9 = v7 / (v1 ? v1 : 1);
    int v10 = v8 ^ v9;
    int v11 = v2 + v3;
    int v12 = v4 - v5;
    int v13 = v6 * v7;
    int v14 = v8 / (v9 ? v9 : 1);
    int v15 = v10 ^ v11;
    int v16 = v12 + v13;
    int v17 = v14 - v15;
    int v18 = v16 * v17;
    int v19 = v18 / (v1 ? v1 : 1);
    int v20 = v19 ^ v10;
    int v21 = v11 + v12;
    int v22 = v13 - v14;
    int v23 = v15 * v16;
    int v24 = v17 / (v18 ? v18 : 1);
    int v25 = v19 ^ v20;
    int v26 = v21 + v22;
    int v27 = v23 - v24;
    int v28 = v25 * v26;
    int v29 = v27 / (v28 ? v28 : 1);
    int v30 = v29 ^ v25;
    
    /* Complex loop with interdependencies to prevent optimization */
    long sum = 0;
    for (int i = 0; i < iterations; i++) {
        /* Chain of operations creating data dependencies */
        v1 = v30 + i;
        v2 = v1 * v29;
        v3 = v2 ^ v28;
        v4 = v3 - v27;
        v5 = v4 / (v26 ? v26 : 1);
        v6 = v5 + v25;
        v7 = v6 * v24;
        v8 = v7 ^ v23;
        v9 = v8 - v22;
        v10 = v9 / (v21 ? v21 : 1);
        v11 = v10 + v20;
        v12 = v11 * v19;
        v13 = v12 ^ v18;
        v14 = v13 - v17;
        v15 = v14 / (v16 ? v16 : 1);
        v16 = v15 + v1;
        v17 = v16 * v2;
        v18 = v17 ^ v3;
        v19 = v18 - v4;
        v20 = v19 / (v5 ? v5 : 1);
        v21 = v20 + v6;
        v22 = v21 * v7;
        v23 = v22 ^ v8;
        v24 = v23 - v9;
        v25 = v24 / (v10 ? v10 : 1);
        v26 = v25 + v11;
        v27 = v26 * v12;
        v28 = v27 ^ v13;
        v29 = v28 - v14;
        v30 = v29 / (v15 ? v15 : 1);
        
        /* Prevent dead code elimination */
        asm volatile("" : "+r"(v1), "+r"(v2), "+r"(v3), "+r"(v4), "+r"(v5));
        sum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
               v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
               v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
    }
    
    return sum;
}

/* Pattern B: Floating-point intensive computation
 * Pressures floating-point register class */
NOINLINE static double pattern_b_float_pressure(int iterations, double seed) {
    /* Many double variables for FP register pressure */
    double d1 = seed * 1.1;
    double d2 = seed / 1.2;
    double d3 = seed + 3.14159;
    double d4 = seed - 2.71828;
    double d5 = d1 * d2;
    double d6 = d3 / d4;
    double d7 = d5 + d6;
    double d8 = d5 - d6;
    double d9 = d7 * d8;
    double d10 = d7 / (d8 != 0.0 ? d8 : 1.0);
    double d11 = d9 + d10;
    double d12 = d9 - d10;
    double d13 = d11 * d12;
    double d14 = d11 / (d12 != 0.0 ? d12 : 1.0);
    double d15 = d13 + d14;
    double d16 = d13 - d14;
    double d17 = d15 * d16;
    double d18 = d15 / (d16 != 0.0 ? d16 : 1.0);
    double d19 = d17 + d18;
    double d20 = d17 - d18;
    
    double result = 0.0;
    for (int i = 0; i < iterations; i++) {
        /* Complex FP operations with dependencies */
        d1 = d20 * (i + 1);
        d2 = d19 / (d1 != 0.0 ? d1 : 1.0);
        d3 = d18 + d1;
        d4 = d17 - d2;
        d5 = d3 * d4;
        d6 = d3 / (d4 != 0.0 ? d4 : 1.0);
        d7 = d5 + d6;
        d8 = d5 - d6;
        d9 = d7 * d8;
        d10 = d7 / (d8 != 0.0 ? d8 : 1.0);
        d11 = d9 + d10;
        d12 = d9 - d10;
        d13 = d11 * d12;
        d14 = d11 / (d12 != 0.0 ? d12 : 1.0);
        d15 = d13 + d14;
        d16 = d13 - d14;
        d17 = d15 * d16;
        d18 = d15 / (d16 != 0.0 ? d16 : 1.0);
        d19 = d17 + d18;
        d20 = d17 - d18;
        
        /* Mix with integer operations to create diverse pressure */
        int idx = i & 0xF;
        switch (idx) {
            case 0: d1 += 1.0; break;
            case 1: d2 -= 1.0; break;
            case 2: d3 *= 1.01; break;
            case 3: d4 /= 1.01; break;
            default: d5 = d1 + d2 + d3 + d4; break;
        }
        
        result += d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10 +
                  d11 + d12 + d13 + d14 + d15 + d16 + d17 + d18 + d19 + d20;
    }
    
    return result;
}

/* Pattern C: Complex control flow with switch statements
 * Creates many basic blocks for complex CFG */
NOINLINE static int pattern_c_cfg_complexity(int iterations, int mode) {
    int result = mode;
    
    for (int i = 0; i < iterations; i++) {
        /* Switch with many cases creates multiple basic blocks */
        switch ((i + mode) % 32) {
            case 0:  result += i * 1; break;
            case 1:  result -= i * 2; break;
            case 2:  result ^= i * 3; break;
            case 3:  result |= i * 4; break;
            case 4:  result &= i * 5; break;
            case 5:  result <<= (i % 8); break;
            case 6:  result >>= (i % 8); break;
            case 7:  result = result * 7 + i; break;
            case 8:  result = result / ((i % 16) + 1); break;
            case 9:  result = ~result; break;
            case 10: result = result ^ 0xAAAAAAAA; break;
            case 11: result = result | 0x55555555; break;
            case 12: result = result & 0x33333333; break;
            case 13: result = result + (i << 16); break;
            case 14: result = result - (i << 8); break;
            case 15: result = result * 3 / 2; break;
            case 16: result = (result << 1) | (result >> 31); break;
            case 17: result = (result >> 1) | (result << 31); break;
            case 18: result = result ^ (result << 13); break;
            case 19: result = result ^ (result >> 17); break;
            case 20: result = result ^ (result << 5); break;
            case 21: result = result + 0x9E3779B9; break;
            case 22: result = result - 0x61C88647; break;
            case 23: result = result * 0x5DEECE66D; break;
            case 24: result = result / ((i & 7) + 1); break;
            case 25: result = result % ((i & 15) + 1); break;
            case 26: result = result & ~i; break;
            case 27: result = result | i; break;
            case 28: result = result ^ (i * i); break;
            case 29: result = result + (result << 2); break;
            case 30: result = result - (result >> 3); break;
            case 31: result = result * 1103515245 + 12345; break;
        }
        
        /* Nested loops with break/continue for more CFG edges */
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

NOINLINE static long pattern_d_vector_pressure(int iterations) {
#ifdef __GNUC__
    /* Vector variables pressure SIMD registers */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = {9, 10, 11, 12};
    v4si vec4 = {13, 14, 15, 16};
    v4si vec5 = vec1 + vec2;
    v4si vec6 = vec3 - vec4;
    v4si vec7 = vec5 * vec6;
    v4si vec8 = vec1 & vec2;
    v4si vec9 = vec3 | vec4;
    v4si vec10 = vec5 ^ vec6;
    
    v4sf fvec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf fvec3 = fvec1 * fvec2;
    v4sf fvec4 = fvec1 + fvec2;
    
    long sum = 0;
    for (int i = 0; i < iterations; i++) {
        /* Vector operations */
        vec1 = vec2 + vec3;
        vec2 = vec3 - vec4;
        vec3 = vec4 * vec5;
        vec4 = vec5 & vec6;
        vec5 = vec6 | vec7;
        vec6 = vec7 ^ vec8;
        vec7 = vec8 + vec9;
        vec8 = vec9 - vec10;
        vec9 = vec10 * vec1;
        vec10 = vec1 & vec2;
        
        fvec1 = fvec2 * fvec3;
        fvec2 = fvec3 + fvec4;
        fvec3 = fvec4 - fvec1;
        fvec4 = fvec1 * fvec2;
        
        /* Extract and sum elements to prevent optimization */
        int* p1 = (int*)&vec1;
        int* p2 = (int*)&vec2;
        sum += p1[0] + p1[1] + p1[2] + p1[3] +
               p2[0] + p2[1] + p2[2] + p2[3];
    }
    
    return sum;
#else
    /* Fallback for non-GCC compilers */
    return iterations * 100;
#endif
}

/* Pattern E: Explicit register variables to conflict with allocator */
NOINLINE static int pattern_e_register_conflict(int iterations) {
    /* Explicit register variables that may conflict with allocator's choices */
    register int r1 asm ("r12") = 1;
    register int r2 asm ("r13") = 2;
    register int r3 asm ("r14") = 3;
    register int r4 asm ("r15") = 4;
    
    int v1 = 10, v2 = 20, v3 = 30, v4 = 40;
    int v5 = 50, v6 = 60, v7 = 70, v8 = 80;
    int v9 = 90, v10 = 100, v11 = 110, v12 = 120;
    
    /* Mix explicit and implicit registers */
    for (int i = 0; i < iterations; i++) {
        r1 = r2 + v1;
        r2 = r3 - v2;
        r3 = r4 * v3;
        r4 = r1 / (v4 ? v4 : 1);
        
        v1 = v2 + r1;
        v2 = v3 - r2;
        v3 = v4 * r3;
        v4 = v5 / (r4 ? r4 : 1);
        
        v5 = v6 + v1;
        v6 = v7 - v2;
        v7 = v8 * v3;
        v8 = v9 / (v4 ? v4 : 1);
        
        v9 = v10 + v5;
        v10 = v11 - v6;
        v11 = v12 * v7;
        v12 = v1 / (v8 ? v8 : 1);
        
        /* Force register spills with many live variables */
        asm volatile("" : "+r"(r1), "+r"(r2), "+r"(r3), "+r"(r4),
                          "+r"(v1), "+r"(v2), "+r"(v3), "+r"(v4),
                          "+r"(v5), "+r"(v6), "+r"(v7), "+r"(v8),
                          "+r"(v9), "+r"(v10), "+r"(v11), "+r"(v12));
    }
    
    return r1 + r2 + r3 + r4 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11 + v12;
}

/* Main function marked cold to potentially affect block ordering */
COLD int main(int argc, char** argv) {
    int iterations = 1000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 100) iterations = 100;
        if (iterations > 10000) iterations = 10000;
    }
    
    long total = 0;
    
    /* Call each pressure pattern with different arguments */
    total += pattern_a_int_pressure(iterations, 42);
    total += (long)pattern_b_float_pressure(iterations / 10, 3.14159);
    total += pattern_c_cfg_complexity(iterations / 5, 12345);
    total += pattern_d_vector_pressure(iterations / 20);
    total += pattern_e_register_conflict(iterations / 2);
    
    /* Use CPU features to engage target-specific optimizations */
#ifdef __GNUC__
    if (__builtin_cpu_supports("avx2")) {
        total += 1000;
    }
    if (__builtin_cpu_supports("sse4.2")) {
        total += 500;
    }
#endif
    
    /* Print result to prevent optimization */
    printf("Result: %ld\n", total);
    
    return (total > 0) ? 0 : 1;
}
