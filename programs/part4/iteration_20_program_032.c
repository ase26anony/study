/* test_mcf_coverage.c
 * 
 * This test program is designed to trigger GCC's Min-Cost Flow pass
 * during register allocation, specifically to cover the special node
 * printing logic in mcf.cc's print_node function.
 *
 * Compile with: gcc -O3 -fprofile-arcs -ftest-coverage -fdump-rtl-all -c test_mcf_coverage.c
 * Then run: ./test_mcf_coverage (to generate profile data)
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))
#define COLD __attribute__((cold))

/* Vector type for SIMD pressure */
typedef int v4si __attribute__((vector_size(16)));

/* ============================================
 * PATTERN A: Integer arithmetic chain with 30+ variables
 * Creates massive register pressure in a loop
 * ============================================ */
NOINLINE int pattern_a_int_pressure(int input) {
    /* 30+ integer variables to pressure general purpose registers */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int a11, a12, a13, a14, a15, a16, a17, a18, a19, a20;
    int a21, a22, a23, a24, a25, a26, a27, a28, a29, a30;
    int a31, a32, a33, a34, a35, a36, a37, a38, a39, a40;
    
    /* Complex interdependent calculations */
    a1 = input + 1;
    a2 = a1 * 2 - input;
    a3 = a2 + a1;
    a4 = a3 * 3 - a2;
    a5 = a4 + a3 + a2;
    a6 = a5 * 2 - a4;
    a7 = a6 + a5 + a4;
    a8 = a7 * 3 - a6;
    a9 = a8 + a7 + a6;
    a10 = a9 * 2 - a8;
    
    a11 = a10 + a9 + a8;
    a12 = a11 * 3 - a10;
    a13 = a12 + a11 + a10;
    a14 = a13 * 2 - a12;
    a15 = a14 + a13 + a12;
    a16 = a15 * 3 - a14;
    a17 = a16 + a15 + a14;
    a18 = a17 * 2 - a16;
    a19 = a18 + a17 + a16;
    a20 = a19 * 3 - a18;
    
    a21 = a20 + a19 + a18;
    a22 = a21 * 2 - a20;
    a23 = a22 + a21 + a20;
    a24 = a23 * 3 - a22;
    a25 = a24 + a23 + a22;
    a26 = a25 * 2 - a24;
    a27 = a26 + a25 + a24;
    a28 = a27 * 3 - a26;
    a29 = a28 + a27 + a26;
    a30 = a29 * 2 - a28;
    
    a31 = a30 + a29 + a28;
    a32 = a31 * 3 - a30;
    a33 = a32 + a31 + a30;
    a34 = a33 * 2 - a32;
    a35 = a34 + a33 + a32;
    a36 = a35 * 3 - a34;
    a37 = a36 + a35 + a34;
    a38 = a37 * 2 - a36;
    a39 = a38 + a37 + a36;
    a40 = a39 * 3 - a38;
    
    /* Loop with complex control flow to create CFG edges */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        /* Use all variables in the loop to prevent optimization */
        if (i & 1) {
            a1 += a2;
            a3 += a4;
            a5 += a6;
            a7 += a8;
            a9 += a10;
        } else {
            a11 += a12;
            a13 += a14;
            a15 += a16;
            a17 += a18;
            a19 += a20;
        }
        
        /* Nested condition for more basic blocks */
        switch (i % 5) {
            case 0: a21 += a22; break;
            case 1: a23 += a24; break;
            case 2: a25 += a26; break;
            case 3: a27 += a28; break;
            case 4: a29 += a30; break;
        }
        
        /* Prevent dead code elimination */
        asm volatile("" : "+r"(a31), "+r"(a32), "+r"(a33), "+r"(a34));
        asm volatile("" : "+r"(a35), "+r"(a36), "+r"(a37), "+r"(a38));
        
        sum += a1 + a3 + a5 + a7 + a9 + a11 + a13 + a15 + a17 + a19 +
               a21 + a23 + a25 + a27 + a29 + a31 + a33 + a35 + a37 + a39;
    }
    
    return sum + a40;
}

/* ============================================
 * PATTERN B: Floating-point intensive computation
 * Pressures floating-point registers
 * ============================================ */
