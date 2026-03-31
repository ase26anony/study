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
#define COLD __attribute__((cold))

/* Stub for non-GCC compilers */
#ifndef __GNUC__
#define __builtin_cpu_supports(x) 0
#endif

/* Vector type for SIMD pressure */
typedef int v4si __attribute__((vector_size(16)));

/* ============================================================
 * PATTERN A: Integer arithmetic chain with 30+ variables
 * Creates massive register pressure in a tight loop
 * ============================================================ */
NOINLINE static int pattern_a_int_pressure(int input) {
    /* 30+ integer variables to pressure general purpose registers */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int a11, a12, a13, a14, a15, a16, a17, a18, a19, a20;
    int a21, a22, a23, a24, a25, a26, a27, a28, a29, a30;
    int a31, a32, a33, a34, a35, a36, a37, a38, a39, a40;
    
    /* Complex initialization chain */
    a1 = input + 1;
    a2 = a1 * 2 - input;
    a3 = a2 + a1;
    a4 = a3 ^ a2;
    a5 = a4 * 3 + a1;
    a6 = a5 - a3;
    a7 = a6 | a4;
    a8 = a7 & a5;
    a9 = a8 * a6;
    a10 = a9 / (a1 + 1);
    
    a11 = a10 << 2;
    a12 = a11 >> 1;
    a13 = a12 + a9;
    a14 = a13 - a8;
    a15 = a14 * a7;
    a16 = a15 % (a6 + 1);
    a17 = a16 ^ a15;
    a18 = a17 | a14;
    a19 = a18 & a13;
    a20 = a19 * 7;
    
    a21 = a20 + a12;
    a22 = a21 - a11;
    a23 = a22 * a10;
    a24 = a23 / (a9 + 1);
    a25 = a24 << 3;
    a26 = a25 >> 2;
    a27 = a26 + a23;
    a28 = a27 - a22;
    a29 = a28 * a21;
    a30 = a29 % (a20 + 1);
    
    a31 = a30 ^ a29;
    a32 = a31 | a28;
    a33 = a32 & a27;
    a34 = a33 * 11;
    a35 = a34 + a26;
    a36 = a35 - a25;
    a37 = a36 * a24;
    a38 = a37 / (a23 + 1);
    a39 = a38 << 1;
    a40 = a39 >> 1;
    
    /* Complex loop with all variables used */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        /* Force all variables to be live across iterations */
        asm volatile ("" : : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5));
        asm volatile ("" : : "r"(a6), "r"(a7), "r"(a8), "r"(a9), "r"(a10));
        asm volatile ("" : : "r"(a11), "r"(a12), "r"(a13), "r"(a14), "r"(a15));
        asm volatile ("" : : "r"(a16), "r"(a17), "r"(a18), "r"(a19), "r"(a20));
        asm volatile ("" : : "r"(a21), "r"(a22), "r"(a23), "r"(a24), "r"(a25));
        asm volatile ("" : : "r"(a26), "r"(a27), "r"(a28), "r"(a29), "r"(a30));
        asm volatile ("" : : "r"(a31), "r"(a32), "r"(a33), "r"(a34), "r"(a35));
        asm volatile ("" : : "r"(a36), "r"(a37), "r"(a38), "r"(a39), "r"(a40));
        
        /* Interdependent computations */
        a1 = (a1 + a40) ^ i;
        a2 = a2 * a39 + i;
        a3 = a3 - a38 | i;
        a4 = a4 & a37 ^ i;
        a5 = a5 | a36 + i;
        
        sum += a1 + a2 + a3 + a4 + a5;
    }
    
    return sum + a40;
}

/* ============================================================
 * PATTERN B: Floating-point intensive computation
 * Pressures floating-point and SSE registers
 * ============================================================ */
