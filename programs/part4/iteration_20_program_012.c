/* test_mcf_coverage.c
 * 
 * This test program is designed to trigger GCC's Min-Cost Flow pass
 * during register allocation, specifically to cover the special node
 * printing logic in mcf.cc's print_node function.
 *
 * Compile with: gcc -O3 -fprofile-arcs -ftest-coverage -fdump-rtl-all test_mcf_coverage.c -o test_mcf
 * Run with: ./test_mcf
 * Generate coverage with: gcov test_mcf_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))
#define COLD __attribute__((cold))

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
    int a31, a32, a33, a34, a35, a36, a37, a38, a39, a40;
    
    /* Complex interdependent computation to prevent optimization */
    a1 = input + 1;
    a2 = a1 * 2 - input;
    a3 = a2 + a1;
    a4 = a3 * a2;
    a5 = a4 - a3 + a2;
    a6 = a5 ^ a4;
    a7 = a6 * 3 + a5;
    a8 = a7 / 2 + a6;
    a9 = a8 | a7;
    a10 = a9 & a8;
    
    a11 = a10 << 1;
    a12 = a11 >> 1;
    a13 = a12 + a11 + a10;
    a14 = a13 * a12;
    a15 = a14 - a13;
    a16 = a15 ^ a14;
    a17 = a16 * 5;
    a18 = a17 + a16;
    a19 = a18 | a17;
    a20 = a19 & a18;
    
    a21 = a20 << 2;
    a22 = a21 >> 1;
    a23 = a22 + a21 + a20;
    a24 = a23 * a22;
    a25 = a24 - a23;
    a26 = a25 ^ a24;
    a27 = a26 * 7;
    a28 = a27 + a26;
    a29 = a28 | a27;
    a30 = a29 & a28;
    
    a31 = a30 << 3;
    a32 = a31 >> 2;
    a33 = a32 + a31 + a30;
    a34 = a33 * a32;
    a35 = a34 - a33;
    a36 = a35 ^ a34;
    a37 = a36 * 11;
    a38 = a37 + a36;
    a39 = a38 | a37;
    a40 = a39 & a38;
    
    /* Loop with all variables live to maximize pressure */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        /* Force all variables to be used in loop */
        asm volatile ("" : "+r"(a1), "+r"(a2), "+r"(a3), "+r"(a4), "+r"(a5));
        asm volatile ("" : "+r"(a6), "+r"(a7), "+r"(a8), "+r"(a9), "+r"(a10));
        asm volatile ("" : "+r"(a11), "+r"(a12), "+r"(a13), "+r"(a14), "+r"(a15));
        asm volatile ("" : "+r"(a16), "+r"(a17), "+r"(a18), "+r"(a19), "+r"(a20));
        asm volatile ("" : "+r"(a21), "+r"(a22), "+r"(a23), "+r"(a24), "+r"(a25));
        asm volatile ("" : "+r"(a26), "+r"(a27), "+r"(a28), "+r"(a29), "+r"(a30));
        asm volatile ("" : "+r"(a31), "+r"(a32), "+r"(a33), "+r"(a34), "+r"(a35));
        asm volatile ("" : "+r"(a36), "+r"(a37), "+r"(a38), "+r"(a39), "+r"(a40));
        
        /* Complex computation that uses all variables */
        sum += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
               a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19 + a20 +
               a21 + a22 + a23 + a24 + a25 + a26 + a27 + a28 + a29 + a30 +
               a31 + a32 + a33 + a34 + a35 + a36 + a37 + a38 + a39 + a40;
        
        /* Rotate values to create data dependencies */
        int temp = a40;
        a40 = a39; a39 = a38; a38 = a37; a37 = a36; a36 = a35;
        a35 = a34; a34 = a33; a33 = a32; a32 = a31; a31 = a30;
        a30 = a29; a29 = a28; a28 = a27; a27 = a26; a26 = a25;
        a25 = a24; a24 = a23; a23 = a22; a22 = a21; a21 = a20;
        a20 = a19; a19 = a18; a18 = a17; a17 = a16; a16 = a15;
        a15 = a14; a14 = a13; a13 = a12; a12 = a11; a11 = a10;
        a10 = a9; a9 = a8; a8 = a7; a7 = a6; a6 = a5;
        a5 = a4; a4 = a3; a3 = a2; a2 = a1; a1 = temp + i;
    }
    
    return sum;
}

