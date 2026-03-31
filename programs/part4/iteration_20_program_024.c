/* test_mcf_coverage.c
 * 
 * This test program is designed to trigger GCC's Min-Cost Flow pass
 * during register allocation, specifically to cover the special node
 * printing logic in mcf.cc's print_node function.
 *
 * Compile with: gcc -O3 -fprofile-arcs -ftest-coverage -fdump-rtl-bbro -c test_mcf_coverage.c
 * Then run: ./test_mcf_coverage (to generate profile data)
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units in same file */
#define NOINLINE __attribute__((noinline))
#define COLD __attribute__((cold))

/* Stub for non-GCC compilers */
#ifndef __GNUC__
#define __builtin_cpu_supports(x) 0
#endif

/* Pattern A: Integer arithmetic chain with 30+ variables in a loop */
NOINLINE static long pattern_a(int iterations, int seed) {
    /* 30+ integer variables to create register pressure */
    int v1 = seed, v2 = seed + 1, v3 = seed + 2, v4 = seed + 3, v5 = seed + 4;
    int v6 = seed + 5, v7 = seed + 6, v8 = seed + 7, v9 = seed + 8, v10 = seed + 9;
    int v11 = seed + 10, v12 = seed + 11, v13 = seed + 12, v14 = seed + 13, v15 = seed + 14;
    int v16 = seed + 15, v17 = seed + 16, v18 = seed + 17, v19 = seed + 18, v20 = seed + 19;
    int v21 = seed + 20, v22 = seed + 21, v23 = seed + 22, v24 = seed + 23, v25 = seed + 24;
    int v26 = seed + 25, v27 = seed + 26, v28 = seed + 27, v29 = seed + 28, v30 = seed + 29;
    int v31 = seed + 30, v32 = seed + 31, v33 = seed + 32, v34 = seed + 33, v35 = seed + 34;
    
    long sum = 0;
    
    /* Complex interdependent loop to force spill decisions */
    for (int i = 0; i < iterations; i++) {
        /* Chain of interdependent operations */
        v1 = v2 + v3;
        v2 = v3 * v4;
        v3 = v4 ^ v5;
        v4 = v5 | v6;
        v5 = v6 & v7;
        v6 = v7 - v8;
        v7 = v8 + v9;
        v8 = v9 * v10;
        v9 = v10 % (v11 + 1);
        v10 = v11 << 1;
        v11 = v12 >> 1;
        v12 = v13 + v14;
        v13 = v14 * v15;
        v14 = v15 ^ v16;
        v15 = v16 | v17;
        v16 = v17 & v18;
        v17 = v18 - v19;
        v18 = v19 + v20;
        v19 = v20 * v21;
        v20 = v21 % (v22 + 1);
        v21 = v22 << 2;
        v22 = v23 >> 2;
        v23 = v24 + v25;
        v24 = v25 * v26;
        v25 = v26 ^ v27;
        v26 = v27 | v28;
        v27 = v28 & v29;
        v28 = v29 - v30;
        v29 = v30 + v31;
        v30 = v31 * v32;
        v31 = v32 % (v33 + 1);
        v32 = v33 << 3;
        v33 = v34 >> 3;
        v34 = v35 + v1;
        v35 = v1 * v2;
        
        /* Prevent dead code elimination */
        asm volatile("" : "+r"(v1), "+r"(v2), "+r"(v3), "+r"(v4), "+r"(v5),
                        "+r"(v6), "+r"(v7), "+r"(v8), "+r"(v9), "+r"(v10),
                        "+r"(v11), "+r"(v12), "+r"(v13), "+r"(v14), "+r"(v15),
                        "+r"(v16), "+r"(v17), "+r"(v18), "+r"(v19), "+r"(v20),
                        "+r"(v21), "+r"(v22), "+r"(v23), "+r"(v24), "+r"(v25),
                        "+r"(v26), "+r"(v27), "+r"(v28), "+r"(v29), "+r"(v30),
                        "+r"(v31), "+r"(v32), "+r"(v33), "+r"(v34), "+r"(v35));
        
        sum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
               v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
               v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30 +
               v31 + v32 + v33 + v34 + v35;
    }
    
    return sum;
}