NOINLINE static double pattern_b_fp_pressure(double input) {
    /* 20+ double variables */
    double b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    double b11, b12, b13, b14, b15, b16, b17, b18, b19, b20;
    
    b1 = input + 1.0;
    b2 = b1 * 2.0 - input;
    b3 = b2 + b1;
    b4 = b3 * b2;
    b5 = b4 / (b1 + 1.0);
    b6 = b5 - b3;
    b7 = b6 * b4;
    b8 = b7 + b5;
    b9 = b8 - b6;
    b10 = b9 * b7;
    
    b11 = b10 / (b8 + 1.0);
    b12 = b11 + b9;
    b13 = b12 * b10;
    b14 = b13 - b11;
    b15 = b14 / (b12 + 1.0);
    b16 = b15 * b13;
    b17 = b16 + b14;
    b18 = b17 - b15;
    b19 = b18 * b16;
    b20 = b19 / (b17 + 1.0);
    
    /* Nested loops with floating-point operations */
    double result = 0.0;
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 10; j++) {
            /* Complex FP operations keeping all variables live */
            b1 = b1 * 1.01 + b20;
            b2 = b2 / 1.01 - b19;
            b3 = b3 + b18 * 0.5;
            b4 = b4 - b17 / 2.0;
            b5 = b5 * b16 + 0.1;
            
            b6 = b6 / b15 - 0.1;
            b7 = b7 + b14 * (i + 1);
            b8 = b8 - b13 / (j + 1);
            b9 = b9 * b12 + i * j;
            b10 = b10 / b11 - i + j;
            
            /* Use all variables to prevent elimination */
            result += b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9 + b10 +
                     b11 + b12 + b13 + b14 + b15 + b16 + b17 + b18 + b19 + b20;
        }
        
        /* Break/continue to create complex CFG */
        if (i % 7 == 0) continue;
        if (i % 13 == 0) break;
    }
    
    return result;
}

/* ============================================================
 * PATTERN C: Complex control flow with switch statement
 * Creates many basic blocks for MCF to analyze
 * ============================================================ */
NOINLINE static int pattern_c_cfg_complexity(int input) {
    int result = input;
    
    /* Outer loop with switch inside */
    for (int i = 0; i < 100; i++) {
        switch (i % 23) {  /* 23 cases for many basic blocks */
            case 0: result += i * 2; break;
            case 1: result ^= i; result *= 3; break;
            case 2: result |= 0xFF; result -= i; break;
            case 3: result &= 0xF0; result += i * i; break;
            case 4: result <<= 2; result |= 1; break;
            case 5: result >>= 1; result ^= 0xAA; break;
            case 6: result = ~result; result += i; break;
            case 7: result = result % 17; result *= i; break;
            case 8: result = result / (i % 5 + 1); break;
            case 9: result = -result; result |= i; break;
            case 10: result = result & 0x0F0F; result <<= 4; break;
            case 11: result = result | 0x3333; result >>= 2; break;
            case 12: result = result ^ 0x5555; result += 1111; break;
            case 13: result = result * 7; result &= 0xFF; break;
            case 14: result = result - 1000; result |= 0x1000; break;
            case 15: result = result + 999; result ^= 0x9999; break;
            case 16: result = result % 23; result <<= 3; break;
            case 17: result = result / 2; result |= 0x8000; break;
            case 18: result = ~result & 0x7FFF; break;
            case 19: result = result + (i << 8); break;
            case 20: result = result - (i << 4); result &= 0xFFF; break;
            case 21: result = result * result % 1000; break;
            case 22: result = (result + i) ^ 0xABCD; break;
            default: result = 0; /* Should never happen */
        }
        
        /* Additional control flow with computed goto (GCC extension) */
        static void* labels[] = {
            &&label0, &&label1, &&label2, &&label3, &&label4
        };
        
        if (i % 5 == 0) {
            goto *labels[i % 5];
        }
        
        label0: result += 1;
        label1: result *= 2;
        label2: result -= 3;
        label3: result |= 4;
        label4: result &= 0xF;
    }
    
    return result;
}

/* ============================================================
 * PATTERN D: SIMD vector operations
 * Pressures vector/SIMD registers
 * ============================================================ */
