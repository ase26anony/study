/* test_mcf_coverage.c
 * 
 * This test program is designed to trigger GCC's Min-Cost Flow pass
 * during register allocation, specifically to exercise the print_node
 * function with special node indices: ENTRY_BLOCK, EXIT_BLOCK,
 * new_exit_index, and new_entry_index.
 *
 * Compile with: gcc -O3 -fprofile-arcs -ftest-coverage -fdump-rtl-all -c test_mcf_coverage.c
 * Then run: ./test_mcf_coverage (to generate profile data)
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
    int a31, a32, a33, a34, a35, a36, a37, a38, a39, a40;
    
    volatile int sum = 0; /* volatile to prevent optimization */
    
    /* Complex interdependent computations */
    a1 = input + 1;
    a2 = a1 * 2 - input;
    a3 = a2 + a1;
    a4 = a3 ^ a2;
    a5 = a4 * a3;
    a6 = a5 / (a4 ? a4 : 1);
    a7 = a6 << 2;
    a8 = a7 | a6;
    a9 = a8 & 0xFFFF;
    a10 = a9 - a8;
    
    a11 = a10 * a9;
    a12 = a11 + a10;
    a13 = a12 ^ a11;
    a14 = a13 * 3;
    a15 = a14 / 2;
    a16 = a15 << 1;
    a17 = a16 | 0xFF;
    a18 = a17 & a16;
    a19 = a18 - a17;
    a20 = a19 * a18;
    
    a21 = a20 + a19;
    a22 = a21 ^ a20;
    a23 = a22 * 5;
    a24 = a23 / 3;
    a25 = a24 << 3;
    a26 = a25 | 0xAA;
    a27 = a26 & a25;
    a28 = a27 - a26;
    a29 = a28 * a27;
    a30 = a29 + a28;
    
    a31 = a30 ^ a29;
    a32 = a31 * 7;
    a33 = a32 / 4;
    a34 = a33 << 2;
    a35 = a34 | 0x55;
    a36 = a35 & a34;
    a37 = a36 - a35;
    a38 = a37 * a36;
    a39 = a38 + a37;
    a40 = a39 ^ a38;
    
    /* Force all variables to be used in a way that prevents dead code elimination */
    asm volatile("" : : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5));
    asm volatile("" : : "r"(a6), "r"(a7), "r"(a8), "r"(a9), "r"(a10));
    asm volatile("" : : "r"(a11), "r"(a12), "r"(a13), "r"(a14), "r"(a15));
    asm volatile("" : : "r"(a16), "r"(a17), "r"(a18), "r"(a19), "r"(a20));
    asm volatile("" : : "r"(a21), "r"(a22), "r"(a23), "r"(a24), "r"(a25));
    asm volatile("" : : "r"(a26), "r"(a27), "r"(a28), "r"(a29), "r"(a30));
    asm volatile("" : : "r"(a31), "r"(a32), "r"(a33), "r"(a34), "r"(a35));
    asm volatile("" : : "r"(a36), "r"(a37), "r"(a38), "r"(a39), "r"(a40));
    
    /* Complex loop with register pressure */
    for (int i = 0; i < 100; i++) {
        /* Mix all variables in computation */
        sum += a1 + a2 - a3 + a4 - a5 + a6 - a7 + a8 - a9 + a10 +
               a11 - a12 + a13 - a14 + a15 - a16 + a17 - a18 + a19 - a20 +
               a21 - a22 + a23 - a24 + a25 - a26 + a27 - a28 + a29 - a30 +
               a31 - a32 + a33 - a34 + a35 - a36 + a37 - a38 + a39 - a40;
        
        /* Modify some variables to prevent loop invariant removal */
        if (i % 2 == 0) {
            a1 += i;
            a20 -= i;
            a40 ^= i;
        } else {
            a10 *= (i % 5) + 1;
            a30 /= (i % 3) + 1;
        }
        
        /* Nested control flow to create complex CFG */
        switch (i % 7) {
            case 0: a5++; break;
            case 1: a15--; break;
            case 2: a25 ^= 0xF; break;
            case 3: a35 |= 0x1; break;
            case 4: a6 <<= 1; break;
            case 5: a16 >>= 1; break;
            case 6: a26 = ~a26; break;
        }
    }
    
    return sum;
}

