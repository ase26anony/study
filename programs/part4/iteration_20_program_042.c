/* test_mcf_coverage.c
 * 
 * This test program is designed to trigger GCC's Min-Cost Flow pass
 * during register allocation, specifically to exercise the print_node
 * function with special node indices: ENTRY_BLOCK, EXIT_BLOCK,
 * new_exit_index, and new_entry_index.
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

/* Stub for non-GCC compilers */
#ifndef __GNUC__
#define __builtin_cpu_supports(x) 0
#endif

/* Prevent dead code elimination */
#define USE(var) asm volatile("" : : "r"(var))

/* Vector type for SIMD pressure */
typedef int v4si __attribute__((vector_size(16)));

/* ============================================================
 * PATTERN A: Integer arithmetic chain with 30+ variables
 * Creates massive register pressure in a tight loop
 * ============================================================ */
NOINLINE int pattern_a_int_pressure(int input) {
    /* 30+ integer variables to pressure general purpose registers */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int a11, a12, a13, a14, a15, a16, a17, a18, a19, a20;
    int a21, a22, a23, a24, a25, a26, a27, a28, a29, a30;
    int result = 0;
    
    /* Initialize with input to prevent constant folding */
    a1 = input + 1;
    a2 = a1 * 2;
    a3 = a2 - input;
    a4 = a3 + a1;
    a5 = a4 * a2;
    a6 = a5 / (input + 2);
    a7 = a6 ^ a3;
    a8 = a7 | a4;
    a9 = a8 & a5;
    a10 = a9 << 2;
    a11 = a10 >> 1;
    a12 = a11 + a6;
    a13 = a12 - a7;
    a14 = a13 * a8;
    a15 = a14 / (a9 + 1);
    a16 = a15 ^ a10;
    a17 = a16 | a11;
    a18 = a17 & a12;
    a19 = a18 << 3;
    a20 = a19 >> 2;
    a21 = a20 + a13;
    a22 = a21 - a14;
    a23 = a22 * a15;
    a24 = a23 / (a16 + 1);
    a25 = a24 ^ a17;
    a26 = a25 | a18;
    a27 = a26 & a19;
    a28 = a27 << 1;
    a29 = a28 >> 1;
    a30 = a29 + a20;
    
    /* Complex loop with interdependencies */
    for (int i = 0; i < 100; i++) {
        a1 = a30 + i;
        a2 = a1 * a29;
        a3 = a2 - a28;
        a4 = a3 + a27;
        a5 = a4 * a26;
        a6 = a5 / (a25 + i + 1);
        a7 = a6 ^ a24;
        a8 = a7 | a23;
        a9 = a8 & a22;
        a10 = a9 << (i & 3);
        
        /* Force spill decisions with many live variables */
        result += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
        
        /* Rotate values to keep all variables live */
        a30 = a29; a29 = a28; a28 = a27; a27 = a26; a26 = a25;
        a25 = a24; a24 = a23; a23 = a22; a22 = a21; a21 = a20;
        a20 = a19; a19 = a18; a18 = a17; a17 = a16; a16 = a15;
        a15 = a14; a14 = a13; a13 = a12; a12 = a11; a11 = a10;
    }
    
    /* Use all variables to prevent optimization */
    USE(a1); USE(a2); USE(a3); USE(a4); USE(a5);
    USE(a6); USE(a7); USE(a8); USE(a9); USE(a10);
    USE(a11); USE(a12); USE(a13); USE(a14); USE(a15);
    USE(a16); USE(a17); USE(a18); USE(a19); USE(a20);
    USE(a21); USE(a22); USE(a23); USE(a24); USE(a25);
    USE(a26); USE(a27); USE(a28); USE(a29); USE(a30);
    
    return result;
}

/* ============================================================
 * PATTERN B: Floating-point intensive computation
 * Pressures floating-point and vector registers
 * ============================================================ */
