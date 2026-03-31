/* test_mcf_coverage.c
 * Designed to trigger GCC's Min-Cost Flow pass with special node indices
 * that will exercise the print_node function's uncovered lines.
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
    
    volatile int result = 0; /* volatile to prevent optimization */
    
    /* Complex interdependent calculations */
    a1 = input + 1;
    a2 = a1 * 2;
    a3 = a2 - a1;
    a4 = a3 + input;
    a5 = a4 * a2;
    a6 = a5 / (a1 + 1);
    a7 = a6 ^ a3;
    a8 = a7 | a4;
    a9 = a8 & a5;
    a10 = a9 << 2;
    
    a11 = a10 >> 1;
    a12 = a11 + a2;
    a13 = a12 - a3;
    a14 = a13 * a4;
    a15 = a14 / a5;
    a16 = a15 ^ a6;
    a17 = a16 | a7;
    a18 = a17 & a8;
    a19 = a18 << 3;
    a20 = a19 >> 2;
    
    a21 = a20 + a9;
    a22 = a21 - a10;
    a23 = a22 * a11;
    a24 = a23 / a12;
    a25 = a24 ^ a13;
    a26 = a25 | a14;
    a27 = a26 & a15;
    a28 = a27 << 1;
    a29 = a28 >> 1;
    a30 = a29 + a16;
    
    a31 = a30 - a17;
    a32 = a31 * a18;
    a33 = a32 / a19;
    a34 = a33 ^ a20;
    a35 = a34 | a21;
    a36 = a35 & a22;
    a37 = a36 << 2;
    a38 = a37 >> 1;
    a39 = a38 + a23;
    a40 = a39 - a24;
    
    /* Force all variables to be used in loop with control flow */
    for (int i = 0; i < 100; i++) {
        if (i & 1) {
            a1 += a2;
            a3 -= a4;
            a5 *= a6;
        } else {
            a7 ^= a8;
            a9 |= a10;
            a11 &= a12;
        }
        
        /* Complex condition to create CFG edges */
        switch (i % 5) {
            case 0: a13 += a14; break;
            case 1: a15 -= a16; break;
            case 2: a17 *= a18; break;
            case 3: a19 ^= a20; break;
            case 4: a21 |= a22; break;
        }
        
        /* Prevent dead code elimination */
        asm volatile("" : "+r"(a1), "+r"(a2), "+r"(a3), "+r"(a4), "+r"(a5));
        asm volatile("" : "+r"(a6), "+r"(a7), "+r"(a8), "+r"(a9), "+r"(a10));
    }
    
    /* Combine all results */
    result = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
             a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19 + a20 +
             a21 + a22 + a23 + a24 + a25 + a26 + a27 + a28 + a29 + a30 +
             a31 + a32 + a33 + a34 + a35 + a36 + a37 + a38 + a39 + a40;
    
    return result;
}

/* ============================================
 * PATTERN B: Floating-point intensive computation
 * Pressures FP/SIMD registers with double variables
 * ============================================ */
NOINLINE double pattern_b_fp_pressure(double input) {
    /* 20+ double variables */
    double b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    double b11, b12, b13, b14, b15, b16, b17, b18, b19, b20;
    double b21, b22, b23, b24, b25;
    
    b1 = input + 1.0;
    b2 = b1 * 2.0;
    b3 = b2 - 1.5;
    b4 = b3 / 2.0;
    b5 = b4 * b1;
    b6 = b5 + b2;
    b7 = b6 - b3;
    b8 = b7 * b4;
    b9 = b8 / b5;
    b10 = b9 + b6;
    
    b11 = b10 - b7;
    b12 = b11 * b8;
    b13 = b12 / b9;
    b14 = b13 + b10;
    b15 = b14 - b11;
    b16 = b15 * b12;
    b17 = b16 / b13;
    b18 = b17 + b14;
    b19 = b18 - b15;
    b20 = b19 * b16;
    
    b21 = b20 / b17;
    b22 = b21 + b18;
    b23 = b22 - b19;
    b24 = b23 * b20;
    b25 = b24 / b21;
    
    /* Nested loops with FP operations */
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 10; j++) {
            if ((i + j) & 1) {
                b1 = b2 * b3;
                b4 = b5 + b6;
                b7 = b8 - b9;
            } else {
                b10 = b11 / b12;
                b13 = b14 * b15;
                b16 = b17 - b18;
            }
            
            /* Mix integer index to create register class conflicts */
            int idx = i * j;
            b19 += idx * 0.01;
            b20 -= idx * 0.02;
            
            /* Prevent optimization */
            asm volatile("" : "+f"(b1), "+f"(b2), "+f"(b3));
        }
        
        /* Break/continue to create CFG complexity */
        if (i == 25) continue;
        if (i == 45) break;
    }
    
    return b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9 + b10 +
           b11 + b12 + b13 + b14 + b15 + b16 + b17 + b18 + b19 + b20 +
           b21 + b22 + b23 + b24 + b25;
}