/* ============================================
 * PATTERN B: Floating-point intensive computation
 * Pressures floating-point and SIMD registers
 * ============================================ */
NOINLINE static double pattern_b_float_pressure(double input) {
    /* 20+ double variables to pressure FP registers */
    double b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    double b11, b12, b13, b14, b15, b16, b17, b18, b19, b20;
    double b21, b22, b23, b24, b25;
    
    volatile double result = 0.0;
    
    /* FP computation chain */
    b1 = input + 1.0;
    b2 = b1 * 2.0 - input;
    b3 = b2 + b1;
    b4 = b3 * b2;
    b5 = b4 / (b3 + 1.0);
    b6 = b5 * 3.14159;
    b7 = b6 / 2.71828;
    b8 = b7 + b6;
    b9 = b8 - b7;
    b10 = b9 * b8;
    
    b11 = b10 / (b9 + 1.0);
    b12 = b11 * 1.41421;
    b13 = b12 + b11;
    b14 = b13 - b12;
    b15 = b14 * b13;
    b16 = b15 / 1.73205;
    b17 = b16 + b15;
    b18 = b17 - b16;
    b19 = b18 * b17;
    b20 = b19 / 2.23607;
    
    b21 = b20 + b19;
    b22 = b21 - b20;
    b23 = b22 * b21;
    b24 = b23 / 3.14159;
    b25 = b24 + b23;
    
    /* Force FP register usage */
    asm volatile("" : : "f"(b1), "f"(b2), "f"(b3), "f"(b4), "f"(b5));
    asm volatile("" : : "f"(b6), "f"(b7), "f"(b8), "f"(b9), "f"(b10));
    asm volatile("" : : "f"(b11), "f"(b12), "f"(b13), "f"(b14), "f"(b15));
    asm volatile("" : : "f"(b16), "f"(b17), "f"(b18), "f"(b19), "f"(b20));
    asm volatile("" : : "f"(b21), "f"(b22), "f"(b23), "f"(b24), "f"(b25));
    
    /* Loop with mixed FP operations */
    for (int i = 0; i < 50; i++) {
        double temp = (double)i;
        
        /* Complex FP expressions */
        result += b1 * temp + b2 / (temp + 1.0) - b3 * b4 + b5 / b6 -
                  b7 * b8 + b9 / b10 + b11 * b12 - b13 / b14 +
                  b15 * b16 - b17 / b18 + b19 * b20 - b21 / b22 +
                  b23 * b24 - b25 / (temp + 2.0);
        
        /* Modify FP variables */
        if (i % 3 == 0) {
            b1 += 0.1;
            b10 *= 1.01;
            b20 -= 0.05;
        } else if (i % 3 == 1) {
            b5 /= 1.1;
            b15 *= 0.99;
            b25 += 0.02;
        }
        
        /* Nested loops with breaks/continues for CFG complexity */
        for (int j = 0; j < 5; j++) {
            if (j == 2) continue;
            result += b1 * j;
            if (j == 4) break;
        }
    }
    
    return result;
}

/* ============================================
 * PATTERN C: Complex control flow with switch
 * Creates many basic blocks for CFG complexity
 * ============================================ */
