/* test_mcf_coverage.c
 * 
 * This test program is designed to trigger GCC's Min-Cost Flow pass
 * during register allocation, specifically to exercise the print_node
 * function with special node indices (ENTRY_BLOCK, EXIT_BLOCK,
 * new_exit_index, new_entry_index).
 *
 * Compile with: gcc -O3 -fprofile-arcs -ftest-coverage -fdump-rtl-all -c test_mcf_coverage.c
 * Then run: ./test_mcf_coverage (to generate profile data)
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))
#define NORETURN __attribute__((noreturn))
#define COLD __attribute__((cold))

/* Stub for non-GCC compilers */
#ifndef __GNUC__
#define __builtin_cpu_supports(x) 0
#endif

/* Prevent dead code elimination without external calls */
#define USE(var) asm volatile("" : : "r"(var))

/* Vector type for SIMD pressure */
typedef int v4si __attribute__((vector_size(16)));

/* ============================================
 * PATTERN A: Integer arithmetic chain with 30+ variables
 * Creates massive register pressure in a loop
 * ============================================ */
NOINLINE
int pattern_a_int_pressure(int seed, int iterations) {
    /* Declare 30+ integer variables to pressure integer registers */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int a11, a12, a13, a14, a15, a16, a17, a18, a19, a20;
    int a21, a22, a23, a24, a25, a26, a27, a28, a29, a30;
    int a31, a32, a33, a34, a35, a36, a37, a38, a39, a40;
    
    /* Initialize with seed to prevent constant folding */
    a1 = seed;
    
    /* Complex interdependent computation chain */
    for (int i = 0; i < iterations; i++) {
        a2 = a1 + i;
        a3 = a2 * a1;
        a4 = a3 - a2;
        a5 = a4 ^ a3;
        a6 = a5 | a4;
        a7 = a6 & a5;
        a8 = a7 << 2;
        a9 = a8 >> 1;
        a10 = a9 + a8;
        a11 = a10 - a9;
        a12 = a11 * a10;
        a13 = a12 / (a11 ? a11 : 1);
        a14 = a13 % (a12 ? a12 : 1);
        a15 = a14 ^ a13;
        a16 = a15 | a14;
        a17 = a16 & a15;
        a18 = a17 << 3;
        a19 = a18 >> 2;
        a20 = a19 + a18;
        a21 = a20 - a19;
        a22 = a21 * a20;
        a23 = a22 / (a21 ? a21 : 1);
        a24 = a23 % (a22 ? a22 : 1);
        a25 = a24 ^ a23;
        a26 = a25 | a24;
        a27 = a26 & a25;
        a28 = a27 << 1;
        a29 = a28 >> 1;
        a30 = a29 + a28;
        a31 = a30 - a29;
        a32 = a31 * a30;
        a33 = a32 / (a31 ? a31 : 1);
        a34 = a33 % (a32 ? a32 : 1);
        a35 = a34 ^ a33;
        a36 = a35 | a34;
        a37 = a36 & a35;
        a38 = a37 << 2;
        a39 = a38 >> 1;
        a40 = a39 + a38;
        
        /* Circular dependency to prevent optimization */
        a1 = a40 + i;
    }
    
    /* Use all variables to prevent elimination */
    USE(a1); USE(a2); USE(a3); USE(a4); USE(a5);
    USE(a6); USE(a7); USE(a8); USE(a9); USE(a10);
    USE(a11); USE(a12); USE(a13); USE(a14); USE(a15);
    USE(a16); USE(a17); USE(a18); USE(a19); USE(a20);
    USE(a21); USE(a22); USE(a23); USE(a24); USE(a25);
    USE(a26); USE(a27); USE(a28); USE(a29); USE(a30);
    USE(a31); USE(a32); USE(a33); USE(a34); USE(a35);
    USE(a36); USE(a37); USE(a38); USE(a39); USE(a40);
    
    return a1 + a20 + a40;
}

/* ============================================
 * PATTERN B: Floating-point intensive computation
 * Pressures floating-point registers
 * ============================================ */