/* ============================================
 * PATTERN C: Complex control flow with switch
 * Creates many basic blocks for CFG complexity
 * ============================================ */
NOINLINE int pattern_c_cfg_complexity(int input) {
    int result = input;
    
    /* Large switch with 20+ cases */
    for (int i = 0; i < 100; i++) {
        switch (i % 23) {  /* Prime number to avoid pattern */
            case 0:  result += i * 2;    break;
            case 1:  result -= i / 2;    break;
            case 2:  result ^= i;        break;
            case 3:  result |= i << 1;   break;
            case 4:  result &= ~i;       break;
            case 5:  result = result * 3 - i; break;
            case 6:  result = result / (i % 7 + 1); break;
            case 7:  result = ~result;   break;
            case 8:  result = result << (i % 4); break;
            case 9:  result = result >> (i % 4); break;
            case 10: result = result + (i * i); break;
            case 11: result = result - (i | 0xF); break;
            case 12: result = result ^ (i & 0xFF); break;
            case 13: result = result | (i << 8); break;
            case 14: result = result & (i | 0xAA); break;
            case 15: result = result * (i % 5 + 1); break;
            case 16: result = result / (i % 3 + 1); break;
            case 17: result = result % (i % 10 + 1); break;
            case 18: result = -result;   break;
            case 19: result = result + (result >> 1); break;
            case 20: result = result - (result << 1); break;
            case 21: result = result ^ 0xDEADBEEF; break;
            case 22: result = result | 0xCAFEBABE; break;
        }
        
        /* Nested if-else chain */
        if (i < 10) {
            result += 1;
        } else if (i < 20) {
            result -= 2;
        } else if (i < 30) {
            result *= 3;
        } else if (i < 40) {
            result /= 4;
        } else if (i < 50) {
            result ^= 5;
        } else if (i < 60) {
            result |= 6;
        } else if (i < 70) {
            result &= 7;
        } else if (i < 80) {
            result <<= 1;
        } else if (i < 90) {
            result >>= 1;
        } else {
            result = ~result;
        }
        
        /* Loop with break/continue */
        for (int j = 0; j < 5; j++) {
            if (j == 2 && (i % 7 == 0)) break;
            if (j == 3 && (i % 11 == 0)) continue;
            result += j;
        }
    }
    
    return result;
}

/* ============================================
 * PATTERN D: Vector/SIMD operations
 * Pressures vector registers
 * ============================================ */
NOINLINE v4si pattern_d_vector_pressure(v4si input) {
    v4si v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    v4si v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    
    /* Initialize vectors */
    v1 = input + (v4si){1, 2, 3, 4};
    v2 = v1 * (v4si){2, 3, 4, 5};
    v3 = v2 - v1;
    v4 = v3 + input;
    v5 = v4 * v2;
    v6 = v5 + v1;
    v7 = v6 - v2;
    v8 = v7 * v3;
    v9 = v8 + v4;
    v10 = v9 - v5;
    
    v11 = v10 * v6;
    v12 = v11 + v7;
    v13 = v12 - v8;
    v14 = v13 * v9;
    v15 = v14 + v10;
    v16 = v15 - v11;
    v17 = v16 * v12;
    v18 = v17 + v13;
    v19 = v18 - v14;
    v20 = v19 * v15;
    
    /* Vector loop with conditionals */
    for (int i = 0; i < 50; i++) {
        if (i & 1) {
            v1 = v1 + v2;
            v3 = v3 - v4;
            v5 = v5 * v6;
        } else {
            v7 = v7 + v8;
            v9 = v9 - v10;
            v11 = v11 * v12;
        }
        
        /* Mix with scalar to create register pressure */
        int scalar = i * 7;
        v13 = v13 + (v4si){scalar, scalar + 1, scalar + 2, scalar + 3};
        v14 = v14 - (v4si){scalar, scalar - 1, scalar - 2, scalar - 3};
        
        /* Prevent dead code elimination */
        asm volatile("" : "+x"(v1), "+x"(v2), "+x"(v3));
    }
    
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
}

