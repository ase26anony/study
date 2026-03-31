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

/* Prevent inlining to ensure separate compilation units in same file */
#define NOINLINE __attribute__((noinline))
#define NORETURN __attribute__((noreturn))
#define COLD __attribute__((cold))

/* Stub for non-GCC compilers */
#ifndef __GNUC__
#define __builtin_cpu_supports(x) 0
#endif

/* ============================================
 * PATTERN A: Integer arithmetic chain with 30+ variables
 * Creates massive register pressure in a loop
 * ============================================ */
NOINLINE static int pattern_a_int_pressure(int input) {
    /* 30+ integer variables to pressure general purpose registers */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int a11, a12, a13, a14, a15, a16, a17, a18, a19, a20;
    int a21, a22, a23, a24, a25, a26, a27, a28, a29, a30;
    int i, result;
    
    /* Initialize with input to prevent constant folding */
    a1 = input + 1;
    a2 = a1 * 2;
    a3 = a2 - input;
    a4 = a3 ^ a1;
    a5 = a4 | a2;
    a6 = a5 & a3;
    a7 = a6 + a4;
    a8 = a7 - a5;
    a9 = a8 * a6;
    a10 = a9 / (a7 ? a7 : 1);
    a11 = a10 ^ a8;
    a12 = a11 | a9;
    a13 = a12 & a10;
    a14 = a13 + a11;
    a15 = a14 - a12;
    a16 = a15 * a13;
    a17 = a16 / (a14 ? a14 : 1);
    a18 = a17 ^ a15;
    a19 = a18 | a16;
    a20 = a19 & a17;
    a21 = a20 + a18;
    a22 = a21 - a19;
    a23 = a22 * a20;
    a24 = a23 / (a21 ? a21 : 1);
    a25 = a24 ^ a22;
    a26 = a25 | a23;
    a27 = a26 & a24;
    a28 = a27 + a25;
    a29 = a28 - a26;
    a30 = a29 * a27;
    
    /* Complex loop with interdependencies */
    result = 0;
    for (i = 0; i < 100; i++) {
        /* Rotate values through all variables */
        int tmp = a1;
        a1 = a2 + i; a2 = a3 - i; a3 = a4 ^ i; a4 = a5 | tmp;
        a5 = a6 & i; a6 = a7 + a1; a7 = a8 - a2; a8 = a9 * a3;
        a9 = a10 / (a4 ? a4 : 1); a10 = a11 ^ a5; a11 = a12 | a6;
        a12 = a13 & a7; a13 = a14 + a8; a14 = a15 - a9; a15 = a16 * a10;
        a16 = a17 / (a11 ? a11 : 1); a17 = a18 ^ a12; a18 = a19 | a13;
        a19 = a20 & a14; a20 = a21 + a15; a21 = a22 - a16; a22 = a23 * a17;
        a23 = a24 / (a18 ? a18 : 1); a24 = a25 ^ a19; a25 = a26 | a20;
        a26 = a27 & a21; a27 = a28 + a22; a28 = a29 - a23; a29 = a30 * a24;
        a30 = tmp ^ i;
        
        result += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
                  a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19 + a20 +
                  a21 + a22 + a23 + a24 + a25 + a26 + a27 + a28 + a29 + a30;
        
        /* Add control flow complexity */
        if (i % 7 == 0) continue;
        if (i % 13 == 0) break;
        if (i % 17 == 0) i += 2;
    }
    
    /* Final computation using all variables */
    return result + a1 - a2 + a3 - a4 + a5 - a6 + a7 - a8 + a9 - a10 +
           a11 - a12 + a13 - a14 + a15 - a16 + a17 - a18 + a19 - a20 +
           a21 - a22 + a23 - a24 + a25 - a26 + a27 - a28 + a29 - a30;
}

/* ============================================
 * PATTERN B: Floating-point intensive computation
 * Pressures floating-point and MMX/SSE registers
 * ============================================ */
