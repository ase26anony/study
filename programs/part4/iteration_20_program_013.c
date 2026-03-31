/* test_mcf_coverage.c
 * 
 * This test program is designed to stress GCC's Min-Cost Flow register allocator
 * to trigger coverage of special node indices in the print_node function.
 * 
 * Compile with: gcc -O3 -fprofile-arcs -ftest-coverage -fdump-rtl-bbro test_mcf_coverage.c -o test_mcf
 * Run with: ./test_mcf
 * Generate coverage with: gcov test_mcf_coverage.c
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

/* ============================================
 * PATTERN A: Integer arithmetic chain with 30+ variables
 * Creates high register pressure in a tight loop
 * ============================================ */
NOINLINE static int pattern_a_int_arithmetic(int input) {
    /* 30+ integer variables to create register pressure */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int a11, a12, a13, a14, a15, a16, a17, a18, a19, a20;
    int a21, a22, a23, a24, a25, a26, a27, a28, a29, a30;
    int a31, a32, a33, a34, a35, a36, a37, a38, a39, a40;
    
    /* Initialize with input to prevent constant folding */
    a1 = input + 1;
    a2 = a1 * 2 + input;
    a3 = a2 - a1 + input;
    a4 = a3 * a2 + input;
    a5 = a4 / (a1 + 1) + input;
    a6 = a5 ^ a4 + input;
    a7 = a6 | a5 + input;
    a8 = a7 & a6 + input;
    a9 = a8 << 2 + input;
    a10 = a9 >> 1 + input;
    
    a11 = a10 + a9 + input;
    a12 = a11 * a10 + input;
    a13 = a12 - a11 + input;
    a14 = a13 * a12 + input;
    a15 = a14 / (a11 + 1) + input;
    a16 = a15 ^ a14 + input;
    a17 = a16 | a15 + input;
    a18 = a17 & a16 + input;
    a19 = a18 << 3 + input;
    a20 = a19 >> 2 + input;
    
    a21 = a20 + a19 + input;
    a22 = a21 * a20 + input;
    a23 = a22 - a21 + input;
    a24 = a23 * a22 + input;
    a25 = a24 / (a21 + 1) + input;
    a26 = a25 ^ a24 + input;
    a27 = a26 | a25 + input;
    a28 = a27 & a26 + input;
    a29 = a28 << 1 + input;
    a30 = a29 >> 1 + input;
    
    a31 = a30 + a29 + input;
    a32 = a31 * a30 + input;
    a33 = a32 - a31 + input;
    a34 = a33 * a32 + input;
    a35 = a34 / (a31 + 1) + input;
    a36 = a35 ^ a34 + input;
    a37 = a36 | a35 + input;
    a38 = a37 & a36 + input;
    a39 = a38 << 4 + input;
    a40 = a39 >> 2 + input;
    
    /* Complex loop with interdependent operations */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        /* Force all variables to be live across iterations */
        a1 = a40 + i;
        a2 = a1 + a39;
        a3 = a2 + a38;
        a4 = a3 + a37;
        a5 = a4 + a36;
        a6 = a5 + a35;
        a7 = a6 + a34;
        a8 = a7 + a33;
        a9 = a8 + a32;
        a10 = a9 + a31;
        
        a11 = a10 + a30;
        a12 = a11 + a29;
        a13 = a12 + a28;
        a14 = a13 + a27;
        a15 = a14 + a26;
        a16 = a15 + a25;
        a17 = a16 + a24;
        a18 = a17 + a23;
        a19 = a18 + a22;
        a20 = a19 + a21;
        
        /* Cross-dependencies */
        a21 = a20 + a1;
        a22 = a21 + a2;
        a23 = a22 + a3;
        a24 = a23 + a4;
        a25 = a24 + a5;
        a26 = a25 + a6;
        a27 = a26 + a7;
        a28 = a27 + a8;
        a29 = a28 + a9;
        a30 = a29 + a10;
        
        a31 = a30 + a11;
        a32 = a31 + a12;
        a33 = a32 + a13;
        a34 = a33 + a14;
        a35 = a34 + a15;
        a36 = a35 + a16;
        a37 = a36 + a17;
        a38 = a37 + a18;
        a39 = a38 + a19;
        a40 = a39 + a20;
        
        sum += a40;
        
        /* Control flow variation */
        if (i % 7 == 0) {
            a1 += input;
            continue;
        }
        if (i % 13 == 0) {
            a2 *= 2;
            break;
        }
        if (i % 17 == 0) {
            a3 -= input;
            continue;
        }
    }
    
    /* Final computation using all variables */
    return a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
           a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19 + a20 +
           a21 + a22 + a23 + a24 + a25 + a26 + a27 + a28 + a29 + a30 +
           a31 + a32 + a33 + a34 + a35 + a36 + a37 + a38 + a39 + a40 +
           sum + input;
}

