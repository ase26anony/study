/* test_mcf_coverage.c
 * 
 * This test program is designed to trigger GCC's Min-Cost Flow pass
 * during register allocation, specifically to cover the special node
 * printing logic in mcf.cc's print_node function.
 *
 * Compile with: gcc -O3 -fprofile-arcs -ftest-coverage -fdump-rtl-all test_mcf_coverage.c -o test_mcf
 * Run with: ./test_mcf
 * Generate coverage: gcov test_mcf_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure each function gets its own compilation unit context */
#define NOINLINE __attribute__((noinline))
#define COLD __attribute__((cold))

/* Stub for non-GCC compilers */
#ifndef __GNUC__
#define __builtin_cpu_supports(x) 0
#endif

/* ===== PATTERN A: Integer arithmetic chain with 30+ variables ===== */
/* Creates massive register pressure in a tight loop */
NOINLINE static int pattern_a_intensive(int input) {
    /* Declare 30+ integer variables to pressure integer registers */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int a11, a12, a13, a14, a15, a16, a17, a18, a19, a20;
    int a21, a22, a23, a24, a25, a26, a27, a28, a29, a30;
    int a31, a32, a33, a34, a35;
    
    /* Initialize with input to prevent constant folding */
    a1 = input;
    
    /* Create complex dependency chain */
    a2 = a1 * 3 + 1;
    a3 = a2 - a1;
    a4 = a3 * a2;
    a5 = a4 / (a1 + 1);
    a6 = a5 ^ a3;
    a7 = a6 | a4;
    a8 = a7 & a5;
    a9 = a8 << 2;
    a10 = a9 >> 1;
    
    a11 = a10 + a9;
    a12 = a11 - a8;
    a13 = a12 * a7;
    a14 = a13 / (a6 + 1);
    a15 = a14 ^ a12;
    a16 = a15 | a13;
    a17 = a16 & a14;
    a18 = a17 << 3;
    a19 = a18 >> 2;
    a20 = a19 + a18;
    
    a21 = a20 - a17;
    a22 = a21 * a16;
    a23 = a22 / (a15 + 1);
    a24 = a23 ^ a21;
    a25 = a24 | a22;
    a26 = a25 & a23;
    a27 = a26 << 1;
    a28 = a27 >> 1;
    a29 = a28 + a27;
    a30 = a29 - a26;
    
    a31 = a30 * a25;
    a32 = a31 / (a24 + 1);
    a33 = a32 ^ a30;
    a34 = a33 | a31;
    a35 = a34 & a32;
    
    /* Use all variables in loop to create live ranges */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        /* Complex computation using all variables */
        sum += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
               a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19 + a20 +
               a21 + a22 + a23 + a24 + a25 + a26 + a27 + a28 + a29 + a30 +
               a31 + a32 + a33 + a34 + a35;
        
        /* Modify variables to prevent dead code elimination */
        a1 = (a1 * 3) % 100;
        a2 = (a2 + i) % 100;
        a3 = (a3 ^ i) % 100;
        a4 = (a4 | i) % 100;
        a5 = (a5 & i) % 100;
    }
    
    /* Force all variables to be used in return */
    asm volatile ("" : : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5),
                      "r"(a6), "r"(a7), "r"(a8), "r"(a9), "r"(a10),
                      "r"(a11), "r"(a12), "r"(a13), "r"(a14), "r"(a15),
                      "r"(a16), "r"(a17), "r"(a18), "r"(a19), "r"(a20),
                      "r"(a21), "r"(a22), "r"(a23), "r"(a24), "r"(a25),
                      "r"(a26), "r"(a27), "r"(a28), "r"(a29), "r"(a30),
                      "r"(a31), "r"(a32), "r"(a33), "r"(a34), "r"(a35));
    
    return sum % 1000;
}

