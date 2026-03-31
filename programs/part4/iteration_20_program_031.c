/* test_mcf_coverage.c
 * 
 * This test program is designed to trigger GCC's Min-Cost Flow pass
 * during register allocation, specifically exercising the print_node
 * function with special node indices (ENTRY_BLOCK, EXIT_BLOCK,
 * new_exit_index, new_entry_index).
 *
 * Compile with: gcc -O3 -fprofile-arcs -ftest-coverage -fdump-rtl-all test_mcf_coverage.c -o test_mcf
 * Run with: ./test_mcf
 * Generate coverage with: gcov test_mcf_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure each function is compiled independently */
#define NOINLINE __attribute__((noinline))
#define NORETURN __attribute__((noreturn))
#define COLD __attribute__((cold))

/* Vector type for SIMD pressure */
typedef int v4si __attribute__((vector_size(16)));

/* ============================================
 * PATTERN A: Integer arithmetic chain with 30+ variables
 * Creates massive register pressure in a tight loop
 * ============================================ */
NOINLINE static int pattern_a_int_pressure(int seed, int iterations) {
    /* Declare 30+ integer variables to pressure integer registers */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int a11, a12, a13, a14, a15, a16, a17, a18, a19, a20;
    int a21, a22, a23, a24, a25, a26, a27, a28, a29, a30;
    int a31, a32, a33, a34, a35, a36, a37, a38, a39, a40;
    
    /* Initialize with seed to prevent constant folding */
    a1 = seed;
    a2 = seed + 1;
    a3 = seed * 2;
    a4 = seed ^ 0x55;
    
    /* Complex interdependent computation in a loop */
    for (int i = 0; i < iterations; i++) {
        /* Chain of arithmetic operations creating data dependencies */
        a5 = a1 + a2;
        a6 = a3 - a4;
        a7 = a5 * a6;
        a8 = a7 / (a1 + 1);
        a9 = a8 ^ a2;
        a10 = a9 << 3;
        a11 = a10 >> 1;
        a12 = a11 | a3;
        a13 = a12 & 0xFF;
        a14 = a13 + a4;
        a15 = a14 * a5;
        a16 = a15 - a6;
        a17 = a16 ^ a7;
        a18 = a17 | a8;
        a19 = a18 & a9;
        a20 = a19 << 2;
        a21 = a20 >> 1;
        a22 = a21 + a10;
        a23 = a22 * a11;
        a24 = a23 - a12;
        a25 = a24 ^ a13;
        a26 = a25 | a14;
        a27 = a26 & a15;
        a28 = a27 + a16;
        a29 = a28 * a17;
        a30 = a29 - a18;
        a31 = a30 ^ a19;
        a32 = a31 | a20;
        a33 = a32 & a21;
        a34 = a33 + a22;
        a35 = a34 * a23;
        a36 = a35 - a24;
        a37 = a36 ^ a25;
        a38 = a37 | a26;
        a39 = a38 & a27;
        a40 = a39 + a28;
        
        /* Rotate values to create loop-carried dependencies */
        a1 = a40 ^ i;
        a2 = a39 - i;
        a3 = a38 * (i & 0xF);
        a4 = a37 | (i << 8);
    }
    
    /* Combine all results to prevent dead code elimination */
    return a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
           a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19 + a20 +
           a21 + a22 + a23 + a24 + a25 + a26 + a27 + a28 + a29 + a30 +
           a31 + a32 + a33 + a34 + a35 + a36 + a37 + a38 + a39 + a40;
}

/* ============================================
 * PATTERN B: Floating-point intensive computation
 * Pressures floating-point registers
 * ============================================ */