NOINLINE static int pattern_c_cfg_complexity(int input) {
    int result = input;
    
    /* Large switch statement with 20+ cases */
    for (int i = 0; i < 100; i++) {
        switch (i % 23) {  /* 23 cases for many basic blocks */
            case 0:  result += i * 2; break;
            case 1:  result -= i / 2; break;
            case 2:  result ^= i; break;
            case 3:  result |= 0xFF; break;
            case 4:  result &= ~0x0F; break;
            case 5:  result <<= 1; break;
            case 6:  result >>= 2; break;
            case 7:  result = ~result; break;
            case 8:  result += result * 3; break;
            case 9:  result -= result / 4; break;
            case 10: result ^= 0xAA; break;
            case 11: result |= 0x55; break;
            case 12: result &= 0xF0; break;
            case 13: result <<= 2; break;
            case 14: result >>= 1; break;
            case 15: result = result * result; break;
            case 16: result += 0x1234; break;
            case 17: result -= 0x5678; break;
            case 18: result ^= 0x9ABC; break;
            case 19: result |= 0xDEF0; break;
            case 20: result &= 0x0F0F; break;
            case 21: result <<= 3; break;
            case 22: result >>= 3; break;
        }
        
        /* Additional control flow with goto (GCC extension) */
        if (i % 7 == 0) {
            /* Computed goto to create irreducible CFG */
            static void *labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4 };
            goto *labels[i % 5];
            
            L0: result += 1; goto end_label;
            L1: result += 2; goto end_label;
            L2: result += 3; goto end_label;
            L3: result += 4; goto end_label;
            L4: result += 5; goto end_label;
            end_label:;
        }
        
        /* Nested switch for extra complexity */
        switch (result % 5) {
            case 0: result++; break;
            case 1: result--; break;
            case 2: result *= 2; break;
            case 3: result /= 2; break;
            case 4: result = -result; break;
        }
    }
    
    return result;
}

/* ============================================
 * PATTERN D: Vector/SIMD operations
 * Pressures vector registers
 * ============================================ */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

NOINLINE static v4si pattern_d_vector_pressure(v4si input) {
    /* Multiple vector variables */
    v4si v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    v4si v11, v12, v13, v14, v15;
    
    v1 = input + (v4si){1, 2, 3, 4};
    v2 = v1 * (v4si){2, 3, 4, 5};
    v3 = v2 - v1;
    v4 = v3 & (v4si){0xFF, 0xFF, 0xFF, 0xFF};
    v5 = v4 | (v4si){0xF0, 0xF0, 0xF0, 0xF0};
    v6 = v5 ^ (v4si){0xAA, 0xAA, 0xAA, 0xAA};
    v7 = v6 << (v4si){1, 2, 1, 2};
    v8 = v7 >> (v4si){1, 1, 2, 2};
    v9 = v8 + v7;
    v10 = v9 - v8;
    
    v11 = v10 * v9;
    v12 = v11 / (v10 + (v4si){1, 1, 1, 1});
    v13 = v12 & v11;
    v14 = v13 | v12;
    v15 = v14 ^ v13;
    
    /* Force vector register usage */
    asm volatile("" : : "x"(v1), "x"(v2), "x"(v3), "x"(v4), "x"(v5));
    asm volatile("" : : "x"(v6), "x"(v7), "x"(v8), "x"(v9), "x"(v10));
    asm volatile("" : : "x"(v11), "x"(v12), "x"(v13), "x"(v14), "x"(v15));
    
    /* Loop with vector operations */
    v4si result = {0, 0, 0, 0};
    for (int i = 0; i < 50; i++) {
        v4si idx = {i, i+1, i+2, i+3};
        result += v1 * idx + v2 / (idx + (v4si){1,1,1,1}) - 
                  v3 * v4 + v5 / (v6 + (v4si){1,1,1,1}) +
                  v7 * v8 - v9 / (v10 + (v4si){1,1,1,1}) +
                  v11 * v12 - v13 / (v14 + (v4si){1,1,1,1}) +
                  v15 * idx;
        
        /* Modify vectors */
        if (i % 4 == 0) {
            v1 += idx;
            v8 -= idx;
            v15 ^= idx;
        }
    }
    
    return result;
}
#endif

/* ============================================
 * PATTERN E: Explicit register variables
 * Conflicts with allocator's choices
 * ============================================ */