NOINLINE static double pattern_b_float_pressure(double input) {
    /* 20+ double variables */
    double b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    double b11, b12, b13, b14, b15, b16, b17, b18, b19, b20;
    int i;
    double result = 0.0;
    
    b1 = input + 1.0;
    b2 = b1 * 2.0;
    b3 = b2 - input;
    b4 = b3 / (b1 + 0.001);
    b5 = b4 * b2;
    b6 = b5 - b3;
    b7 = b6 / (b4 + 0.001);
    b8 = b7 * b5;
    b9 = b8 - b6;
    b10 = b9 / (b7 + 0.001);
    b11 = b10 * b8;
    b12 = b11 - b9;
    b13 = b12 / (b10 + 0.001);
    b14 = b13 * b11;
    b15 = b14 - b12;
    b16 = b15 / (b13 + 0.001);
    b17 = b16 * b14;
    b18 = b17 - b15;
    b19 = b18 / (b16 + 0.001);
    b20 = b19 * b17;
    
    /* Loop with floating-point operations */
    for (i = 0; i < 50; i++) {
        double t = (double)i * 0.1;
        
        /* Chain computations */
        b1 = b2 + t; b2 = b3 - t; b3 = b4 * t; b4 = b5 / (t + 0.001);
        b5 = b6 + b1; b6 = b7 - b2; b7 = b8 * b3; b8 = b9 / (b4 + 0.001);
        b9 = b10 + b5; b10 = b11 - b6; b11 = b12 * b7; b12 = b13 / (b8 + 0.001);
        b13 = b14 + b9; b14 = b15 - b10; b15 = b16 * b11; b16 = b17 / (b12 + 0.001);
        b17 = b18 + b13; b18 = b19 - b14; b19 = b20 * b15; b20 = b1 / (b16 + 0.001);
        
        result += b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9 + b10 +
                  b11 + b12 + b13 + b14 + b15 + b16 + b17 + b18 + b19 + b20;
        
        /* Prevent optimization */
        asm volatile("" : "+rm"(result));
    }
    
    return result;
}

/* ============================================
 * PATTERN C: Complex control flow with switch statement
 * Creates many basic blocks for CFG complexity
 * ============================================ */
NOINLINE static int pattern_c_control_flow(int input) {
    int result = input;
    int i, j;
    
    /* Outer loop */
    for (i = 0; i < 20; i++) {
        /* Inner loop with switch */
        for (j = 0; j < 30; j++) {
            switch ((i * j + input) % 23) {
                case 0:  result += i * 2; break;
                case 1:  result -= j * 3; break;
                case 2:  result ^= i + j; break;
                case 3:  result |= i << 2; break;
                case 4:  result &= j << 3; break;
                case 5:  result += result * 2; break;
                case 6:  result -= result / 2; break;
                case 7:  result ^= ~i; break;
                case 8:  result |= 0xFF00; break;
                case 9:  result &= 0x00FF; break;
                case 10: result = result << 1; break;
                case 11: result = result >> 1; break;
                case 12: result = result * 3; break;
                case 13: result = result / 3; break;
                case 14: result = -result; break;
                case 15: result = ~result; break;
                case 16: result = result + 0x1234; break;
                case 17: result = result - 0x5678; break;
                case 18: result = result ^ 0x9ABC; break;
                case 19: result = result | 0xDEF0; break;
                case 20: result = result & 0x0F0F; break;
                case 21: result = (result << 4) | (result >> 28); break;
                case 22: result = (result >> 4) | (result << 28); break;
            }
            
            /* Additional control flow */
            if (j % 5 == 0) continue;
            if (j % 11 == 0) break;
            if (j % 7 == 0) j += 1;
        }
        
        /* More control flow in outer loop */
        if (i % 3 == 0) {
            result += 1000;
            continue;
        }
        if (i % 7 == 0) {
            result -= 500;
            break;
        }
    }
    
    return result;
}