/* ============================================================
 * PATTERN B: Floating-point intensive computation
 * Pressures floating-point and vector registers
 * ============================================================ */
NOINLINE double pattern_b_fp_pressure(double input) {
    /* 20+ double variables to pressure FP registers */
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    double d11, d12, d13, d14, d15, d16, d17, d18, d19, d20;
    double d21, d22, d23, d24, d25;
    
    d1 = input;
    d2 = d1 * 1.1;
    d3 = d2 + d1;
    d4 = d3 * 0.9;
    d5 = d4 - d3;
    d6 = d5 / 2.0;
    d7 = d6 * 3.14159;
    d8 = d7 + d6;
    d9 = d8 * 2.71828;
    d10 = d9 - d8;
    
    d11 = d10 * 1.41421;
    d12 = d11 + d10;
    d13 = d12 * 0.7071;
    d14 = d13 - d12;
    d15 = d14 / 3.0;
    d16 = d15 * 1.73205;
    d17 = d16 + d15;
    d18 = d17 * 0.57735;
    d19 = d18 - d17;
    d20 = d19 * 2.0;
    
    d21 = d20 + 1.0;
    d22 = d21 * 0.5;
    d23 = d22 + d21;
    d24 = d23 * 0.33333;
    d25 = d24 - d23;
    
    /* Loop with FP operations */
    double sum = 0.0;
    for (int i = 0; i < 50; i++) {
        /* Force all FP variables to be live */
        asm volatile ("" : "+f"(d1), "+f"(d2), "+f"(d3), "+f"(d4), "+f"(d5));
        asm volatile ("" : "+f"(d6), "+f"(d7), "+f"(d8), "+f"(d9), "+f"(d10));
        asm volatile ("" : "+f"(d11), "+f"(d12), "+f"(d13), "+f"(d14), "+f"(d15));
        asm volatile ("" : "+f"(d16), "+f"(d17), "+f"(d18), "+f"(d19), "+f"(d20));
        asm volatile ("" : "+f"(d21), "+f"(d22), "+f"(d23), "+f"(d24), "+f"(d25));
        
        /* Complex FP computation */
        sum += d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10 +
               d11 + d12 + d13 + d14 + d15 + d16 + d17 + d18 + d19 + d20 +
               d21 + d22 + d23 + d24 + d25;
        
        /* Update values to prevent dead code elimination */
        double temp = d25;
        d25 = d24 * 0.99; d24 = d23 * 1.01; d23 = d22 + 0.1; d22 = d21 - 0.1;
        d21 = d20 * 0.9; d20 = d19 * 1.1; d19 = d18 + 0.2; d18 = d17 - 0.2;
        d17 = d16 * 0.8; d16 = d15 * 1.2; d15 = d14 + 0.3; d14 = d13 - 0.3;
        d13 = d12 * 0.7; d12 = d11 * 1.3; d11 = d10 + 0.4; d10 = d9 - 0.4;
        d9 = d8 * 0.6; d8 = d7 * 1.4; d7 = d6 + 0.5; d6 = d5 - 0.5;
        d5 = d4 * 0.5; d4 = d3 * 1.5; d3 = d2 + 0.6; d2 = d1 - 0.6;
        d1 = temp + i * 0.01;
    }
    
    return sum;
}

/* ============================================================
 * PATTERN C: Complex control flow with switch statement
 * Creates many basic blocks for complex CFG
 * ============================================================ */