/* ============================================
 * PATTERN E: Explicit register variables
 * Conflicts with allocator's choices
 * ============================================ */
NOINLINE int pattern_e_explicit_registers(int input) {
    /* Explicit register variables that may conflict */
    register int r1 asm ("r12") = input + 1;
    register int r2 asm ("r13") = r1 * 2;
    register int r3 asm ("r14") = r2 - 3;
    register int r4 asm ("r15") = r3 + 4;
    
    int local1, local2, local3, local4, local5;
    int local6, local7, local8, local9, local10;
    
    /* Mix explicit and implicit registers */
    local1 = r1 + r2;
    local2 = r3 - r4;
    local3 = local1 * local2;
    local4 = r1 | r3;
    local5 = r2 & r4;
    
    /* Complex loop with register pressure */
    for (int i = 0; i < 100; i++) {
        /* Force spills by using many variables */
        local6 = local1 + i;
        local7 = local2 - i;
        local8 = local3 * (i + 1);
        local9 = local4 ^ i;
        local10 = local5 | i;
        
        /* Update register variables */
        r1 += local6;
        r2 -= local7;
        r3 *= (local8 % 100) + 1;
        r4 ^= local9;
        
        /* Use computed goto (GCC extension) for CFG complexity */
        void* labels[] = {&&L0, &&L1, &&L2, &&L3, &&L4};
        goto *labels[i % 5];
        
        L0: local1++; continue;
        L1: local2--; continue;
        L2: local3 *= 2; continue;
        L3: local4 ^= 0xFF; continue;
        L4: local5 |= 0xAA; continue;
    }
    
    return r1 + r2 + r3 + r4 + local1 + local2 + local3 + local4 + local5 +
           local6 + local7 + local8 + local9 + local10;
}

/* ============================================
 * Main function - cold attribute may affect block ordering
 * ============================================ */
COLD int main(int argc, char** argv) {
    int result = 0;
    double fp_result = 0.0;
    v4si vec_result = {0};
    
    /* Array of inputs to prevent constant folding */
    int inputs[] = {1, 2, 3, 5, 7, 11, 13, 17, 19, 23};
    double fp_inputs[] = {1.1, 2.2, 3.3, 4.4, 5.5};
    v4si vec_inputs[] = {
        {1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}
    };
    
    /* Call all patterns with different inputs */
    for (int i = 0; i < 10; i++) {
        result ^= pattern_a_int_pressure(inputs[i % 10]);
        
        if (i < 5) {
            fp_result += pattern_b_fp_pressure(fp_inputs[i]);
        }
        
        result += pattern_c_cfg_complexity(inputs[i % 10]);
        
        if (i < 3) {
            vec_result = vec_result + pattern_d_vector_pressure(vec_inputs[i]);
        }
        
        result += pattern_e_explicit_registers(inputs[i % 10]);
    }
    
    /* Use CPU feature check to engage target-specific optimizations */
#ifdef __GNUC__
    if (__builtin_cpu_supports("avx2")) {
        result += 1;
    }
    if (__builtin_cpu_supports("sse4.2")) {
        result += 2;
    }
#endif
    
    /* Combine all results to prevent dead code elimination */
    int final_result = result + (int)fp_result;
    for (int i = 0; i < 4; i++) {
        final_result += vec_result[i];
    }
    
    /* Optional debug output */
    if (argc > 1) {
        printf("Final result: %d\n", final_result);
    }
    
    return final_result % 256;
}