/* ============================================
 * PATTERN D: Vector/SIMD operations
 * Pressures vector registers
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
    
    int i, result = 0;
    int* ptr;
    
    /* Loop with vector operations */
    for (i = 0; i < 40; i++) {
        /* Integer vector operations */
        vec1 = vec1 + vec2;
        vec2 = vec2 - vec3;
        vec3 = vec3 * vec4;
        vec4 = vec4 & vec5;
        vec5 = vec5 | vec6;
        vec6 = vec6 ^ vec7;
        vec7 = vec7 + vec8;
        vec8 = vec8 - vec1;
        
        /* Floating vector operations */
        fvec1 = fvec1 + fvec2;
        fvec2 = fvec2 * fvec1;
        
        /* Extract results to prevent dead code elimination */
        ptr = (int*)&vec1; result += ptr[0] + ptr[1] + ptr[2] + ptr[3];
        ptr = (int*)&vec2; result += ptr[0] + ptr[1] + ptr[2] + ptr[3];
        ptr = (int*)&vec3; result += ptr[0] + ptr[1] + ptr[2] + ptr[3];
        ptr = (int*)&fvec1; result += (int)ptr[0] + (int)ptr[1];
        
        /* Control flow */
        if (i % 9 == 0) {
            vec1 = vec1 << 1;
            continue;
        }
        if (i % 13 == 0) break;
    }
    
    return result;
}

/* ============================================
 * PATTERN E: Explicit register variables
 * Conflicts with allocator's choices
 * ============================================ */
NOINLINE static int pattern_e_explicit_registers(int input) {
    /* Try to use specific registers */
    register int r1 asm ("r12") = input;
    register int r2 asm ("r13") = input + 1;
    register int r3 asm ("r14") = input + 2;
    register int r4 asm ("r15") = input + 3;
    
    int var1, var2, var3, var4, var5, var6, var7, var8;
    int i, result = 0;
    
    /* Mix explicit register vars with regular vars */
    var1 = r1 * 2;
    var2 = r2 + var1;
    var3 = r3 - var2;
    var4 = r4 ^ var3;
    var5 = var1 | var4;
    var6 = var2 & var5;
    var7 = var3 + var6;
    var8 = var4 - var7;
    
    /* Loop that uses all variables */
    for (i = 0; i < 25; i++) {
        /* Rotate through explicit registers */
        int tmp = r1;
        r1 = r2 + i;
        r2 = r3 - i;
        r3 = r4 * i;
        r4 = tmp / (i ? i : 1);
        
        /* Use all variables */
        var1 = var2 + r1;
        var2 = var3 - r2;
        var3 = var4 * r3;
        var4 = var5 / (r4 ? r4 : 1);
        var5 = var6 ^ var1;
        var6 = var7 | var2;
        var7 = var8 & var3;
        var8 = var1 + var4;
        
        result += r1 + r2 + r3 + r4 + var1 + var2 + var3 + var4 + 
                  var5 + var6 + var7 + var8;
        
        /* Complex control flow */
        switch (i % 8) {
            case 0: r1 += 10; break;
            case 1: r2 -= 5; break;
            case 2: r3 *= 2; break;
            case 3: r4 ^= 0xFF; break;
            case 4: var1 <<= 1; break;
            case 5: var2 >>= 1; break;
            case 6: continue;
            case 7: break;
        }
    }
    
    return result;
}

/* ============================================
 * Main function - calls all patterns
 * ============================================ */
COLD int main(int argc, char** argv) {
    int i, result = 0;
    double dresult = 0.0;
    
    /* Use CPU features to engage target-specific optimizations */
    if (__builtin_cpu_supports("avx2")) {
        /* This may affect register allocation strategies */
        result += 1000;
    }
    if (__builtin_cpu_supports("sse4.2")) {
        result += 500;
    }
    
    /* Call each pattern multiple times with different inputs */
    for (i = 0; i < 10; i++) {
        /* Pattern A - Integer pressure */
        result += pattern_a_int_pressure(i);
        
        /* Pattern B - Floating point */
        dresult += pattern_b_float_pressure(i * 0.5);
        
        /* Pattern C - Control flow */
        result += pattern_c_control_flow(i);
        
        /* Pattern D - Vector ops */
        result += pattern_d_vector_ops(i);
        
        /* Pattern E - Explicit registers */
        result += pattern_e_explicit_registers(i);
        
        /* Prevent loop unrolling from simplifying too much */
        asm volatile("" : "+r"(result), "+r"(i));
    }
    
    /* Convert double to int for final result */
    result += (int)dresult;
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %d\n", result);
    
    /* Use result to affect exit code */
    return result % 256;
}