NOINLINE static v4si pattern_d_simd_pressure(v4si input) {
    v4si v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    
    /* Initialize vectors */
    v1 = input + (v4si){1, 2, 3, 4};
    v2 = v1 * (v4si){2, 3, 4, 5};
    v3 = v2 - v1;
    v4 = v3 & (v4si){0xFF, 0xFF, 0xFF, 0xFF};
    v5 = v4 | (v4si){0x100, 0x200, 0x300, 0x400};
    v6 = v5 ^ (v4si){0x111, 0x222, 0x333, 0x444};
    v7 = v6 << (v4si){1, 2, 1, 2};
    v8 = v7 >> (v4si){1, 1, 2, 2};
    v9 = v8 + v6;
    v10 = v9 - v5;
    
    /* Loop with vector operations */
    for (int i = 0; i < 100; i++) {
        /* Rotate through vector operations */
        v1 = v1 + v10;
        v2 = v2 * v9;
        v3 = v3 - v8;
        v4 = v4 & v7;
        v5 = v5 | v6;
        v6 = v6 ^ v5;
        v7 = v7 << (v4si){i%4, (i+1)%4, (i+2)%4, (i+3)%4};
        v8 = v8 >> (v4si){(i+3)%4, (i+2)%4, (i+1)%4, i%4};
        v9 = v9 + (v4si){i, i+1, i+2, i+3};
        v10 = v10 - (v4si){i, i-1, i-2, i-3};
        
        /* Prevent dead code elimination */
        asm volatile ("" : : "x"(v1), "x"(v2), "x"(v3), "x"(v4), "x"(v5));
        asm volatile ("" : : "x"(v6), "x"(v7), "x"(v8), "x"(v9), "x"(v10));
    }
    
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
}

/* ============================================================
 * PATTERN E: Explicit register variables
 * Conflicts with allocator's choices, prompting fixup
 * ============================================================ */
NOINLINE static int pattern_e_register_conflict(int input) {
    /* Explicit register variables that conflict with allocator */
    register int r1 asm ("r12") = input + 1;
    register int r2 asm ("r13") = r1 * 2;
    register int r3 asm ("r14") = r2 + input;
    register int r4 asm ("r15") = r3 ^ r2;
    
    /* Many other variables that will need registers */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    
    /* Force interaction between register and regular variables */
    v1 = r1 + 1;
    v2 = r2 - v1;
    v3 = r3 * v2;
    v4 = r4 ^ v3;
    
    v5 = v1 + v2 + v3 + v4;
    v6 = v5 * r1;
    v7 = v6 / (r2 + 1);
    v8 = v7 | r3;
    v9 = v8 & r4;
    v10 = v9 << 2;
    
    v11 = v10 >> 1;
    v12 = v11 + v9;
    v13 = v12 - v8;
    v14 = v13 * v7;
    v15 = v14 % (v6 + 1);
    v16 = v15 ^ v14;
    v17 = v16 | v13;
    v18 = v17 & v12;
    v19 = v18 * 7;
    v20 = v19 + v11;
    
    /* Complex loop using all variables */
    int sum = 0;
    for (int i = 0; i < 50; i++) {
        /* Update register variables */
        asm volatile ("# Force register use" : "+r"(r1), "+r"(r2), "+r"(r3), "+r"(r4));
        
        r1 = r1 + v20;
        r2 = r2 * v19;
        r3 = r3 - v18;
        r4 = r4 ^ v17;
        
        v1 = v1 + r4;
        v2 = v2 * r3;
        v3 = v3 - r2;
        v4 = v4 ^ r1;
        
        sum += r1 + r2 + r3 + r4 + v1 + v2 + v3 + v4;
    }
    
    return sum + v20;
}

/* ============================================================
 * Main function - calls all patterns
 * ============================================================ */
COLD int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use CPU feature check to engage target-specific optimizations */
    if (__builtin_cpu_supports("avx2")) {
        /* This may affect register allocation strategies */
        result += 1000;
    }
    
    /* Test different inputs to prevent constant folding */
    int test_inputs[] = {1, 2, 3, 5, 7, 11, 13, 17, 19, 23};
    
    for (int i = 0; i < 10; i++) {
        /* Call each pattern with different inputs */
        result += pattern_a_int_pressure(test_inputs[i]);
        result += (int)pattern_b_fp_pressure(test_inputs[i] * 1.5);
        result += pattern_c_cfg_complexity(test_inputs[i]);
        
        v4si vec_input = {test_inputs[i], test_inputs[i]+1, 
                         test_inputs[i]+2, test_inputs[i]+3};
        v4si vec_result = pattern_d_simd_pressure(vec_input);
        result += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
        
        result += pattern_e_register_conflict(test_inputs[i]);
    }
    
    /* Print result to prevent entire program elimination */
    printf("Final result: %d\n", result);
    
    return result % 256;
}