NOINLINE static double pattern_b_fp_pressure(double seed, int iterations) {
    /* 20+ double variables to pressure FP registers */
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    double d11, d12, d13, d14, d15, d16, d17, d18, d19, d20;
    double d21, d22, d23, d24, d25;
    
    d1 = seed;
    d2 = seed * 1.1;
    d3 = seed / 1.2;
    d4 = seed + 3.14159;
    
    for (int i = 0; i < iterations; i++) {
        /* FP arithmetic chain with transcendental functions */
        d5 = d1 + d2;
        d6 = d3 - d4;
        d7 = d5 * d6;
        d8 = d7 / (d1 + 1.0);
        d9 = d8 * 0.5;
        d10 = d9 + d2;
        d11 = d10 - d3;
        d12 = d11 * d4;
        d13 = d12 / d5;
        d14 = d13 + d6;
        d15 = d14 * d7;
        d16 = d15 - d8;
        d17 = d16 / d9;
        d18 = d17 + d10;
        d19 = d18 * d11;
        d20 = d19 - d12;
        d21 = d20 / d13;
        d22 = d21 + d14;
        d23 = d22 * d15;
        d24 = d23 - d16;
        d25 = d24 / d17;
        
        /* Mix in some integer arithmetic to create register class conflicts */
        int mix = i & 0xF;
        d1 = d25 + mix;
        d2 = d24 - (mix * 0.1);
        d3 = d23 * (1.0 + mix / 100.0);
        d4 = d22 / (1.0 + mix / 50.0);
    }
    
    return d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10 +
           d11 + d12 + d13 + d14 + d15 + d16 + d17 + d18 + d19 + d20 +
           d21 + d22 + d23 + d24 + d25;
}

/* ============================================
 * PATTERN C: Complex control flow with switch statement
 * Creates many basic blocks for CFG complexity
 * ============================================ */
NOINLINE static int pattern_c_cfg_complexity(int seed, int iterations) {
    int result = seed;
    
    for (int i = 0; i < iterations; i++) {
        /* Switch with 20+ cases creates many basic blocks */
        switch ((result + i) % 23) {
            case 0:  result += i * 2; break;
            case 1:  result -= i * 3; break;
            case 2:  result ^= i; break;
            case 3:  result |= 0xF0F0; break;
            case 4:  result &= 0x0F0F; break;
            case 5:  result = result << 1; break;
            case 6:  result = result >> 2; break;
            case 7:  result = result * 3; break;
            case 8:  result = result / 2; break;
            case 9:  result = result % 17; break;
            case 10: result = ~result; break;
            case 11: result = result + (i << 4); break;
            case 12: result = result - (i << 3); break;
            case 13: result = result | (i << 8); break;
            case 14: result = result & (i | 0xFF); break;
            case 15: result = result ^ (i << 16); break;
            case 16: result = (result << 3) | (result >> 29); break;
            case 17: result = result * result; break;
            case 18: result = result + 0x12345678; break;
            case 19: result = result - 0x87654321; break;
            case 20: result = result ^ 0xAAAAAAAA; break;
            case 21: result = result | 0x55555555; break;
            case 22: result = result & 0x33333333; break;
            default: result = 1; /* Should never happen */
        }
        
        /* Nested loops with break/continue for additional CFG edges */
        for (int j = 0; j < 5; j++) {
            if (j == 2) continue;
            result += j;
            if (result > 1000000) break;
        }
    }
    
    return result;
}

/* ============================================
 * PATTERN D: SIMD vector operations
 * Pressures vector/SIMD registers
 * ============================================ */
NOINLINE static v4si pattern_d_simd_pressure(v4si seed, int iterations) {
    v4si v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    v4si v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    
    v1 = seed;
    v2 = seed + (v4si){1, 2, 3, 4};
    v3 = seed * (v4si){2, 2, 2, 2};
    v4 = seed & (v4si){0xFF, 0xFF, 0xFF, 0xFF};
    
    for (int i = 0; i < iterations; i++) {
        v5 = v1 + v2;
        v6 = v3 - v4;
        v7 = v5 * v6;
        v8 = v7 & v1;
        v9 = v8 | v2;
        v10 = v9 ^ v3;
        v11 = v10 << 1;
        v12 = v11 >> 2;
        v13 = v12 + v4;
        v14 = v13 * v5;
        v15 = v14 - v6;
        v16 = v15 & v7;
        v17 = v16 | v8;
        v18 = v17 ^ v9;
        v19 = v18 << 3;
        v20 = v19 >> 1;
        
        /* Rotate with loop index */
        v4si idx = (v4si){i, i+1, i+2, i+3};
        v1 = v20 + idx;
        v2 = v19 - idx;
        v3 = v18 * idx;
        v4 = v17 | idx;
    }
    
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
}

