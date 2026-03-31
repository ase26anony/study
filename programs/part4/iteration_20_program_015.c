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

/* Prevent inlining to ensure each function is compiled independently */
#define NOINLINE __attribute__((noinline))
#define COLD __attribute__((cold))

/* Stub for non-GCC compilers */
#ifndef __GNUC__
#define __builtin_cpu_supports(x) 0
#endif

/* ============================================================
 * PATTERN A: Integer arithmetic chain with 30+ variables
 * Creates massive register pressure in a tight loop
 * ============================================================ */
NOINLINE static int pattern_a_intensive(int input) {
    /* 30+ integer variables to pressure general purpose registers */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int a11, a12, a13, a14, a15, a16, a17, a18, a19, a20;
    int a21, a22, a23, a24, a25, a26, a27, a28, a29, a30;
    int a31, a32, a33, a34, a35, a36, a37, a38, a39, a40;
    
    /* Initialize with input to prevent constant folding */
    a1 = input;
    
    /* Long dependency chain to force sequential evaluation */
    for (int i = 0; i < 100; i++) {
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
        a13 = a12 / (a11 + 1);
        a14 = a13 % (a12 + 1);
        a15 = a14 ^ a13;
        a16 = a15 | a14;
        a17 = a16 & a15;
        a18 = a17 << 3;
        a19 = a18 >> 2;
        a20 = a19 + a18;
        a21 = a20 - a19;
        a22 = a21 * a20;
        a23 = a22 / (a21 + 1);
        a24 = a23 % (a22 + 1);
        a25 = a24 ^ a23;
        a26 = a25 | a24;
        a27 = a26 & a25;
        a28 = a27 << 1;
        a29 = a28 >> 1;
        a30 = a29 + a28;
        a31 = a30 - a29;
        a32 = a31 * a30;
        a33 = a32 / (a31 + 1);
        a34 = a33 % (a32 + 1);
        a35 = a34 ^ a33;
        a36 = a35 | a34;
        a37 = a36 & a35;
        a38 = a37 << 2;
        a39 = a38 >> 1;
        a40 = a39 + a38;
        
        /* Feed back to create loop-carried dependency */
        a1 = a40 % 997; /* Prime to avoid simple patterns */
    }
    
    /* Complex return to use all variables */
    return a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
           a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19 + a20 +
           a21 + a22 + a23 + a24 + a25 + a26 + a27 + a28 + a29 + a30 +
           a31 + a32 + a33 + a34 + a35 + a36 + a37 + a38 + a39 + a40;
}

/* ============================================================
 * PATTERN B: Floating-point intensive computation
 * Pressures floating-point and MMX/SSE registers
 * ============================================================ */
NOINLINE static double pattern_b_fp_intensive(double input) {
    /* 20+ double variables */
    double b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    double b11, b12, b13, b14, b15, b16, b17, b18, b19, b20;
    double b21, b22, b23, b24, b25;
    
    b1 = input;
    
    /* Floating-point operations with different precision demands */
    for (int i = 0; i < 50; i++) {
        b2 = b1 * 1.1;
        b3 = b2 / 1.3;
        b4 = b3 + b2;
        b5 = b4 - b3;
        b6 = b5 * 0.9;
        b7 = b6 / 1.7;
        b8 = b7 + b6;
        b9 = b8 - b7;
        b10 = b9 * 1.2;
        b11 = b10 / 1.5;
        b12 = b11 + b10;
        b13 = b12 - b11;
        b14 = b13 * 0.8;
        b15 = b14 / 1.9;
        b16 = b15 + b14;
        b17 = b16 - b15;
        b18 = b17 * 1.3;
        b19 = b18 / 1.1;
        b20 = b19 + b18;
        b21 = b20 - b19;
        b22 = b21 * 0.7;
        b23 = b22 / 2.1;
        b24 = b23 + b22;
        b25 = b24 - b23;
        
        b1 = b25 * 0.99;
    }
    
    return b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9 + b10 +
           b11 + b12 + b13 + b14 + b15 + b16 + b17 + b18 + b19 + b20 +
           b21 + b22 + b23 + b24 + b25;
}

/* ============================================================
 * PATTERN C: Complex control flow with switch statement
 * Creates many basic blocks for CFG complexity
 * ============================================================ */