/* ============================================
 * PATTERN B: Floating-point intensive computation
 * Stresses floating-point register allocation
 * ============================================ */
NOINLINE static double pattern_b_float_pressure(double input) {
    /* 20+ double variables */
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    double d11, d12, d13, d14, d15, d16, d17, d18, d19, d20;
    double d21, d22, d23, d24, d25;
    
    d1 = input + 1.0;
    d2 = d1 * 2.0 + input;
    d3 = d2 - d1 + input;
    d4 = d3 * d2 + input;
    d5 = d4 / (d1 + 1.0) + input;
    d6 = d5 * 0.5 + input;
    d7 = d6 + d5 + input;
    d8 = d7 * d6 + input;
    d9 = d8 - d7 + input;
    d10 = d9 / (d8 + 1.0) + input;
    
    d11 = d10 * 3.14159 + input;
    d12 = d11 + d10 + input;
    d13 = d12 * d11 + input;
    d14 = d13 - d12 + input;
    d15 = d14 / (d13 + 1.0) + input;
    d16 = d15 * 2.71828 + input;
    d17 = d16 + d15 + input;
    d18 = d17 * d16 + input;
    d19 = d18 - d17 + input;
    d20 = d19 / (d18 + 1.0) + input;
    
    d21 = d20 * 1.414 + input;
    d22 = d21 + d20 + input;
    d23 = d22 * d21 + input;
    d24 = d23 - d22 + input;
    d25 = d24 / (d23 + 1.0) + input;
    
    /* Nested loops with floating operations */
    double result = 0.0;
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 10; j++) {
            /* Complex floating-point operations */
            d1 = d25 * j + i;
            d2 = d1 + d24;
            d3 = d2 * d23;
            d4 = d3 - d22;
            d5 = d4 / (d21 + 1.0);
            d6 = d5 * d20;
            d7 = d6 + d19;
            d8 = d7 * d18;
            d9 = d8 - d17;
            d10 = d9 / (d16 + 1.0);
            
            d11 = d10 * d15;
            d12 = d11 + d14;
            d13 = d12 * d13;  /* Self-modification */
            d14 = d13 - d12;
            d15 = d14 / (d11 + 1.0);
            d16 = d15 * d10;
            d17 = d16 + d9;
            d18 = d17 * d8;
            d19 = d18 - d7;
            d20 = d19 / (d6 + 1.0);
            
            d21 = d20 * d5;
            d22 = d21 + d4;
            d23 = d22 * d3;
            d24 = d23 - d2;
            d25 = d24 / (d1 + 1.0);
            
            result += d25;
            
            /* Control flow with floating comparisons */
            if (d25 > 1000.0) {
                d1 *= 0.5;
                continue;
            }
            if (d25 < -1000.0) {
                d2 /= 2.0;
                break;
            }
        }
        
        /* Loop-carried dependency */
        input += 0.1;
    }
    
    return d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10 +
           d11 + d12 + d13 + d14 + d15 + d16 + d17 + d18 + d19 + d20 +
           d21 + d22 + d23 + d24 + d25 + result + input;
}

/* ============================================
 * PATTERN C: Complex control flow with switch statement
 * Creates many basic blocks for CFG complexity
 * ============================================ */