NOINLINE
double pattern_b_fp_pressure(double seed, int iterations) {
    /* 20+ double variables for FP register pressure */
    double b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    double b11, b12, b13, b14, b15, b16, b17, b18, b19, b20;
    double b21, b22, b23, b24, b25;
    
    b1 = seed;
    
    for (int i = 0; i < iterations; i++) {
        double t = (double)i;
        b2 = b1 + t;
        b3 = b2 * b1;
        b4 = b3 - b2;
        b5 = b4 / (b3 != 0.0 ? b3 : 1.0);
        b6 = b5 * b4;
        b7 = b6 + b5;
        b8 = b7 - b6;
        b9 = b8 * b7;
        b10 = b9 / (b8 != 0.0 ? b8 : 1.0);
        b11 = b10 + b9;
        b12 = b11 - b10;
        b13 = b12 * b11;
        b14 = b13 / (b12 != 0.0 ? b12 : 1.0);
        b15 = b14 + b13;
        b16 = b15 - b14;
        b17 = b16 * b15;
        b18 = b17 / (b16 != 0.0 ? b16 : 1.0);
        b19 = b18 + b17;
        b20 = b19 - b18;
        b21 = b20 * b19;
        b22 = b21 / (b20 != 0.0 ? b20 : 1.0);
        b23 = b22 + b21;
        b24 = b23 - b22;
        b25 = b24 * b23;
        
        b1 = b25 + t;
    }
    
    USE(b1); USE(b2); USE(b3); USE(b4); USE(b5);
    USE(b6); USE(b7); USE(b8); USE(b9); USE(b10);
    USE(b11); USE(b12); USE(b13); USE(b14); USE(b15);
    USE(b16); USE(b17); USE(b18); USE(b19); USE(b20);
    USE(b21); USE(b22); USE(b23); USE(b24); USE(b25);
    
    return b1 + b13 + b25;
}

/* ============================================
 * PATTERN C: Complex control flow with switch
 * Creates many basic blocks for CFG complexity
 * ============================================ */
NOINLINE
int pattern_c_complex_cfg(int seed, int iterations) {
    int result = seed;
    
    for (int i = 0; i < iterations; i++) {
        /* Switch with 20+ cases creates many basic blocks */
        switch (i % 23) {
            case 0: result += i * 2; break;
            case 1: result -= i * 3; break;
            case 2: result ^= i; break;
            case 3: result |= i << 1; break;
            case 4: result &= ~i; break;
            case 5: result = result * i + 1; break;
            case 6: result = result / (i ? i : 1); break;
            case 7: result = result % (i ? i : 1); break;
            case 8: result = result << (i % 4); break;
            case 9: result = result >> (i % 4); break;
            case 10: result = ~result; break;
            case 11: result = result + (i << 2); break;
            case 12: result = result - (i << 1); break;
            case 13: result = result ^ (i * 7); break;
            case 14: result = result | 0xAAAA; break;
            case 15: result = result & 0x5555; break;
            case 16: result = result * 3 + i; break;
            case 17: result = result / 2 - i; break;
            case 18: result = result % 100 + i; break;
            case 19: result = result << 2 | i; break;
            case 20: result = result >> 1 & i; break;
            case 21: result = ~result + i; break;
            case 22: result = result * result - i; break;
            default: result += 1; break;
        }
        
        /* Nested loop with break/continue for more CFG edges */
        for (int j = 0; j < 5; j++) {
            if (j == 2) continue;
            if (j == 4) break;
            result += j;
        }
    }
    
    USE(result);
    return result;
}

/* ============================================
 * PATTERN D: SIMD vector operations
 * Pressures vector/SIMD registers
 * ============================================ */
NOINLINE
v4si pattern_d_simd_pressure(v4si seed, int iterations) {
    v4si v1 = seed;
    v4si v2, v3, v4, v5, v6, v7, v8, v9, v10;
    v4si v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    
    v4si inc = {1, 2, 3, 4};
    
    for (int i = 0; i < iterations; i++) {
        v2 = v1 + inc;
        v3 = v2 * v1;
        v4 = v3 - v2;
        v5 = v4 & v3;
        v6 = v5 | v4;
        v7 = v6 ^ v5;
        v8 = v7 << 1;
        v9 = v8 >> 1;
        v10 = v9 + v8;
        v11 = v10 - v9;
        v12 = v11 * v10;
        v13 = v12 & v11;
        v14 = v13 | v12;
        v15 = v14 ^ v13;
        v16 = v15 << 2;
        v17 = v16 >> 1;
        v18 = v17 + v16;
        v19 = v18 - v17;
        v20 = v19 * v18;
        
        v1 = v20 + inc;
        inc = inc + (v4si){1, 1, 1, 1};
    }
    
    USE(v1); USE(v2); USE(v3); USE(v4); USE(v5);
    USE(v6); USE(v7); USE(v8); USE(v9); USE(v10);
    USE(v11); USE(v12); USE(v13); USE(v14); USE(v15);
    USE(v16); USE(v17); USE(v18); USE(v19); USE(v20);
    
    return v1 + v10 + v20;
}