/* Pattern B: Floating-point intensive computation */
NOINLINE static double pattern_b(int iterations, double seed) {
    /* 20+ double variables to pressure FP registers */
    double d1 = seed, d2 = seed * 1.1, d3 = seed * 1.2, d4 = seed * 1.3, d5 = seed * 1.4;
    double d6 = seed * 1.5, d7 = seed * 1.6, d8 = seed * 1.7, d9 = seed * 1.8, d10 = seed * 1.9;
    double d11 = seed * 2.0, d12 = seed * 2.1, d13 = seed * 2.2, d14 = seed * 2.3, d15 = seed * 2.4;
    double d16 = seed * 2.5, d17 = seed * 2.6, d18 = seed * 2.7, d19 = seed * 2.8, d20 = seed * 2.9;
    double d21 = seed * 3.0, d22 = seed * 3.1, d23 = seed * 3.2, d24 = seed * 3.3, d25 = seed * 3.4;
    
    double sum = 0.0;
    
    for (int i = 0; i < iterations; i++) {
        /* FP operation chain */
        d1 = d2 + d3;
        d2 = d3 * d4;
        d3 = d4 / (d5 + 0.001);
        d4 = d5 - d6;
        d5 = d6 * d7;
        d6 = d7 + d8;
        d7 = d8 / (d9 + 0.001);
        d8 = d9 - d10;
        d9 = d10 * d11;
        d10 = d11 + d12;
        d11 = d12 / (d13 + 0.001);
        d12 = d13 - d14;
        d13 = d14 * d15;
        d14 = d15 + d16;
        d15 = d16 / (d17 + 0.001);
        d16 = d17 - d18;
        d17 = d18 * d19;
        d18 = d19 + d20;
        d19 = d20 / (d21 + 0.001);
        d20 = d21 - d22;
        d21 = d22 * d23;
        d22 = d23 + d24;
        d23 = d24 / (d25 + 0.001);
        d24 = d25 - d1;
        d25 = d1 * d2;
        
        /* Prevent optimization */
        asm volatile("" : "+f"(d1), "+f"(d2), "+f"(d3), "+f"(d4), "+f"(d5),
                        "+f"(d6), "+f"(d7), "+f"(d8), "+f"(d9), "+f"(d10),
                        "+f"(d11), "+f"(d12), "+f"(d13), "+f"(d14), "+f"(d15),
                        "+f"(d16), "+f"(d17), "+f"(d18), "+f"(d19), "+f"(d20),
                        "+f"(d21), "+f"(d22), "+f"(d23), "+f"(d24), "+f"(d25));
        
        sum += d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10 +
               d11 + d12 + d13 + d14 + d15 + d16 + d17 + d18 + d19 + d20 +
               d21 + d22 + d23 + d24 + d25;
    }
    
    return sum;
}

/* Pattern C: Complex control flow with switch statement */
NOINLINE static int pattern_c(int iterations, int seed) {
    int result = seed;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex switch with many cases creates many basic blocks */
        switch (i % 25) {
            case 0: result += i * 2; break;
            case 1: result ^= i; result *= 3; break;
            case 2: result |= 0xFF; result -= i; break;
            case 3: result &= 0xF0; result += seed; break;
            case 4: result <<= 1; result ^= 0xAA; break;
            case 5: result >>= 2; result |= 0x55; break;
            case 6: result = ~result; result += i; break;
            case 7: result = result % 17; result *= i; break;
            case 8: result = result / (i % 7 + 1); result += 42; break;
            case 9: result = (result << 3) | (result >> 5); break;
            case 10: result = result ^ (result >> 1); break;
            case 11: result = result * 1103515245 + 12345; break;
            case 12: result = (result & 0xFFFF) * i; break;
            case 13: result = result | (i << 8); break;
            case 14: result = result & (~i); result += seed; break;
            case 15: result = result ^ seed; result *= i; break;
            case 16: result = result + (i << 4); break;
            case 17: result = result - (i >> 2); break;
            case 18: result = result * 3 + 1; break;
            case 19: result = (result + i) % 256; break;
            case 20: result = result | 1; result <<= i % 8; break;
            case 21: result = result >> (i % 4); result ^= 0xCC; break;
            case 22: result = ~result & 0xFF; result += i; break;
            case 23: result = result * result % 1000; break;
            case 24: result = (result + seed) & 0xFFF; break;
            default: result = 0; /* Should never happen */
        }
        
        /* Nested loop with break/continue to complicate CFG */
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

NOINLINE static long pattern_d(int iterations, int seed) {
#ifdef __GNUC__
    v4si v1 = {seed, seed+1, seed+2, seed+3};
    v4si v2 = {seed+4, seed+5, seed+6, seed+7};
    v4si v3 = {seed+8, seed+9, seed+10, seed+11};
    v4si v4 = {seed+12, seed+13, seed+14, seed+15};
    v4si v5 = {seed+16, seed+17, seed+18, seed+19};
    v4si v6 = {seed+20, seed+21, seed+22, seed+23};
    v4si v7 = {seed+24, seed+25, seed+26, seed+27};
    v4si v8 = {seed+28, seed+29, seed+30, seed+31};
    
    long sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Vector operations */
        v1 = v2 + v3;
        v2 = v3 * v4;
        v3 = v4 ^ v5;
        v4 = v5 | v6;
        v5 = v6 & v7;
        v6 = v7 - v8;
        v7 = v8 + v1;
        v8 = v1 * v2;
        
        /* Prevent optimization */
        asm volatile("" : "+x"(v1), "+x"(v2), "+x"(v3), "+x"(v4),
                        "+x"(v5), "+x"(v6), "+x"(v7), "+x"(v8));
        
        /* Extract and sum elements */
        int* p1 = (int*)&v1;
        int* p2 = (int*)&v2;
        int* p3 = (int*)&v3;
        int* p4 = (int*)&v4;
        int* p5 = (int*)&v5;
        int* p6 = (int*)&v6;
        int* p7 = (int*)&v7;
        int* p8 = (int*)&v8;
        
        for (int j = 0; j < 4; j++) {
            sum += p1[j] + p2[j] + p3[j] + p4[j] + 
                   p5[j] + p6[j] + p7[j] + p8[j];
        }
    }
    
    return sum;
#else
    return seed * iterations; /* Fallback for non-GCC */
#endif
}

