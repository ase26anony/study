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

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))
#define COLD __attribute__((cold))

/* Stub for non-GCC compilers */
#ifndef __GNUC__
#define __builtin_cpu_supports(x) 0
#endif

/* Pattern A: Integer arithmetic chain with 30+ variables in a loop
 * Forces spill decisions through register pressure */
NOINLINE static long pattern_a_intensive(int iterations, int seed) {
    /* Declare 30+ integer variables to create register pressure */
    int v1 = seed + 1;
    int v2 = v1 * 2;
    int v3 = v2 + v1;
    int v4 = v3 - seed;
    int v5 = v4 * 3;
    int v6 = v5 / 2;
    int v7 = v6 | v5;
    int v8 = v7 & 0xFF;
    int v9 = v8 << 2;
    int v10 = v9 >> 1;
    int v11 = v10 ^ v9;
    int v12 = v11 + v10;
    int v13 = v12 * v11;
    int v14 = v13 % 17;
    int v15 = v14 + v13;
    int v16 = v15 - v14;
    int v17 = v16 * v15;
    int v18 = v17 / 3;
    int v19 = v18 | 0xAAAA;
    int v20 = v19 & 0x5555;
    int v21 = v20 + v19;
    int v22 = v21 * 2;
    int v23 = v22 - v21;
    int v24 = v23 + seed;
    int v25 = v24 * v23;
    int v26 = v25 % 19;
    int v27 = v26 ^ 0x1234;
    int v28 = v27 << 3;
    int v29 = v28 >> 2;
    int v30 = v29 + v28;
    int v31 = v30 * v29;
    int v32 = v31 - v30;
    
    long result = 0;
    
    /* Tight interdependent loop to maximize register pressure */
    for (int i = 0; i < iterations; i++) {
        /* Create complex interdependencies */
        v1 = v2 + i;
        v2 = v3 - v1;
        v3 = v4 * v2;
        v4 = v5 ^ v3;
        v5 = v6 | v4;
        v6 = v7 + v5;
        v7 = v8 - v6;
        v8 = v9 * v7;
        v9 = v10 ^ v8;
        v10 = v11 | v9;
        v11 = v12 + v10;
        v12 = v13 - v11;
        v13 = v14 * v12;
        v14 = v15 ^ v13;
        v15 = v16 | v14;
        v16 = v17 + v15;
        v17 = v18 - v16;
        v18 = v19 * v17;
        v19 = v20 ^ v18;
        v20 = v21 | v19;
        v21 = v22 + v20;
        v22 = v23 - v21;
        v23 = v24 * v22;
        v24 = v25 ^ v23;
        v25 = v26 | v24;
        v26 = v27 + v25;
        v27 = v28 - v26;
        v28 = v29 * v27;
        v29 = v30 ^ v28;
        v30 = v31 | v29;
        v31 = v32 + v30;
        v32 = v1 ^ v31;
        
        /* Prevent loop elimination */
        asm volatile("" : "+r"(v1), "+r"(v2), "+r"(v3), "+r"(v4),
                        "+r"(v5), "+r"(v6), "+r"(v7), "+r"(v8),
                        "+r"(v9), "+r"(v10), "+r"(v11), "+r"(v12),
                        "+r"(v13), "+r"(v14), "+r"(v15), "+r"(v16),
                        "+r"(v17), "+r"(v18), "+r"(v19), "+r"(v20),
                        "+r"(v21), "+r"(v22), "+r"(v23), "+r"(v24),
                        "+r"(v25), "+r"(v26), "+r"(v27), "+r"(v28),
                        "+r"(v29), "+r"(v30), "+r"(v31), "+r"(v32) : : "memory");
        
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + 
                 v9 + v10 + v11 + v12 + v13 + v14 + v15 + v16 +
                 v17 + v18 + v19 + v20 + v21 + v22 + v23 + v24 +
                 v25 + v26 + v27 + v28 + v29 + v30 + v31 + v32;
    }
    
    return result;
}

/* Pattern B: Floating-point intensive computation
 * Pressures floating-point register class */