NOINLINE double pattern_b_fp_pressure(double input) {
    /* 20+ double variables */
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    double d11, d12, d13, d14, d15, d16, d17, d18, d19, d20;
    double result = 0.0;
    
    d1 = input + 1.0;
    d2 = d1 * 2.0;
    d3 = d2 - input;
    d4 = d3 + d1;
    d5 = d4 * d2;
    d6 = d5 / (input + 2.0);
    d7 = d6 * d3;
    d8 = d7 + d4;
    d9 = d8 * d5;
    d10 = d9 / d6;
    d11 = d10 + d7;
    d12 = d11 * d8;
    d13 = d12 / d9;
    d14 = d13 + d10;
    d15 = d14 * d11;
    d16 = d15 / d12;
    d17 = d16 + d13;
    d18 = d17 * d14;
    d19 = d18 / d15;
    d20 = d19 + d16;
    
    /* Loop with floating-point operations */
    for (int i = 0; i < 50; i++) {
        double t = (double)i;
        d1 = d20 + t;
        d2 = d1 * d19;
        d3 = d2 - d18;
        d4 = d3 + d17;
        d5 = d4 * d16;
        d6 = d5 / (d15 + t + 1.0);
        
        /* Mix operations to prevent optimization */
        result += d1 * 0.1 + d2 * 0.2 + d3 * 0.3 + d4 * 0.4 + d5 * 0.5 + d6 * 0.6;
        
        /* Rotate to keep variables live */
        d20 = d19; d19 = d18; d18 = d17; d17 = d16; d16 = d15;
        d15 = d14; d14 = d13; d13 = d12; d12 = d11; d11 = d10;
        d10 = d9; d9 = d8; d8 = d7; d7 = d6; d6 = d5;
    }
    
    USE(d1); USE(d2); USE(d3); USE(d4); USE(d5);
    USE(d6); USE(d7); USE(d8); USE(d9); USE(d10);
    USE(d11); USE(d12); USE(d13); USE(d14); USE(d15);
    USE(d16); USE(d17); USE(d18); USE(d19); USE(d20);
    
    return result;
}

/* ============================================================
 * PATTERN C: Complex control flow with switch statement
 * Creates many basic blocks for CFG complexity
 * ============================================================ */
NOINLINE int pattern_c_complex_cfg(int input) {
    int result = input;
    
    /* Nested loops with breaks and continues */
    for (int i = 0; i < 20; i++) {
        if (i % 3 == 0) continue;
        
        for (int j = 0; j < 15; j++) {
            if (j == i) break;
            
            /* Switch with many cases - creates many basic blocks */
            switch ((i + j) % 21) {
                case 0:  result += 1; break;
                case 1:  result -= 2; break;
                case 2:  result *= 3; break;
                case 3:  result /= 4; break;
                case 4:  result ^= 5; break;
                case 5:  result |= 6; break;
                case 6:  result &= 7; break;
                case 7:  result <<= 1; break;
                case 8:  result >>= 2; break;
                case 9:  result += i; break;
                case 10: result -= j; break;
                case 11: result *= (i + 1); break;
                case 12: result /= (j + 1); break;
                case 13: result ^= i; break;
                case 14: result |= j; break;
                case 15: result &= (i | j); break;
                case 16: result <<= (i & 3); break;
                case 17: result >>= (j & 3); break;
                case 18: result = ~result; break;
                case 19: result = -result; break;
                case 20: result = result + result; break;
                default: result += 100; break;
            }
            
            /* Additional control flow */
            if (result > 1000) {
                result /= 2;
                continue;
            }
        }
        
        if (result < 0) {
            result = -result;
            break;
        }
    }
    
    return result;
}

/* ============================================================
 * PATTERN D: SIMD vector operations
 * Pressures vector/SIMD registers
 * ============================================================ */
NOINLINE v4si pattern_d_simd_pressure(v4si input) {
    v4si v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    v4si result = {0, 0, 0, 0};
    
    v1 = input + (v4si){1, 2, 3, 4};
    v2 = v1 * (v4si){2, 3, 4, 5};
    v3 = v2 - input;
    v4 = v3 + v1;
    v5 = v4 * v2;
    v6 = v5 / (input + (v4si){2, 3, 4, 5});
    v7 = v6 ^ v3;
    v8 = v7 | v4;
    v9 = v8 & v5;
    v10 = v9 << (v4si){1, 2, 1, 2};
    
    /* Loop with vector operations */
    for (int i = 0; i < 25; i++) {
        v4si idx = {i, i+1, i+2, i+3};
        v1 = v10 + idx;
        v2 = v1 * v9;
        v3 = v2 - v8;
        v4 = v3 + v7;
        v5 = v4 * v6;
        
        result = result + v1 + v2 + v3 + v4 + v5;
        
        /* Rotate vector variables */
        v10 = v9; v9 = v8; v8 = v7; v7 = v6; v6 = v5;
    }
    
    USE(v1); USE(v2); USE(v3); USE(v4); USE(v5);
    USE(v6); USE(v7); USE(v8); USE(v9); USE(v10);
    
    return result;
}