NOINLINE int pattern_c_complex_cfg(int input) {
    int result = 0;
    
    /* Nested loops with breaks/continues */
    for (int i = 0; i < 20; i++) {
        if (i % 3 == 0) continue;
        
        for (int j = 0; j < 15; j++) {
            if (j == i) break;
            
            /* Switch with many cases creates multiple basic blocks */
            switch ((i * j + input) % 25) {
                case 0:  result += i + j; break;
                case 1:  result += i - j; break;
                case 2:  result += i * j; break;
                case 3:  result += i ^ j; break;
                case 4:  result += i | j; break;
                case 5:  result += i & j; break;
                case 6:  result += i << 1; break;
                case 7:  result += j >> 1; break;
                case 8:  result += ~i; break;
                case 9:  result += ~j; break;
                case 10: result += i * 2 + j; break;
                case 11: result += i + j * 2; break;
                case 12: result += i * 3 - j; break;
                case 13: result += i - j * 3; break;
                case 14: result += (i << 2) | j; break;
                case 15: result += i | (j << 2); break;
                case 16: result += (i & 0xF) + j; break;
                case 17: result += i + (j & 0xF); break;
                case 18: result += (i ^ 0xFF) - j; break;
                case 19: result += i - (j ^ 0xFF); break;
                case 20: result += (i % 5) * j; break;
                case 21: result += i * (j % 5); break;
                case 22: result += (i / 2) + (j / 2); break;
                case 23: result += (i + 1) * (j - 1); break;
                case 24: result += (i - 1) * (j + 1); break;
                default: result += 1; break;
            }
            
            /* Additional control flow */
            if (result > 1000) {
                goto early_exit;
            }
        }
        
        /* Another level of control complexity */
        int k = 0;
        while (k < 5) {
            result += k;
            if (result % 7 == 0) {
                k += 2;
                continue;
            }
            k++;
        }
    }
    
early_exit:
    return result;
}

/* ============================================================
 * PATTERN D: SIMD vector operations
 * Pressures vector/SIMD registers
 * ============================================================ */
NOINLINE v4si pattern_d_simd_pressure(v4si input) {
    v4si v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    v4si v11, v12, v13, v14, v15;
    
    /* Initialize vectors */
    v1 = input;
    v2 = v1 + (v4si){1, 2, 3, 4};
    v3 = v2 * (v4si){2, 2, 2, 2};
    v4 = v3 - v2;
    v5 = v4 & (v4si){0xFF, 0xFF, 0xFF, 0xFF};
    v6 = v5 | (v4si){0x01, 0x01, 0x01, 0x01};
    v7 = v6 ^ (v4si){0x0F, 0x0F, 0x0F, 0x0F};
    v8 = v7 << 1;
    v9 = v8 >> 1;
    v10 = v9 + v8;
    
    v11 = v10 * (v4si){3, 3, 3, 3};
    v12 = v11 - v10;
    v13 = v12 & (v4si){0x7F, 0x7F, 0x7F, 0x7F};
    v14 = v13 | (v4si){0x80, 0x80, 0x80, 0x80};
    v15 = v14 ^ (v4si){0xFF, 0xFF, 0xFF, 0xFF};
    
    /* Loop with vector operations */
    v4si result = {0, 0, 0, 0};
    for (int i = 0; i < 30; i++) {
        /* Force all vectors to be live */
        asm volatile ("" : "+x"(v1), "+x"(v2), "+x"(v3));
        asm volatile ("" : "+x"(v4), "+x"(v5), "+x"(v6));
        asm volatile ("" : "+x"(v7), "+x"(v8), "+x"(v9));
        asm volatile ("" : "+x"(v10), "+x"(v11), "+x"(v12));
        asm volatile ("" : "+x"(v13), "+x"(v14), "+x"(v15));
        
        /* Complex vector computation */
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                  v11 + v12 + v13 + v14 + v15;
        
        /* Rotate vectors */
        v4si temp = v15;
        v15 = v14; v14 = v13; v13 = v12; v12 = v11; v11 = v10;
        v10 = v9; v9 = v8; v8 = v7; v7 = v6; v6 = v5;
        v5 = v4; v4 = v3; v3 = v2; v2 = v1; 
        v1 = temp + (v4si){i, i+1, i+2, i+3};
    }
    
    return result;
}

/* ============================================================
 * PATTERN E: Explicit register variables
 * Conflicts with register allocator choices
 * ============================================================ */