NOINLINE double pattern_b_fp_pressure(double input) {
    /* 20+ double variables */
    double b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    double b11, b12, b13, b14, b15, b16, b17, b18, b19, b20;
    double b21, b22, b23, b24, b25;
    
    b1 = input;
    b2 = b1 * 1.1;
    b3 = b2 + b1;
    b4 = b3 * 1.2;
    b5 = b4 - b3;
    b6 = b5 * 1.3;
    b7 = b6 / (b5 + 1.0);
    b8 = b7 * 1.4;
    b9 = b8 + b7;
    b10 = b9 * 1.5;
    
    b11 = b10 - b9;
    b12 = b11 * 1.6;
    b13 = b12 + b11;
    b14 = b13 * 1.7;
    b15 = b14 - b13;
    b16 = b15 * 1.8;
    b17 = b16 / (b15 + 1.0);
    b18 = b17 * 1.9;
    b19 = b18 + b17;
    b20 = b19 * 2.0;
    
    b21 = b20 - b19;
    b22 = b21 * 2.1;
    b23 = b22 + b21;
    b24 = b23 * 2.2;
    b25 = b24 - b23;
    
    /* Complex loop with floating-point operations */
    double result = 0.0;
    for (int i = 0; i < 50; i++) {
        /* Mix of operations to use different FP units */
        if (i % 3 == 0) {
            b1 = b2 * b3;
            b4 = b5 + b6;
            b7 = b8 - b9;
        } else if (i % 3 == 1) {
            b10 = b11 / b12;
            b13 = b14 * b15;
            b16 = b17 + b18;
        } else {
            b19 = b20 - b21;
            b22 = b23 * b24;
            b25 = b1 + b2;
        }
        
        /* Prevent optimization */
        asm volatile("" : "+x"(b1), "+x"(b2), "+x"(b3), "+x"(b4));
        asm volatile("" : "+x"(b5), "+x"(b6), "+x"(b7), "+x"(b8));
        
        result += b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9 + b10 +
                  b11 + b12 + b13 + b14 + b15 + b16 + b17 + b18 + b19 + b20 +
                  b21 + b22 + b23 + b24 + b25;
    }
    
    return result;
}

/* ============================================
 * PATTERN C: Complex CFG with switch statement
 * Creates many basic blocks for MCF to analyze
 * ============================================ */
NOINLINE int pattern_c_complex_cfg(int input) {
    int result = input;
    
    /* Switch with many cases creates many basic blocks */
    for (int i = 0; i < 100; i++) {
        switch (i % 20) {
            case 0:  result += i * 2; break;
            case 1:  result -= i * 3; break;
            case 2:  result *= (i + 1); break;
            case 3:  result /= (i + 2); break;
            case 4:  result ^= i; break;
            case 5:  result |= (i << 1); break;
            case 6:  result &= ~i; break;
            case 7:  result = (result << 3) | i; break;
            case 8:  result = (result >> 2) ^ i; break;
            case 9:  result = result + (i * i); break;
            case 10: result = result - (i / 2); break;
            case 11: result = result * (i % 7); break;
            case 12: result = result ^ (i << 2); break;
            case 13: result = result | (0xFF & i); break;
            case 14: result = result & (0xF0 | i); break;
            case 15: result = (result << 1) + i; break;
            case 16: result = (result >> 1) - i; break;
            case 17: result = result + (i << 4); break;
            case 18: result = result - (i >> 2); break;
            case 19: result = result * ((i % 5) + 1); break;
        }
        
        /* Nested loops with break/continue for more CFG complexity */
        for (int j = 0; j < 10; j++) {
            if (j == 5) continue;
            if (j == 8) break;
            result += j;
        }
    }
    
    return result;
}

/* ============================================
 * PATTERN D: SIMD vector operations
 * Pressures vector/SIMD registers
 * ============================================ */
NOINLINE v4si pattern_d_simd_pressure(v4si input) {
    v4si v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    v4si v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    
    /* Initialize vectors */
    v1 = input + (v4si){1, 2, 3, 4};
    v2 = v1 * (v4si){2, 3, 4, 5};
    v3 = v2 + v1;
    v4 = v3 * (v4si){3, 4, 5, 6};
    v5 = v4 - v3;
    v6 = v5 * (v4si){4, 5, 6, 7};
    v7 = v6 + v5;
    v8 = v7 * (v4si){5, 6, 7, 8};
    v9 = v8 - v7;
    v10 = v9 * (v4si){6, 7, 8, 9};
    
    v11 = v10 + v9;
    v12 = v11 * (v4si){7, 8, 9, 10};
    v13 = v12 - v11;
    v14 = v13 * (v4si){8, 9, 10, 11};
    v15 = v14 + v13;
    v16 = v15 * (v4si){9, 10, 11, 12};
    v17 = v16 - v15;
    v18 = v17 * (v4si){10, 11, 12, 13};
    v19 = v18 + v17;
    v20 = v19 * (v4si){11, 12, 13, 14};
    
    /* Loop with vector operations */
    v4si result = {0, 0, 0, 0};
    for (int i = 0; i < 50; i++) {
        /* Mix vector operations */
        if (i & 1) {
            v1 = v2 + v3;
            v4 = v5 * v6;
            v7 = v8 - v9;
        } else {
            v10 = v11 + v12;
            v13 = v14 * v15;
            v16 = v17 - v18;
        }
        
        /* Prevent dead code elimination */
        asm volatile("" : "+x"(v1), "+x"(v2), "+x"(v3));
        asm volatile("" : "+x"(v4), "+x"(v5), "+x"(v6));
        
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                  v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    }
    
    return result;
}