/* ============================================
 * PATTERN E: Explicit register variables
 * Conflicts with allocator's choices
 * ============================================ */
NOINLINE
int pattern_e_explicit_registers(int seed, int iterations) {
    /* Try to use specific registers (GCC may ignore or conflict) */
    register int r1 asm ("r12") = seed;
    register int r2 asm ("r13") = seed + 1;
    register int r3 asm ("r14") = seed + 2;
    register int r4 asm ("r15") = seed + 3;
    register int r5 asm ("ebx") = seed + 4;
    
    int temp1, temp2, temp3, temp4, temp5;
    int temp6, temp7, temp8, temp9, temp10;
    
    for (int i = 0; i < iterations; i++) {
        /* Force spills by using many temporaries */
        temp1 = r1 + i;
        temp2 = r2 * temp1;
        temp3 = r3 - temp2;
        temp4 = r4 ^ temp3;
        temp5 = r5 | temp4;
        temp6 = temp1 + temp2;
        temp7 = temp3 * temp4;
        temp8 = temp5 ^ temp6;
        temp9 = temp7 - temp8;
        temp10 = temp9 / (temp8 ? temp8 : 1);
        
        /* Update register variables */
        r1 = temp10 + r1;
        r2 = temp9 + r2;
        r3 = temp8 + r3;
        r4 = temp7 + r4;
        r5 = temp6 + r5;
    }
    
    /* Mix register and stack variables */
    USE(r1); USE(r2); USE(r3); USE(r4); USE(r5);
    USE(temp1); USE(temp2); USE(temp3); USE(temp4); USE(temp5);
    USE(temp6); USE(temp7); USE(temp8); USE(temp9); USE(temp10);
    
    return r1 + r2 + r3 + r4 + r5;
}

/* ============================================
 * Helper to use CPU features (triggers target-specific RA)
 * ============================================ */
NOINLINE
int use_cpu_features(void) {
    int has_avx2 = __builtin_cpu_supports("avx2");
    int has_sse4 = __builtin_cpu_supports("sse4.2");
    int has_popcnt = __builtin_cpu_supports("popcnt");
    
    USE(has_avx2);
    USE(has_sse4);
    USE(has_popcnt);
    
    return has_avx2 + has_sse4 + has_popcnt;
}

/* ============================================
 * Main function (marked cold to affect block ordering)
 * ============================================ */
COLD
int main(int argc, char **argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 10) iterations = 10;
        if (iterations > 1000) iterations = 1000;
    }
    
    int total = 0;
    double fp_total = 0.0;
    
    /* Call each pattern multiple times with different seeds */
    for (int i = 0; i < 5; i++) {
        total += pattern_a_int_pressure(i * 7, iterations);
        fp_total += pattern_b_fp_pressure(i * 3.14, iterations / 2);
        total += pattern_c_complex_cfg(i * 11, iterations);
        
        v4si vec_seed = {i, i+1, i+2, i+3};
        v4si vec_result = pattern_d_simd_pressure(vec_seed, iterations / 4);
        total += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
        
        total += pattern_e_explicit_registers(i * 13, iterations / 2);
    }
    
    /* Trigger CPU feature detection */
    total += use_cpu_features();
    
    /* Use computed goto for additional CFG complexity */
    void* labels[] = {&&L0, &&L1, &&L2, &&L3, &&L4};
    
    for (int i = 0; i < 5; i++) {
        goto *labels[i % 5];
        
        L0: total += 1; continue;
        L1: total += 2; continue;
        L2: total += 3; continue;
        L3: total += 4; continue;
        L4: total += 5; continue;
    }
    
    /* Prevent optimization of final result */
    USE(total);
    USE(fp_total);
    
    printf("Result: %d (fp: %f)\n", total, fp_total);
    
    return total > 0 ? 0 : 1;
}