NOINLINE static double pattern_b_fp_intensive(int iterations, double seed) {
    /* Declare many double variables */
    double d1 = seed;
    double d2 = d1 * 1.1;
    double d3 = d2 + 0.5;
    double d4 = d3 - d2;
    double d5 = d4 * 2.0;
    double d6 = d5 / 1.5;
    double d7 = d6 + d5;
    double d8 = d7 * 0.75;
    double d9 = d8 - d7;
    double d10 = d9 * 3.14159;
    double d11 = d10 / 2.71828;
    double d12 = d11 + d10;
    double d13 = d12 * 1.41421;
    double d14 = d13 - d12;
    double d15 = d14 * 0.7071;
    double d16 = d15 + d14;
    double d17 = d16 * 1.73205;
    double d18 = d17 / 2.23607;
    double d19 = d18 + d17;
    double d20 = d19 * 0.61803;
    
    double result = 0.0;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex FP operations with interdependencies */
        d1 = d2 * d20 + (double)i;
        d2 = d3 - d1 * 0.5;
        d3 = d4 * d2 + d1;
        d4 = d5 / (d3 + 1.0);
        d5 = d6 * d4 - d3;
        d6 = d7 + d5 * 0.25;
        d7 = d8 * d6 / 2.0;
        d8 = d9 - d7 + d6;
        d9 = d10 * d8 * 0.333;
        d10 = d11 + d9 - d8;
        d11 = d12 * d10 / 1.1;
        d12 = d13 - d11 + d10;
        d13 = d14 * d12 * 0.9;
        d14 = d15 + d13 - d12;
        d15 = d16 * d14 / 1.05;
        d16 = d17 - d15 + d14;
        d17 = d18 * d16 * 0.95;
        d18 = d19 + d17 - d16;
        d19 = d20 * d18 / 1.01;
        d20 = d1 + d19 - d18;
        
        /* Prevent optimization */
        asm volatile("" : "+f"(d1), "+f"(d2), "+f"(d3), "+f"(d4),
                        "+f"(d5), "+f"(d6), "+f"(d7), "+f"(d8),
                        "+f"(d9), "+f"(d10), "+f"(d11), "+f"(d12),
                        "+f"(d13), "+f"(d14), "+f"(d15), "+f"(d16),
                        "+f"(d17), "+f"(d18), "+f"(d19), "+f"(d20) : : "memory");
        
        result += d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + 
                 d9 + d10 + d11 + d12 + d13 + d14 + d15 + d16 +
                 d17 + d18 + d19 + d20;
    }
    
    return result;
}

/* Pattern C: Complex control flow with switch statement
 * Creates many basic blocks for complex CFG */
NOINLINE static int pattern_c_complex_cfg(int value, int iterations) {
    int result = value;
    
    for (int i = 0; i < iterations; i++) {
        /* Switch with many cases creates multiple basic blocks */
        switch (result % 23) {
            case 0:  result = result * 2 + 1; break;
            case 1:  result = result / 2 + 3; break;
            case 2:  result = result ^ 0x55AA; break;
            case 3:  result = result | 0xFF00; break;
            case 4:  result = result & 0x00FF; break;
            case 5:  result = result << 3; break;
            case 6:  result = result >> 2; break;
            case 7:  result = result + 0x1234; break;
            case 8:  result = result - 0x5678; break;
            case 9:  result = result * 3; break;
            case 10: result = result / 4; break;
            case 11: result = result % 17; break;
            case 12: result = result ^ result; break;
            case 13: result = result | (result + 1); break;
            case 14: result = result & (result - 1); break;
            case 15: result = result << (i % 4); break;
            case 16: result = result >> (i % 4); break;
            case 17: result = ~result; break;
            case 18: result = result + i; break;
            case 19: result = result - i; break;
            case 20: result = result * i; if (i == 0) result = 1; break;
            case 21: result = result ^ i; break;
            case 22: result = result | i; break;
            default: result = 0; break;
        }
        
        /* Nested loop with break/continue for additional CFG complexity */
        for (int j = 0; j < 5; j++) {
            if (j == 2) continue;
            if (j == 4) break;
            result += j;
        }
        
        /* Prevent dead code elimination */
        asm volatile("" : "+r"(result) : : "memory");
    }
    
    return result;
}

/* Pattern D: Vector extensions for SIMD register pressure */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

NOINLINE static long pattern_d_vector_ops(int iterations) {
#ifdef __GNUC__
    /* Initialize vector variables */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = {9, 10, 11, 12};
    v4si vec4 = {13, 14, 15, 16};
    v4si vec5 = {17, 18, 19, 20};
    v4si vec6 = {21, 22, 23, 24};
    v4si vec7 = {25, 26, 27, 28};
    v4si vec8 = {29, 30, 31, 32};
    
    v4sf fvec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf fvec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf fvec4 = {13.0f, 14.0f, 15.0f, 16.0f};
    
    long result = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Vector operations creating register pressure */
        vec1 = vec1 + vec2;
        vec2 = vec2 * vec3;
        vec3 = vec3 - vec4;
        vec4 = vec4 & vec5;
        vec5 = vec5 | vec6;
        vec6 = vec6 ^ vec7;
        vec7 = vec7 + vec8;
        vec8 = vec8 * vec1;
        
        fvec1 = fvec1 + fvec2;
        fvec2 = fvec2 * fvec3;
        fvec3 = fvec3 - fvec4;
        fvec4 = fvec4 * 2.0f;
        
        /* Extract and sum elements */
        int* v1p = (int*)&vec1;
        int* v2p = (int*)&vec2;
        int* v3p = (int*)&vec3;
        int* v4p = (int*)&vec4;
        
        for (int j = 0; j < 4; j++) {
            result += v1p[j] + v2p[j] + v3p[j] + v4p[j];
        }
        
        /* Prevent optimization */
        asm volatile("" : "+x"(vec1), "+x"(vec2), "+x"(vec3), "+x"(vec4),
                        "+x"(vec5), "+x"(vec6), "+x"(vec7), "+x"(vec8),
                        "+x"(fvec1), "+x"(fvec2), "+x"(fvec3), "+x"(fvec4) : : "memory");
    }
    
    return result;