NOINLINE static int pattern_c_complex_cfg(int input) {
    int result = input;
    
    /* Outer loop with switch inside */
    for (int i = 0; i < 100; i++) {
        switch (i % 23) {  /* 23 cases for many basic blocks */
            case 0: {
                int t1 = result * 2;
                int t2 = t1 + 1;
                int t3 = t2 ^ t1;
                int t4 = t3 | t2;
                result = t4 & 0xFF;
                break;
            }
            case 1: {
                int t1 = result + 7;
                int t2 = t1 * 3;
                int t3 = t2 - t1;
                int t4 = t3 << 2;
                result = t4 >> 1;
                break;
            }
            case 2: {
                int t1 = result ^ 0x55;
                int t2 = t1 & 0xAA;
                int t3 = t2 | 0x33;
                int t4 = t3 * result;
                result = t4 % 256;
                break;
            }
            case 3: {
                int t1 = result + 11;
                int t2 = t1 * 5;
                int t3 = t2 / (result + 1);
                int t4 = t3 << 3;
                result = t4 | 0xF;
                break;
            }
            case 4: {
                int t1 = result - 3;
                int t2 = t1 * 7;
                int t3 = t2 ^ result;
                int t4 = t3 & 0x7F;
                result = t4 + 128;
                break;
            }
            case 5: {
                int t1 = result * 9;
                int t2 = t1 + 13;
                int t3 = t2 % 17;
                int t4 = t3 << 4;
                result = t4 >> 2;
                break;
            }
            case 6: {
                int t1 = result | 0xCC;
                int t2 = t1 & 0x33;
                int t3 = t2 ^ 0xAA;
                int t4 = t3 + result;
                result = t4 * 2;
                break;
            }
            case 7: {
                int t1 = result + 17;
                int t2 = t1 * 11;
                int t3 = t2 - 19;
                int t4 = t3 / (result + 2);
                result = t4 ^ 0xFF;
                break;
            }
            case 8: {
                int t1 = result << 1;
                int t2 = t1 >> 1;
                int t3 = t2 + 23;
                int t4 = t3 * 3;
                result = t4 % 100;
                break;
            }
            case 9: {
                int t1 = result & 0xF0;
                int t2 = t1 | 0x0F;
                int t3 = t2 ^ result;
                int t4 = t3 + 29;
                result = t4 * 5;
                break;
            }
            case 10: {
                int t1 = result + 31;
                int t2 = t1 * 7;
                int t3 = t2 / 3;
                int t4 = t3 << 5;
                result = t4 >> 3;
                break;
            }
            case 11: {
                int t1 = result ^ 0x77;
                int t2 = t1 & 0x88;
                int t3 = t2 | 0x44;
                int t4 = t3 + 37;
                result = t4 % 150;
                break;
            }
            case 12: {
                int t1 = result * 13;
                int t2 = t1 + 41;
                int t3 = t2 - 43;
                int t4 = t3 ^ result;
                result = t4 & 0x3F;
                break;
            }
            case 13: {
                int t1 = result | 0x22;
                int t2 = t1 & 0xDD;
                int t3 = t2 + 47;
                int t4 = t3 * 11;
                result = t4 / 5;
                break;
            }
            case 14: {
                int t1 = result + 53;
                int t2 = t1 << 2;
                int t3 = t2 >> 1;
                int t4 = t3 ^ 0x99;
                result = t4 % 200;
                break;
            }
            case 15: {
                int t1 = result * 17;
                int t2 = t1 & 0xBB;
                int t3 = t2 | 0x66;
                int t4 = t3 + 59;
                result = t4 - 61;
                break;
            }
            case 16: {
                int t1 = result ^ 0x11;
                int t2 = t1 + 67;
                int t3 = t2 * 19;
                int t4 = t3 / 7;
                result = t4 << 1;
                break;
            }
            case 17: {
                int t1 = result & 0xFE;
                int t2 = t1 | 0x01;
                int t3 = t2 + 71;
                int t4 = t3 ^ result;
                result = t4 % 180;
                break;
            }
            case 18: {
                int t1 = result + 73;
                int t2 = t1 * 23;
                int t3 = t2 - 79;
                int t4 = t3 & 0x7F;
                result = t4 | 0x80;
                break;
            }
            case 19: {
                int t1 = result << 3;
                int t2 = t1 >> 2;
                int t3 = t2 ^ 0xEE;
                int t4 = t3 + 83;
                result = t4 * 3;
                break;
            }
            case 20: {
                int t1 = result | 0x18;
                int t2 = t1 & 0xE7;
                int t3 = t2 + 89;
                int t4 = t3 / 13;
                result = t4 % 220;
                break;
            }
            case 21: {
                int t1 = result ^ 0x44;
                int t2 = t1 * 29;
                int t3 = t2 + 97;
                int t4 = t3 - 101;
                result = t4 & 0xFF;
                break;
            }
            case 22: {
                int t1 = result + 103;
                int t2 = t1 & 0x3C;
                int t3 = t2 | 0xC3;
                int t4 = t3 << 4;
                result = t4 >> 2;
                break;
            }
        }
        
        /* Additional control flow with breaks and continues */
        if (i % 7 == 0) {
            result += 107;
            continue;
        }
        if (i % 11 == 0) {
            result *= 2;
            if (result > 1000) break;
        }
        if (i % 13 == 0) {
            result ^= 0xAA;
            continue;
        }
    }
    
    return result;
}