/* ============================================================
 * PATTERN E: Explicit register variables
 * Conflicts with allocator's choices
 * ============================================================ */
NOINLINE int pattern_e_register_conflict(int input) {
    /* Explicit register variables that may conflict */
    register int r1 asm ("r12") = input + 1;
    register int r2 asm ("r13") = r1 * 2;
    register int r3 asm ("r14") = r2 - input;
    register int r4 asm ("r15") = r3 + r1;
    
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int result = 0;
    
    /* Mix explicit and implicit registers */
    a1 = r4 * 3;
    a2 = a1 + r3;
    a3 = a2 - r2;
    a4 = a3 + r1;
    a5 = a4 * input;
    a6 = a5 / (r4 + 1);
    a7 = a6 ^ r3;
    a8 = a7 | r2;
    a9 = a8 & r1;
    a10 = a9 << 2;
    
    /* Loop that uses both register types */
    for (int i = 0; i < 30; i++) {
        r1 = a10 + i;
        r2 = r1 * a9;
        r3 = r2 - a8;
        r4 = r3 + a7;
        
        a1 = r4 * a6;
        a2 = a1 + r3;
        a3 = a2 - r2;
        a4 = a3 + r1;
        
        result += r1 + r2 + r3 + r4 + a1 + a2 + a3 + a4;
        
        /* Force register shuffling */
        asm volatile("" : "+r"(r1), "+r"(r2), "+r"(r3), "+r"(r4));
    }
    
    USE(a1); USE(a2); USE(a3); USE(a4); USE(a5);
    USE(a6); USE(a7); USE(a8); USE(a9); USE(a10);
    
    return result + r1 + r2 + r3 + r4;
}

/* ============================================================
 * Main function - calls all patterns with varying inputs
 * ============================================================ */
COLD int main(int argc, char **argv) {
    int int_result = 0;
    double fp_result = 0.0;
    v4si vec_result;
    int cfg_result = 0;
    int reg_result = 0;
    
    /* Use CPU feature check to engage target-specific optimizations */
    int use_avx = __builtin_cpu_supports("avx2");
    int use_sse = __builtin_cpu_supports("sse4.2");
    
    /* Array of inputs to prevent constant folding */
    int int_inputs[] = {1, 2, 3, 5, 7, 11, 13, 17, 19, 23};
    double fp_inputs[] = {1.1, 2.2, 3.3, 5.5, 7.7, 11.11};
    v4si vec_inputs[] = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12}
    };
    
    /* Call pattern A multiple times */
    for (int i = 0; i < 10; i++) {
        int_result ^= pattern_a_int_pressure(int_inputs[i % 10] + i);
    }
    
    /* Call pattern B */
    for (int i = 0; i < 6; i++) {
        fp_result += pattern_b_fp_pressure(fp_inputs[i] + (double)i * 0.1);
    }
    
    /* Call pattern C with complex control flow */
    for (int i = 0; i < 10; i++) {
        cfg_result += pattern_c_complex_cfg(int_inputs[i % 10] * i);
    }
    
    /* Call pattern D if SIMD is available */
    if (use_sse || use_avx) {
        for (int i = 0; i < 3; i++) {
            vec_result = pattern_d_simd_pressure(vec_inputs[i]);
            int_result += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
        }
    }
    
    /* Call pattern E */
    for (int i = 0; i < 8; i++) {
        reg_result += pattern_e_register_conflict(int_inputs[i % 10] + i * 3);
    }
    
    /* Combine results to prevent dead code elimination */
    int final_result = int_result + (int)fp_result + cfg_result + reg_result;
    
    /* Optional debug output - doesn't affect coverage */
    if (argc > 1) {
        printf("Results: int=%d, fp=%.2f, cfg=%d, reg=%d, final=%d\n",
               int_result, fp_result, cfg_result, reg_result, final_result);
    }
    
    /* Use the result to prevent optimization */
    USE(final_result);
    
    return final_result % 256;
}