#else
    return iterations * 100;
#endif
}

/* Pattern E: Explicit register variables to conflict with allocator */
NOINLINE static int pattern_e_explicit_registers(int iterations) {
    /* Try to use specific registers that might conflict with allocator */
    register int r1 asm ("r12") = 1;
    register int r2 asm ("r13") = 2;
    register int r3 asm ("r14") = 3;
    register int r4 asm ("r15") = 4;
    
    int temp1, temp2, temp3, temp4;
    
    for (int i = 0; i < iterations; i++) {
        /* Force register shuffling */
        asm volatile("mov %0, %1" : "=r"(temp1) : "r"(r1));
        asm volatile("mov %0, %1" : "=r"(temp2) : "r"(r2));
        asm volatile("mov %0, %1" : "=r"(temp3) : "r"(r3));
        asm volatile("mov %0, %1" : "=r"(temp4) : "r"(r4));
        
        r1 = temp2 + i;
        r2 = temp3 - i;
        r3 = temp4 * i;
        r4 = temp1 ^ i;
        
        /* Complex arithmetic to create spill pressure */
        int v1 = r1 + r2;
        int v2 = r3 - r4;
        int v3 = v1 * v2;
        int v4 = v3 / (i + 1);
        int v5 = v4 | v3;
        int v6 = v5 & 0xFF;
        int v7 = v6 << 2;
        int v8 = v7 >> 1;
        int v9 = v8 ^ v7;
        int v10 = v9 + v8;
        
        /* Force these to be live across asm statements */
        asm volatile("" : "+r"(v1), "+r"(v2), "+r"(v3), "+r"(v4),
                        "+r"(v5), "+r"(v6), "+r"(v7), "+r"(v8),
                        "+r"(v9), "+r"(v10) : : "memory");
        
        r1 = v1 + v10;
        r2 = v2 + v9;
        r3 = v3 + v8;
        r4 = v4 + v7;
    }
    
    return r1 + r2 + r3 + r4;
}

/* Main function that calls all patterns */
COLD int main(int argc, char** argv) {
    int iterations = 1000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 100) iterations = 100;
        if (iterations > 10000) iterations = 10000;
    }
    
    /* Use CPU feature check to engage target-specific optimizations */
    int use_avx2 = __builtin_cpu_supports("avx2");
    int use_sse4 = __builtin_cpu_supports("sse4.2");
    
    printf("Starting MCF stress test (iterations: %d)\n", iterations);
    printf("AVX2 support: %d, SSE4.2 support: %d\n", use_avx2, use_sse4);
    
    long total = 0;
    
    /* Call each pattern multiple times with different inputs */
    for (int i = 0; i < 5; i++) {
        total += pattern_a_intensive(iterations / 10, i * 12345);
        total += (long)pattern_b_fp_intensive(iterations / 20, i * 1.2345);
        total += pattern_c_complex_cfg(total % 1000, iterations / 50);
        total += pattern_d_vector_ops(iterations / 30);
        total += pattern_e_explicit_registers(iterations / 40);
        
        /* Prevent optimization across iterations */
        asm volatile("" : "+r"(total) : : "memory");
    }
    
    printf("Result: %ld\n", total);
    
    /* Generate some profile data */
    for (int i = 0; i < 100; i++) {
        if (i % 3 == 0) {
            pattern_a_intensive(10, i);
        }
        if (i % 5 == 0) {
            pattern_c_complex_cfg(i, 5);
        }
    }
    
    return (int)(total % 256);
}

/* Additional helper to ensure code isn't dead */
void __attribute__((constructor)) init_mcf_test(void) {
    /* This runs before main, ensuring all code paths are considered */
    static int initialized = 0;
    if (!initialized) {
        initialized = 1;
        /* Touch all patterns briefly */
        pattern_a_intensive(1, 42);
        pattern_b_fp_intensive(1, 3.14);
        pattern_c_complex_cfg(42, 1);
        pattern_d_vector_ops(1);
        pattern_e_explicit_registers(1);
    }
}