NOINLINE int pattern_e_explicit_registers(int input) {
    /* Explicit register variables that conflict with allocator */
    register int r1 asm ("r12") = input;
    register int r2 asm ("r13") = input + 1;
    register int r3 asm ("r14") = input + 2;
    register int r4 asm ("r15") = input + 3;
    
    int a, b, c, d, e, f, g, h, i, j;
    int k, l, m, n, o, p, q, r, s, t;
    
    /* Force interaction between explicit and automatic registers */
    a = r1 * 2;
    b = r2 + a;
    c = r3 - b;
    d = r4 * c;
    e = a ^ b;
    f = c | d;
    g = e & f;
    h = r1 << 2;
    i = r2 >> 1;
    j = r3 + r4;
    
    k = a + b + c;
    l = d - e - f;
    m = g * h * i;
    n = j ^ k ^ l;
    o = m | n;
    p = r1 & r2 & r3;
    q = r4 + a + b;
    r = c * d * e;
    s = f ^ g ^ h;
    t = i | j | k;
    
    /* Complex computation mixing all variables */
    int result = 0;
    for (int idx = 0; idx < 40; idx++) {
        /* Force explicit registers to stay live */
        asm volatile ("" : "+r"(r1), "+r"(r2), "+r"(r3), "+r"(r4));
        
        result += r1 + r2 + r3 + r4 + a + b + c + d + e + f +
                  g + h + i + j + k + l + m + n + o + p +
                  q + r + s + t;
        
        /* Update values */
        int temp = r4;
        r4 = r3 + idx; r3 = r2 - idx; r2 = r1 * idx; r1 = temp;
        a = b; b = c; c = d; d = e; e = f; f = g; g = h; h = i; i = j;
        j = k; k = l; l = m; m = n; n = o; o = p; p = q; q = r; r = s; s = t;
        t = result % 100;
    }
    
    return result;
}

/* ============================================================
 * Helper function to use CPU features
 * Ensures target-specific optimizations are engaged
 * ============================================================ */
NOINLINE int use_cpu_features(void) {
    int has_avx2 = 0;
    int has_sse4 = 0;
    
    /* Use GCC builtins to engage target-specific optimization passes */
#ifdef __GNUC__
    #ifdef __x86_64__
    has_avx2 = __builtin_cpu_supports("avx2");
    has_sse4 = __builtin_cpu_supports("sse4.2");
    #endif
#endif
    
    return has_avx2 + has_sse4;
}

/* ============================================================
 * Main function - calls all patterns with varying inputs
 * ============================================================ */
COLD int main(int argc, char **argv) {
    int total = 0;
    
    /* Use CPU features to engage target-specific passes */
    int cpu_features = use_cpu_features();
    total += cpu_features;
    
    /* Array of inputs to prevent constant folding */
    int int_inputs[] = {1, 2, 3, 5, 7, 11, 13, 17, 19, 23};
    double fp_inputs[] = {1.0, 2.5, 3.14, 5.67, 8.91, 12.34};
    v4si vec_input = {1, 2, 3, 4};
    
    /* Call pattern A multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        total += pattern_a_int_pressure(int_inputs[i % 10]);
    }
    
    /* Call pattern B with FP inputs */
    for (int i = 0; i < 6; i++) {
        double fp_result = pattern_b_fp_pressure(fp_inputs[i]);
        total += (int)fp_result;
    }
    
    /* Call pattern C with complex CFG */
    for (int i = 0; i < 8; i++) {
        total += pattern_c_complex_cfg(int_inputs[i % 10]);
    }
    
    /* Call pattern D with SIMD */
    for (int i = 0; i < 5; i++) {
        v4si vec_result = pattern_d_simd_pressure(vec_input);
        total += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
        vec_input[0]++; vec_input[1]++; vec_input[2]++; vec_input[3]++;
    }
    
    /* Call pattern E with explicit registers */
    for (int i = 0; i < 7; i++) {
        total += pattern_e_explicit_registers(int_inputs[i % 10]);
    }
    
    /* Print result to prevent dead code elimination */
    printf("Total result: %d\n", total % 1000);
    
    return 0;
}