/* ============================================
 * PATTERN D: Vector/SIMD operations
 * Stresses vector register allocation
 * ============================================ */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

NOINLINE static int pattern_d_vector_ops(int input) {
    v4si vec1 = {input, input + 1, input + 2, input + 3};
    v4si vec2 = {input + 4, input + 5, input + 6, input + 7};
    v4si vec3 = {input + 8, input + 9, input + 10, input + 11};
    v4si vec4 = {input + 12, input + 13, input + 14, input + 15};
    v4si vec5 = {input + 16, input + 17, input + 18, input + 19};
    v4si vec6 = {input + 20, input + 21, input + 22, input + 23};
    v4si vec7 = {input + 24, input + 25, input + 26, input + 27};
    v4si vec8 = {input + 28, input + 29, input + 30, input + 31};
    
    v4sf fvec1 = {input * 0.1f, input * 0.2f, input * 0.3f, input * 0.4f};
    v4sf fvec2 = {input * 0.5f, input * 0.6f, input * 0.7f, input * 0.8f};
    v4sf fvec3 = {input * 0.9f, input * 1.0f, input * 1.1f, input * 1.2f};
    v4sf fvec4 = {input * 1.3f, input * 1.4f, input * 1.5f, input * 1.6f};
    
    int sum = 0;
    for (int i = 0; i < 50; i++) {
        /* Integer vector operations */
        vec1 = vec1 + vec2;
        vec2 = vec2 * vec3;
        vec3 = vec3 - vec4;
        vec4 = vec4 & vec5;
        vec5 = vec5 | vec6;
        vec6 = vec6 ^ vec7;
        vec7 = vec7 << 1;
        vec8 = vec8 >> 1;
        
        /* Floating vector operations */
        fvec1 = fvec1 + fvec2;
        fvec2 = fvec2 * fvec3;
        fvec3 = fvec3 - fvec4;
        fvec4 = fvec4 * 2.0f;
        
        /* Cross-type operations (extract and use) */
        int el1 = vec1[0] + vec1[1] + vec1[2] + vec1[3];
        int el2 = vec2[0] + vec2[1] + vec2[2] + vec2[3];
        int el3 = vec3[0] + vec3[1] + vec3[2] + vec3[3];
        int el4 = vec4[0] + vec4[1] + vec4[2] + vec4[3];
        
        float fel1 = fvec1[0] + fvec1[1] + fvec1[2] + fvec1[3];
        float fel2 = fvec2[0] + fvec2[1] + fvec2[2] + fvec2[3];
        
        sum += el1 + el2 + el3 + el4 + (int)fel1 + (int)fel2;
        
        /* Control flow affecting vector operations */
        if (i % 7 == 0) {
            vec1 = vec8 + vec1;
            continue;
        }
        if (i % 13 == 0) {
            vec2 = vec2 * 2;
            break;
        }
    }
    
    /* Extract final results */
    int result = vec1[0] + vec1[1] + vec1[2] + vec1[3] +
                 vec2[0] + vec2[1] + vec2[2] + vec2[3] +
                 vec3[0] + vec3[1] + vec3[2] + vec3[3] +
                 vec4[0] + vec4[1] + vec4[2] + vec4[3] +
                 sum + input;
    
    return result;
}

/* ============================================
 * PATTERN E: Explicit register variables
 * Conflicts with allocator's choices
 * ============================================ */