/* ===== PATTERN B: Floating-point intensive computation ===== */
/* Pressures floating-point registers */
NOINLINE static double pattern_b_fp_intensive(double input) {
    /* 20+ double variables */
    double b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    double b11, b12, b13, b14, b15, b16, b17, b18, b19, b20;
    double b21, b22, b23, b24, b25;
    
    b1 = input;
    b2 = b1 * 1.1;
    b3 = b2 / 1.2;
    b4 = b3 + b2;
    b5 = b4 - b3;
    b6 = b5 * 1.3;
    b7 = b6 / 1.4;
    b8 = b7 + b6;
    b9 = b8 - b7;
    b10 = b9 * 1.5;
    
    b11 = b10 / 1.6;
    b12 = b11 + b10;
    b13 = b12 - b11;
    b14 = b13 * 1.7;
    b15 = b14 / 1.8;
    b16 = b15 + b14;
    b17 = b16 - b15;
    b18 = b17 * 1.9;
    b19 = b18 / 2.0;
    b20 = b19 + b18;
    
    b21 = b20 - b19;
    b22 = b21 * 2.1;
    b23 = b22 / 2.2;
    b24 = b23 + b22;
    b25 = b24 - b23;
    
    /* Complex loop with floating operations */
    double result = 0.0;
    for (int i = 0; i < 50; i++) {
        double t = (double)i;
        result += b1 * t + b2 / (t + 1.0) + b3 * t * t + 
                 b4 * b5 + b6 - b7 + b8 * b9 - b10 +
                 b11 * b12 / (b13 + 1.0) + b14 - b15 * b16 +
                 b17 * b18 / b19 + b20 * b21 - b22 +
                 b23 * b24 / (b25 + t);
        
        /* Prevent optimization */
        b1 = b1 * 0.99;
        b2 = b2 + 0.01;
        b3 = b3 * 1.01;
    }
    
    /* Force all FP variables to be live */
    asm volatile ("" : : "f"(b1), "f"(b2), "f"(b3), "f"(b4), "f"(b5),
                      "f"(b6), "f"(b7), "f"(b8), "f"(b9), "f"(b10),
                      "f"(b11), "f"(b12), "f"(b13), "f"(b14), "f"(b15),
                      "f"(b16), "f"(b17), "f"(b18), "f"(b19), "f"(b20),
                      "f"(b21), "f"(b22), "f"(b23), "f"(b24), "f"(b25));
    
    return result;
}

/* ===== PATTERN C: Complex control flow with switch ===== */
/* Creates many basic blocks for complex CFG */
NOINLINE static int pattern_c_complex_cfg(int input) {
    int result = input;
    
    /* Outer loop with breaks/continues */
    for (int i = 0; i < 100; i++) {
        if (i % 10 == 0) continue;
        
        /* Switch with 20+ cases */
        switch (i % 23) {
            case 0: result += 1; break;
            case 1: result *= 2; break;
            case 2: result ^= 0x55; break;
            case 3: result |= 0xAA; break;
            case 4: result &= 0xF0; break;
            case 5: result <<= 1; break;
            case 6: result >>= 1; break;
            case 7: result = ~result; break;
            case 8: result += i * 2; break;
            case 9: result -= i / 2; break;
            case 10: result = result * 3 + 1; break;
            case 11: result = (result + 7) % 256; break;
            case 12: result ^= result >> 4; break;
            case 13: result |= result << 4; break;
            case 14: result &= 0x0F0F; break;
            case 15: result = result * result % 1000; break;
            case 16: result = -result; break;
            case 17: result = abs(result); break;
            case 18: result = result / (i % 5 + 1); break;
            case 19: result = result % 100; break;
            case 20: result = result | 1; break;
            case 21: result = result & ~1; break;
            case 22: result = result ^ 0xFF; break;
        }
        
        /* Nested loop with break */
        for (int j = 0; j < 10; j++) {
            if (j == result % 5) break;
            result += j;
        }
        
        if (result > 1000) break;
    }
    
    return result;
}

/* ===== PATTERN D: Vector extensions ===== */
/* Pressures SIMD/vector registers */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