NOINLINE static int pattern_e_explicit_registers(int input) {
    /* Explicit register variables that conflict with allocator */
    register int r1 asm ("r12") = input + 1;
    register int r2 asm ("r13") = r1 * 2;
    register int r3 asm ("r14") = r2 + r1;
    register int r4 asm ("r15") = r3 ^ r2;
    
    /* More variables without explicit registers to cause conflicts */
    int v1 = r4 * 3;
    int v2 = v1 / 2;
    int v3 = v2 << 1;
    int v4 = v3 | 0xFF;
    int v5 = v4 & r1;
    int v6 = v5 - r2;
    int v7 = v6 * r3;
    int v8 = v7 / (r4 ? r4 : 1);
    int v9 = v8 << 2;
    int v10 = v9 | r1;
    
    /* Force register usage */
    asm volatile("" : : "r"(r1), "r"(r2), "r"(r3), "r"(r4));
    asm volatile("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4), "r"(v5));
    asm volatile("" : : "r"(v6), "r"(v7), "r"(v8), "r"(v9), "r"(v10));
    
    int result = 0;
    for (int i = 0; i < 100; i++) {
        /* Mix explicit and implicit registers */
        result += r1 * i - r2 / (i + 1) + r3 ^ r4 - 
                  v1 * v2 + v3 / (v4 + 1) - v5 ^ v6 +
                  v7 * v8 - v9 / (v10 + 1);
        
        /* Complex control flow with early returns */
        if (i % 13 == 0) {
            result += 1000;
            if (result > 10000) {
                /* Early return creates additional CFG edges */
                return result;
            }
        } else if (i % 17 == 0) {
            result -= 500;
            if (result < -1000) {
                /* Another early return */
                return result;
            }
        }
        
        /* Modify register variables */
        switch (i % 5) {
            case 0: r1++; break;
            case 1: r2--; break;
            case 2: r3 ^= i; break;
            case 3: r4 |= 0xF; break;
            case 4: r1 = r2 + r3; break;
        }
    }
    
    return result;
}

/* ============================================
 * Main function with cold attribute
 * Calls all patterns with varying inputs
 * ============================================ */
COLD int main(int argc, char *argv[]) {
    int int_result = 0;
    double float_result = 0.0;
    
    /* Use CPU features to engage target-specific optimizations */
    if (__builtin_cpu_supports("avx2")) {
        /* This may affect register allocation strategy */
        int_result += 1000;
    }
    if (__builtin_cpu_supports("sse4.2")) {
        int_result += 500;
    }
    
    /* Array of inputs to prevent constant folding */
    int int_inputs[] = {1, 2, 3, 5, 7, 11, 13, 17, 19, 23};
    double float_inputs[] = {1.0, 2.5, 3.14159, 2.71828, 1.41421, 1.73205};
    
    /* Call pattern A multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        int_result += pattern_a_int_pressure(int_inputs[i % 10]);
    }
    
    /* Call pattern B */
    for (int i = 0; i < 6; i++) {
        float_result += pattern_b_float_pressure(float_inputs[i % 6]);
    }
    
    /* Call pattern C */
    for (int i = 0; i < 10; i++) {
        int_result += pattern_c_cfg_complexity(int_inputs[i % 10]);
    }
    
    /* Call pattern D if supported */
    #ifdef __GNUC__
    v4si vec_input = {1, 2, 3, 4};
    v4si vec_result = pattern_d_vector_pressure(vec_input);
    /* Extract result to prevent optimization */
    int vec_sum = vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
    int_result += vec_sum;
    #endif
    
    /* Call pattern E */
    for (int i = 0; i < 10; i++) {
        int_result += pattern_e_explicit_registers(int_inputs[i % 10]);
    }
    
    /* Mix results to create final output */
    int final_result = int_result + (int)float_result;
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(final_result));
    
    /* Simple output to verify execution */
    printf("Test completed. Final value: %d\n", final_result % 1000);
    
    return final_result % 256;  /* Return non-zero exit code */
}