NOINLINE static int pattern_e_explicit_registers(int input) {
    /* Try to use specific registers that might conflict */
    register int r1 asm ("r12") = input + 1;
    register int r2 asm ("r13") = input + 2;
    register int r3 asm ("r14") = input + 3;
    register int r4 asm ("r15") = input + 4;
    
    /* Additional non-register variables for pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    
    v1 = r1 * 2;
    v2 = r2 + v1;
    v3 = r3 ^ v2;
    v4 = r4 & v3;
    v5 = v4 | r1;
    v6 = v5 << r2;
    v7 = v6 >> r3;
    v8 = v7 + r4;
    v9 = v8 * v1;
    v10 = v9 / (v2 + 1);
    
    v11 = v10 ^ v3;
    v12 = v11 & v4;
    v13 = v12 | v5;
    v14 = v13 << 1;
    v15 = v14 >> 2;
    v16 = v15 + v6;
    v17 = v16 * v7;
    v18 = v17 / (v8 + 1);
    v19 = v18 ^ v9;
    v20 = v19 & v10;
    
    /* Complex loop with register variable usage */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        /* Force register variables to be live and modified */
        asm volatile ("" : "+r" (r1), "+r" (r2), "+r" (r3), "+r" (r4));
        
        r1 = v20 + i;
        r2 = r1 + v19;
        r3 = r2 + v18;
        r4 = r3 + v17;
        
        v1 = r4 + v16;
        v2 = v1 + v15;
        v3 = v2 + v14;
        v4 = v3 + v13;
        v5 = v4 + v12;
        v6 = v5 + v11;
        v7 = v6 + v10;
        v8 = v7 + v9;
        v9 = v8 + v20;
        v10 = v9 + v19;
        
        v11 = v10 + v18;
        v12 = v11 + v17;
        v13 = v12 + v16;
        v14 = v13 + v15;
        v15 = v14 + v1;
        v16 = v15 + v2;
        v17 = v16 + v3;
        v18 = v17 + v4;
        v19 = v18 + v5;
        v20 = v19 + v6;
        
        sum += r1 + r2 + r3 + r4 + v20;
        
        /* Computed goto for additional CFG complexity */
        void* labels[] = {&&L0, &&L1, &&L2, &&L3, &&L4, &&L5};
        goto *labels[i % 6];
        
        L0:
            r1 += 1;
            continue;
        L1:
            r2 *= 2;
            continue;
        L2:
            r3 ^= 0xFF;
            continue;
        L3:
            r4 -= 1;
            continue;
        L4:
            v1 = v20;
            continue;
        L5:
            v2 = v19;
            if (i > 50) break;
            continue;
    }
    
    return r1 + r2 + r3 + r4 + v1 + v2 + v3 + v4 + v5 + v6 +
           v7 + v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15 +
           v16 + v17 + v18 + v19 + v20 + sum + input;
}

/* ============================================
 * Main function - calls all patterns
 * ============================================ */
COLD int main(int argc, char** argv) {
    int result = 0;
    
    /* Use CPU feature check to engage target-specific optimizations */
    if (__builtin_cpu_supports("avx2") || 
        __builtin_cpu_supports("sse4.2") ||
        __builtin_cpu_supports("popcnt")) {
        /* This ensures the compilation pipeline includes target-specific
         * optimizations that interact with register allocation */
    }
    
    /* Call each pattern multiple times with different inputs */
    int inputs[] = {1, 2, 3, 5, 7, 11, 13, 17, 19, 23};
    
    for (int i = 0; i < 10; i++) {
        result += pattern_a_int_arithmetic(inputs[i]);
        result += (int)pattern_b_float_pressure((double)inputs[i]);
        result += pattern_c_complex_cfg(inputs[i]);
        result += pattern_d_vector_ops(inputs[i]);
        result += pattern_e_explicit_registers(inputs[i]);
        
        /* Prevent compiler from optimizing everything away */
        asm volatile ("" : "+r" (result));
    }
    
    /* Optional debug output */
#ifdef DEBUG_OUTPUT
    printf("Final result: %d\n", result);
#endif
    
    return result % 256;  /* Return small value to avoid overflow issues */
}