/* ============================================
 * PATTERN E: Explicit register variables
 * Conflicts with allocator's choices
 * ============================================ */
NOINLINE static int pattern_e_explicit_registers(int seed, int iterations) {
    /* Try to use specific registers that might conflict */
    register int r12_var asm ("r12") = seed;
    register int r13_var asm ("r13") = seed + 1;
    register int r14_var asm ("r14") = seed * 2;
    register int r15_var asm ("r15") = seed ^ 0x55;
    
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;
    
    for (int i = 0; i < iterations; i++) {
        /* Force use of explicit registers in computation */
        asm volatile ("" : "+r" (r12_var), "+r" (r13_var), "+r" (r14_var), "+r" (r15_var));
        
        tmp1 = r12_var + r13_var;
        tmp2 = r14_var - r15_var;
        tmp3 = tmp1 * tmp2;
        tmp4 = tmp3 / (r12_var + 1);
        tmp5 = tmp4 ^ r13_var;
        tmp6 = tmp5 << 3;
        tmp7 = tmp6 >> 1;
        tmp8 = tmp7 | r14_var;
        
        /* Update register variables */
        r12_var = tmp8 + i;
        r13_var = tmp7 - i;
        r14_var = tmp6 * (i & 0xF);
        r15_var = tmp5 | (i << 8);
    }
    
    /* Force all register variables to be live at return */
    asm volatile ("" : : "r" (r12_var), "r" (r13_var), "r" (r14_var), "r" (r15_var));
    
    return r12_var + r13_var + r14_var + r15_var + 
           tmp1 + tmp2 + tmp3 + tmp4 + tmp5 + tmp6 + tmp7 + tmp8;
}

/* ============================================
 * Helper to prevent optimization
 * ============================================ */
static void use_result(int result) {
    /* Use inline asm to prevent dead code elimination */
    asm volatile ("" : : "r" (result));
}

/* ============================================
 * Main function - orchestrates all patterns
 * ============================================ */
COLD int main(int argc, char **argv) {
    int total = 0;
    
    /* Use varying inputs to prevent constant folding */
    int base_seed = argc > 1 ? atoi(argv[1]) : 12345;
    
    /* Pattern A: Integer pressure */
    int result_a = pattern_a_int_pressure(base_seed, 100);
    use_result(result_a);
    total += result_a;
    
    /* Pattern B: Floating-point pressure */
    double result_b = pattern_b_fp_pressure(base_seed * 0.5, 50);
    /* Convert double to int for use_result */
    int result_b_int = (int)result_b;
    use_result(result_b_int);
    total += result_b_int;
    
    /* Pattern C: CFG complexity */
    int result_c = pattern_c_cfg_complexity(base_seed + 1, 200);
    use_result(result_c);
    total += result_c;
    
    /* Pattern D: SIMD pressure */
    v4si seed_vec = {base_seed, base_seed + 1, base_seed + 2, base_seed + 3};
    v4si result_d = pattern_d_simd_pressure(seed_vec, 75);
    /* Extract elements from vector */
    int result_d_int = result_d[0] + result_d[1] + result_d[2] + result_d[3];
    use_result(result_d_int);
    total += result_d_int;
    
    /* Pattern E: Explicit registers */
    int result_e = pattern_e_explicit_registers(base_seed + 2, 150);
    use_result(result_e);
    total += result_e;
    
    /* Use CPU feature detection to engage target-specific optimizations */
#ifdef __GNUC__
    if (__builtin_cpu_supports("avx2")) {
        total += 1; /* Mark that AVX2 is available */
    }
    if (__builtin_cpu_supports("sse4.2")) {
        total += 2;
    }
#endif
    
    /* Final result depends on all computations */
    printf("Total: %d\n", total % 1000);
    
    return total % 256;
}