/* Pattern E: Explicit register variables to conflict with allocator */
NOINLINE static int pattern_e(int iterations, int seed) {
    /* Explicit register variables that may conflict with allocator choices */
    register int r1 asm ("r12") = seed;
    register int r2 asm ("r13") = seed + 1;
    register int r3 asm ("r14") = seed + 2;
    register int r4 asm ("r15") = seed + 3;
    
    int v1 = seed + 4, v2 = seed + 5, v3 = seed + 6, v4 = seed + 7;
    int v5 = seed + 8, v6 = seed + 9, v7 = seed + 10, v8 = seed + 11;
    
    for (int i = 0; i < iterations; i++) {
        /* Mix explicit and implicit registers */
        r1 = r2 + v1;
        r2 = r3 * v2;
        r3 = r4 ^ v3;
        r4 = r1 | v4;
        v1 = v2 + r1;
        v2 = v3 * r2;
        v3 = v4 ^ r3;
        v4 = v5 | r4;
        v5 = v6 & v1;
        v6 = v7 - v2;
        v7 = v8 + v3;
        v8 = v1 * v4;
        
        /* Force register usage */
        asm volatile("" : "+r"(r1), "+r"(r2), "+r"(r3), "+r"(r4),
                        "+r"(v1), "+r"(v2), "+r"(v3), "+r"(v4),
                        "+r"(v5), "+r"(v6), "+r"(v7), "+r"(v8));
    }
    
    return r1 + r2 + r3 + r4 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
}

/* Main driver function */
COLD int main(int argc, char** argv) {
    int iterations = 1000;
    int seed = 42;
    
    /* Use __builtin_cpu_supports to engage target-specific optimizations */
    int use_avx2 = __builtin_cpu_supports("avx2");
    int use_sse4 = __builtin_cpu_supports("sse4.2");
    
    printf("Starting MCF coverage test (AVX2: %d, SSE4.2: %d)\n", use_avx2, use_sse4);
    
    /* Call all patterns with varying arguments to prevent constant folding */
    long result_a = pattern_a(iterations + (argc > 1 ? atoi(argv[1]) % 10 : 0), seed);
    double result_b = pattern_b(iterations / 10, seed * 1.234);
    int result_c = pattern_c(iterations * 2, seed ^ 0xDEADBEEF);
    long result_d = pattern_d(iterations / 2, seed + 100);
    int result_e = pattern_e(iterations, seed * 3);
    
    /* Use results to prevent dead code elimination */
    volatile long final_result = result_a + (long)result_b + result_c + result_d + result_e;
    
    printf("Test completed. Final checksum: %ld\n", (long)final_result);
    
    return 0;
}