NOINLINE static int pattern_c_complex_cfg(int input) {
    int result = input;
    
    /* Nested loops with switch inside */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            /* Switch with 20+ cases - each creates a basic block */
            switch ((i * j + input) % 23) {
                case 0:  result += i; break;
                case 1:  result -= j; break;
                case 2:  result *= 2; break;
                case 3:  result /= 3; break;
                case 4:  result ^= i; break;
                case 5:  result |= j; break;
                case 6:  result &= 0xFF; break;
                case 7:  result <<= 1; break;
                case 8:  result >>= 2; break;
                case 9:  result = ~result; break;
                case 10: result += i * j; break;
                case 11: result -= i + j; break;
                case 12: result *= i - j; break;
                case 13: result = result % 17; break;
                case 14: result = result ^ j; break;
                case 15: result = result | i; break;
                case 16: result = result & 0x7F; break;
                case 17: result = result << 3; break;
                case 18: result = result >> 1; break;
                case 19: result = -result; break;
                case 20: result = result + 100; break;
                case 21: result = result - 50; break;
                case 22: result = result * 3; break;
                default: result += 1; break;
            }
            
            /* Conditional break/continue to add more CFG edges */
            if (result > 1000) {
                result /= 2;
                continue;
            }
            if (result < 0) {
                result = -result;
                break;
            }
        }
    }
    
    return result;
}

/* ============================================================
 * PATTERN D: Vector/SIMD operations
 * Pressures vector registers
 * ============================================================ */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

NOINLINE static int pattern_d_vector_ops(int input) {
    v4si vec1 = {input, input + 1, input + 2, input + 3};
    v4si vec2 = {4, 5, 6, 7};
    v4si vec3 = {8, 9, 10, 11};
    v4si vec4 = {12, 13, 14, 15};
    v4si vec5 = {16, 17, 18, 19};
    v4si vec6 = {20, 21, 22, 23};
    v4si vec7 = {24, 25, 26, 27};
    v4si vec8 = {28, 29, 30, 31};
    
    v4sf fvec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf fvec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf fvec4 = {13.0f, 14.0f, 15.0f, 16.0f};
    
    /* Vector operations in loop */
    for (int i = 0; i < 20; i++) {
        vec1 = vec1 + vec2;
        vec2 = vec2 * vec3;
        vec3 = vec3 - vec4;
        vec4 = vec4 & vec5;
        vec5 = vec5 | vec6;
        vec6 = vec6 ^ vec7;
        vec7 = vec7 << 1;
        vec8 = vec8 >> 2;
        
        fvec1 = fvec1 * fvec2;
        fvec2 = fvec2 + fvec3;
        fvec3 = fvec3 - fvec4;
        fvec4 = fvec4 * 1.1f;
        
        /* Mix vector types */
        vec1 = vec1 + (v4si){i, i, i, i};
    }
    
    /* Extract results to prevent elimination */
    int sum = vec1[0] + vec1[1] + vec1[2] + vec1[3] +
              vec2[0] + vec2[1] + vec2[2] + vec2[3] +
              (int)fvec1[0] + (int)fvec1[1];
    
    return sum;
}

/* ============================================================
 * PATTERN E: Explicit register variables
 * Conflicts with allocator's choices
 * ============================================================ */
NOINLINE static int pattern_e_explicit_registers(int input) {
    /* Try to bind specific registers */
    register int r1 asm ("r12") = input;
    register int r2 asm ("r13") = input + 1;
    register int r3 asm ("r14") = input + 2;
    register int r4 asm ("r15") = input + 3;
    
    int local1, local2, local3, local4, local5, local6;
    
    /* Force spills by using many variables */
    for (int i = 0; i < 100; i++) {
        local1 = r1 * i;
        local2 = r2 + local1;
        local3 = r3 - local2;
        local4 = r4 ^ local3;
        local5 = local1 | local4;
        local6 = local2 & local5;
        
        /* Update register variables */
        asm volatile ("" : "+r" (r1), "+r" (r2), "+r" (r3), "+r" (r4));
        
        r1 = local6 + 1;
        r2 = local5 - 1;
        r3 = local4 * 2;
        r4 = local3 / 2;
    }
    
    return r1 + r2 + r3 + r4 + local1 + local2 + local3 + local4 + local5 + local6;
}

/* ============================================================
 * Main function - calls all patterns
 * ============================================================ */
COLD int main(int argc, char **argv) {
    int result = 0;
    
    /* Use CPU feature check to engage target-specific optimizations */
    if (__builtin_cpu_supports("sse2") ||
        __builtin_cpu_supports("avx") ||
        __builtin_cpu_supports("avx2")) {
        /* This encourages GCC to use architecture-specific register allocation */
    }
    
    /* Array of inputs to prevent constant folding */
    int inputs[] = {1, 2, 3, 5, 7, 11, 13, 17, 19, 23};
    double fp_inputs[] = {1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1};
    
    /* Call each pattern multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        result ^= pattern_a_intensive(inputs[i]);
        result += (int)pattern_b_fp_intensive(fp_inputs[i]);
        result ^= pattern_c_complex_cfg(inputs[i]);
        result += pattern_d_vector_ops(inputs[i]);
        result ^= pattern_e_explicit_registers(inputs[i]);
        
        /* Prevent compiler from optimizing away loops */
        asm volatile ("" : : "r" (result));
    }
    
    /* Optional: Print result to prevent dead code elimination */
    printf("Final result: %d\n", result);
    
    return result % 256;
}