NOINLINE static int pattern_d_vector_ops(int input) {
#ifdef __GNUC__
    /* Multiple vector variables */
    v4si v1 = {input, input + 1, input + 2, input + 3};
    v4si v2 = {input * 2, input * 3, input * 4, input * 5};
    v4si v3 = {input + 10, input + 20, input + 30, input + 40};
    v4si v4 = {input - 1, input - 2, input - 3, input - 4};
    v4si v5 = {input * input, input + 100, input - 50, input ^ 0xFF};
    
    v4sf f1 = {input * 1.0f, input * 2.0f, input * 3.0f, input * 4.0f};
    v4sf f2 = {input / 2.0f, input / 3.0f, input / 4.0f, input / 5.0f};
    
    /* Vector operations in loop */
    v4si accum_int = {0, 0, 0, 0};
    v4sf accum_float = {0.0f, 0.0f, 0.0f, 0.0f};
    
    for (int i = 0; i < 100; i++) {
        v1 = v1 + v2;
        v2 = v2 * v3;
        v3 = v3 - v4;
        v4 = v4 ^ v5;
        v5 = v5 | v1;
        
        f1 = f1 + f2;
        f2 = f2 * f1;
        
        accum_int = accum_int + v1 + v2 + v3;
        accum_float = accum_float + f1 * f2;
        
        /* Prevent optimization */
        asm volatile ("" : "+x"(v1), "+x"(v2), "+x"(v3), "+x"(v4), "+x"(v5),
                          "+x"(f1), "+x"(f2));
    }
    
    /* Extract results */
    int sum = accum_int[0] + accum_int[1] + accum_int[2] + accum_int[3];
    sum += (int)(accum_float[0] + accum_float[1] + accum_float[2] + accum_float[3]);
    
    return sum % 1000;
#else
    return input;
#endif
}

/* ===== PATTERN E: Explicit register variables ===== */
/* Conflicts with allocator's choices */
NOINLINE static int pattern_e_register_conflict(int input) {
#ifdef __GNUC__
    /* Try to use specific registers */
    register int r12_var asm ("r12") = input;
    register int r13_var asm ("r13") = input * 2;
    register int r14_var asm ("r14") = input + 100;
    register int r15_var asm ("r15") = input - 50;
    
    /* Force these to be used in complex computation */
    int result = 0;
    for (int i = 0; i < 100; i++) {
        r12_var = r12_var * 3 + 1;
        r13_var = r13_var ^ r12_var;
        r14_var = r14_var | r13_var;
        r15_var = r15_var & r14_var;
        
        result += r12_var + r13_var + r14_var + r15_var;
        
        /* Force register spilling by using many temporaries */
        int t1 = r12_var * 2;
        int t2 = r13_var / 2;
        int t3 = r14_var + t1;
        int t4 = r15_var - t2;
        int t5 = t3 ^ t4;
        int t6 = t5 | t3;
        int t7 = t6 & t4;
        int t8 = t7 << 2;
        int t9 = t8 >> 1;
        int t10 = t9 + t8;
        
        result += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
    }
    
    /* Ensure register variables are marked as used */
    asm volatile ("" : : "r"(r12_var), "r"(r13_var), "r"(r14_var), "r"(r15_var));
    
    return result % 10000;
#else
    return input;
#endif
}

/* ===== Main function ===== */
COLD int main(int argc, char **argv) {
    int result = 0;
    
    /* Use CPU feature check to engage target-specific optimizations */
    if (__builtin_cpu_supports("avx2") || 
        __builtin_cpu_supports("sse4.2") ||
        __builtin_cpu_supports("popcnt")) {
        /* This may affect register allocation decisions */
        result += 1;
    }
    
    /* Array of inputs to prevent constant folding */
    int inputs[] = {1, 2, 3, 5, 7, 11, 13, 17, 19, 23};
    double fp_inputs[] = {1.0, 2.5, 3.14, 5.0, 7.7, 11.1};
    
    /* Call all patterns with different inputs */
    for (int i = 0; i < 10; i++) {
        result += pattern_a_intensive(inputs[i % 10]);
        result += pattern_c_complex_cfg(inputs[i % 10]);
        result += pattern_d_vector_ops(inputs[i % 10]);
        result += pattern_e_register_conflict(inputs[i % 10]);
        
        if (i < 6) {
            /* Cast to int to mix with integer result */
            result += (int)pattern_b_fp_intensive(fp_inputs[i]);
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %d\n", result);
    
    return result == 0 ? 0 : 1;
}