/* ============================================
 * PATTERN E: Explicit register variables
 * Conflicts with allocator's choices
 * ============================================ */
NOINLINE int pattern_e_explicit_registers(int input) {
    /* Explicit register variables that conflict with allocator */
    register int r1 asm ("r12") = input;
    register int r2 asm ("r13") = input * 2;
    register int r3 asm ("r14") = input * 3;
    register int r4 asm ("r15") = input * 4;
    
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    
    /* Force interaction between explicit and automatic variables */
    v1 = r1 + 1;
    v2 = r2 + v1;
    v3 = r3 + v2;
    v4 = r4 + v3;
    v5 = v1 * v2;
    v6 = v3 * v4;
    v7 = v5 + v6;
    v8 = v7 - r1;
    v9 = v8 * r2;
    v10 = v9 / (r3 + 1);
    
    /* Complex loop that uses both register and stack variables */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        /* Update register variables */
        asm volatile("add %1, %0" : "+r"(r1) : "r"(i));
        asm volatile("sub %1, %0" : "+r"(r2) : "r"(i));
        
        /* Update automatic variables */
        v1 += r1;
        v2 += r2;
        v3 += r3;
        v4 += r4;
        
        /* Mix them together */
        v5 = v1 * r1;
        v6 = v2 * r2;
        v7 = v3 * r3;
        v8 = v4 * r4;
        
        sum += v5 + v6 + v7 + v8 + v9 + v10;
    }
    
    return sum + r1 + r2 + r3 + r4;
}

/* ============================================
 * Helper function to use __builtin_cpu_supports
 * Ensures target-specific optimizations are engaged
 * ============================================ */
NOINLINE int use_cpu_features(void) {
    int has_avx2 = 0;
    int has_sse4 = 0;
    
    /* Check CPU features to engage target-specific optimizations */
#ifdef __GNUC__
    has_avx2 = __builtin_cpu_supports("avx2");
    has_sse4 = __builtin_cpu_supports("sse4.2");
#else
    /* Stub for non-GCC compilers */
    has_avx2 = 0;
    has_sse4 = 0;
#endif
    
    return has_avx2 + has_sse4;
}

/* ============================================
 * Main function - cold attribute affects block ordering
 * ============================================ */
COLD int main(int argc, char **argv) {
    int result = 0;
    double fp_result = 0.0;
    v4si vec_result;
    int vec_sum = 0;
    
    /* Use CPU features to ensure target-specific pipeline */
    int cpu_features = use_cpu_features();
    (void)cpu_features; /* Prevent unused warning */
    
    /* Array of inputs to prevent constant folding */
    int inputs[] = {1, 2, 3, 5, 7, 11, 13, 17, 19, 23};
    double fp_inputs[] = {1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1};
    v4si vec_inputs[] = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12},
        {13, 14, 15, 16}
    };
    
    /* Call all patterns multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        /* Pattern A - Integer pressure */
        result += pattern_a_int_pressure(inputs[i % 10]);
        
        /* Pattern B - Floating-point pressure */
        fp_result += pattern_b_fp_pressure(fp_inputs[i % 10]);
        
        /* Pattern C - Complex CFG */
        result += pattern_c_complex_cfg(inputs[i % 10]);
        
        /* Pattern D - SIMD pressure */
        vec_result = pattern_d_simd_pressure(vec_inputs[i % 4]);
        for (int j = 0; j < 4; j++) {
            vec_sum += vec_result[j];
        }
        
        /* Pattern E - Explicit registers */
        result += pattern_e_explicit_registers(inputs[i % 10]);
    }
    
    /* Mix results to prevent dead code elimination */
    result += (int)fp_result + vec_sum;
    
    /* Optional debug output - doesn't affect coverage collection */
    if (argc > 1) {
        printf("Result: %d\n", result);
        printf("FP Result: %f\n", fp_result);
        printf("Vector Sum: %d\n", vec_sum);
    }
    
    return result & 0xFF; /* Return non-zero to indicate success */
}
