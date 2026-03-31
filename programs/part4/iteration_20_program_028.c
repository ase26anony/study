/* test_mcf_coverage.c
 * 
 * This test program is designed to stress GCC's Min-Cost Flow (MCF) register
 * allocator to trigger coverage of special node indices in the print_node
 * function. The program creates multiple high register-pressure functions
 * with complex control flow to generate ENTRY_BLOCK, EXIT_BLOCK, 
 * new_exit_index, and new_entry_index nodes in the fixup graph.
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure each function is compiled independently */
#define NOINLINE __attribute__((noinline))
#define COLD __attribute__((cold))

/* Vector type for SIMD pressure */
typedef int v4si __attribute__((vector_size(16)));

/* Pattern A: Integer arithmetic chain with 30+ variables in a loop */
NOINLINE static long pattern_a(int input) {
    /* 30+ integer variables to create register pressure */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int a11, a12, a13, a14, a15, a16, a17, a18, a19, a20;
    int a21, a22, a23, a24, a25, a26, a27, a28, a29, a30;
    int a31, a32, a33, a34, a35;
    long sum = 0;
    
    /* Complex initialization chain */
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
    a31 = a30 - a21;
    a32 = a31 * a22;
    a33 = a32 / (a23 + 1);
    a34 = a33 ^ a24;
    a35 = a34 | a25;
    
    /* Interdependent loop to force spill decisions */
    for (int i = 0; i < 100; i++) {
        a1 = (a1 + a2) & 0xFF;
        a2 = (a2 + a3) & 0xFF;
        a3 = (a3 + a4) & 0xFF;
        a4 = (a4 + a5) & 0xFF;
        a5 = (a5 + a6) & 0xFF;
        a6 = (a6 + a7) & 0xFF;
        a7 = (a7 + a8) & 0xFF;
        a8 = (a8 + a9) & 0xFF;
        a9 = (a9 + a10) & 0xFF;
        a10 = (a10 + a11) & 0xFF;
        a11 = (a11 + a12) & 0xFF;
        a12 = (a12 + a13) & 0xFF;
        a13 = (a13 + a14) & 0xFF;
        a14 = (a14 + a15) & 0xFF;
        a15 = (a15 + a16) & 0xFF;
        a16 = (a16 + a17) & 0xFF;
        a17 = (a17 + a18) & 0xFF;
        a18 = (a18 + a19) & 0xFF;
        a19 = (a19 + a20) & 0xFF;
        a20 = (a20 + a21) & 0xFF;
        
        /* Prevent dead code elimination */
        asm volatile("" : "+r"(a1), "+r"(a2), "+r"(a3), "+r"(a4), "+r"(a5));
        asm volatile("" : "+r"(a6), "+r"(a7), "+r"(a8), "+r"(a9), "+r"(a10));
    }
    
    /* Sum all variables to ensure they're used */
    sum = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
          a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19 + a20 +
          a21 + a22 + a23 + a24 + a25 + a26 + a27 + a28 + a29 + a30 +
          a31 + a32 + a33 + a34 + a35;
    
    return sum;
}

/* Pattern B: Floating-point intensive computation */
NOINLINE static double pattern_b(double input) {
    /* 20+ double variables to pressure FP registers */
    double b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    double b11, b12, b13, b14, b15, b16, b17, b18, b19, b20;
    double b21, b22, b23, b24, b25;
    
    /* Complex FP computation chain */
    b1 = input + 1.0;
    b2 = b1 * 2.0;
    b3 = b2 - input;
    b4 = b3 + b1;
    b5 = b4 * b2;
    b6 = b5 / (input + 2.0);
    b7 = b6 * b3;
    b8 = b7 + b4;
    b9 = b8 * b5;
    b10 = b9 / b6;
    b11 = b10 + b7;
    b12 = b11 * b8;
    b13 = b12 / b9;
    b14 = b13 + b10;
    b15 = b14 * b11;
    b16 = b15 / b12;
    b17 = b16 + b13;
    b18 = b17 * b14;
    b19 = b18 / b15;
    b20 = b19 + b16;
    b21 = b20 * b17;
    b22 = b21 / b18;
    b23 = b22 + b19;
    b24 = b23 * b20;
    b25 = b24 / b21;
    
    /* Loop with FP operations */
    for (int i = 0; i < 50; i++) {
        b1 = b1 * 0.99 + b2;
        b2 = b2 * 0.98 + b3;
        b3 = b3 * 0.97 + b4;
        b4 = b4 * 0.96 + b5;
        b5 = b5 * 0.95 + b6;
        b6 = b6 * 0.94 + b7;
        b7 = b7 * 0.93 + b8;
        b8 = b8 * 0.92 + b9;
        b9 = b9 * 0.91 + b10;
        b10 = b10 * 0.90 + b11;
        
        /* Mix control flow to create complex CFG */
        if (i % 3 == 0) {
            b11 = b11 * 1.01;
        } else if (i % 3 == 1) {
            b12 = b12 * 1.02;
        } else {
            b13 = b13 * 1.03;
        }
        
        /* Prevent dead code elimination */
        asm volatile("" : "+x"(b1), "+x"(b2), "+x"(b3));
    }
    
    return b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9 + b10 +
           b11 + b12 + b13 + b14 + b15 + b16 + b17 + b18 + b19 + b20 +
           b21 + b22 + b23 + b24 + b25;
}

/* Pattern C: Complex control flow with switch statement */
NOINLINE static int pattern_c(int input) {
    int result = input;
    
    /* Many small basic blocks from switch */
    for (int i = 0; i < 100; i++) {
        switch (i % 23) {  /* 23 cases for complex CFG */
            case 0: result += i * 2; break;
            case 1: result -= i * 3; break;
            case 2: result ^= i; break;
            case 3: result |= i << 2; break;
            case 4: result &= ~i; break;
            case 5: result = result * 7 + i; break;
            case 6: result = result / (i + 1) + 5; break;
            case 7: result = (result << 3) | i; break;
            case 8: result = (result >> 2) ^ i; break;
            case 9: result = result + i * i; break;
            case 10: result = result - i / 2; break;
            case 11: result = result * 11 % 256; break;
            case 12: result = result & 0xAA + i; break;
            case 13: result = result | 0x55 - i; break;
            case 14: result = result ^ 0xFF; break;
            case 15: result = ~result + i; break;
            case 16: result = result * 3 / 2; break;
            case 17: result = result + (i << 4); break;
            case 18: result = result - (i >> 1); break;
            case 19: result = result * 13 % 1024; break;
            case 20: result = result & 0xCC; break;
            case 21: result = result | 0x33; break;
            case 22: result = result ^ 0x99; break;
        }
        
        /* Additional control flow with break/continue */
        if (i % 7 == 0) {
            continue;
        }
        if (i % 11 == 0) {
            result += 1000;
            break;
        }
        if (i % 13 == 0) {
            result -= 500;
            continue;
        }
    }
    
    return result;
}

/* Pattern D: SIMD vector operations */
NOINLINE static v4si pattern_d(v4si input) {
    v4si v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    v4si v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    
    /* Initialize vectors */
    v1 = input + (v4si){1, 2, 3, 4};
    v2 = v1 * (v4si){2, 3, 4, 5};
    v3 = v2 - input;
    v4 = v3 + v1;
    v5 = v4 * v2;
    v6 = v5 / ((v4si){2, 2, 2, 2});
    v7 = v6 + v3;
    v8 = v7 * v4;
    v9 = v8 - v5;
    v10 = v9 / ((v4si){3, 3, 3, 3});
    v11 = v10 + v6;
    v12 = v11 * v7;
    v13 = v12 - v8;
    v14 = v13 / ((v4si){4, 4, 4, 4});
    v15 = v14 + v9;
    v16 = v15 * v10;
    v17 = v16 - v11;
    v18 = v17 / ((v4si){5, 5, 5, 5});
    v19 = v18 + v12;
    v20 = v19 * v13;
    
    /* Loop with vector operations */
    for (int i = 0; i < 50; i++) {
        v1 = v1 + v2;
        v2 = v2 + v3;
        v3 = v3 + v4;
        v4 = v4 + v5;
        v5 = v5 + v6;
        v6 = v6 + v7;
        v7 = v7 + v8;
        v8 = v8 + v9;
        v9 = v9 + v10;
        v10 = v10 + v11;
        
        /* Conditional vector operations */
        if (i % 2 == 0) {
            v11 = v11 * ((v4si){2, 2, 2, 2});
        } else {
            v12 = v12 / ((v4si){2, 2, 2, 2});
        }
        
        /* Prevent dead code elimination */
        asm volatile("" : "+x"(v1), "+x"(v2));
    }
    
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
}

/* Pattern E: Explicit register variables to conflict with allocator */
NOINLINE static int pattern_e(int input) {
    /* Explicit register variables that may conflict with allocator */
    register int r1 asm ("r12") = input + 1;
    register int r2 asm ("r13") = r1 * 2;
    register int r3 asm ("r14") = r2 - input;
    register int r4 asm ("r15") = r3 + r1;
    
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int a11, a12, a13, a14, a15;
    
    /* Mix explicit and automatic variables */
    a1 = r4 * 3;
    a2 = a1 + r3;
    a3 = a2 - r2;
    a4 = a3 * r1;
    a5 = a4 / (input + 2);
    a6 = a5 ^ a1;
    a7 = a6 | a2;
    a8 = a7 & a3;
    a9 = a8 << 1;
    a10 = a9 >> 1;
    a11 = a10 + a4;
    a12 = a11 - a5;
    a13 = a12 * a6;
    a14 = a13 / (a7 + 1);
    a15 = a14 ^ a8;
    
    /* Complex loop with register pressure */
    for (int i = 0; i < 100; i++) {
        r1 = (r1 + r2) & 0xFF;
        r2 = (r2 + r3) & 0xFF;
        r3 = (r3 + r4) & 0xFF;
        r4 = (r4 + a1) & 0xFF;
        a1 = (a1 + a2) & 0xFF;
        a2 = (a2 + a3) & 0xFF;
        a3 = (a3 + a4) & 0xFF;
        a4 = (a4 + a5) & 0xFF;
        a5 = (a5 + a6) & 0xFF;
        
        /* Use computed goto for complex control flow (GCC extension) */
        void* labels[] = {&&L0, &&L1, &&L2, &&L3, &&L4, &&L5};
        goto *labels[i % 6];
        
        L0: a6 = a6 * 2; continue;
        L1: a7 = a7 / 2; continue;
        L2: a8 = a8 + i; continue;
        L3: a9 = a9 - i; continue;
        L4: a10 = a10 ^ i; continue;
        L5: a11 = a11 | i; continue;
    }
    
    return r1 + r2 + r3 + r4 + a1 + a2 + a3 + a4 + a5 + a6 +
           a7 + a8 + a9 + a10 + a11 + a12 + a13 + a14 + a15;
}

/* Helper to use CPU features - may affect register allocation */
static int use_cpu_features(void) {
#ifdef __GNUC__
    /* Check for various CPU features that affect register allocation */
    int has_avx2 = __builtin_cpu_supports("avx2");
    int has_avx = __builtin_cpu_supports("avx");
    int has_sse4 = __builtin_cpu_supports("sse4.2");
    
    /* Use the results to affect compilation */
    if (has_avx2) {
        return 1;
    } else if (has_avx) {
        return 2;
    } else if (has_sse4) {
        return 3;
    }
#endif
    return 0;
}

/* Main function marked cold to potentially affect block ordering */
COLD int main(int argc, char** argv) {
    int i;
    long total = 0;
    double fp_total = 0.0;
    
    /* Use CPU features to engage target-specific optimizations */
    int cpu_feat = use_cpu_features();
    
    /* Array of inputs to prevent constant folding */
    int inputs[] = {1, 2, 3, 5, 7, 11, 13, 17, 19, 23};
    double fp_inputs[] = {1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1};
    
    /* Call pattern_a multiple times with different inputs */
    for (i = 0; i < 10; i++) {
        total += pattern_a(inputs[i] + cpu_feat);
    }
    
    /* Call pattern_b with floating point inputs */
    for (i = 0; i < 10; i++) {
        fp_total += pattern_b(fp_inputs[i] + cpu_feat);
    }
    
    /* Call pattern_c with various inputs */
    for (i = 0; i < 10; i++) {
        total += pattern_c(inputs[i] * 3);
    }
    
    /* Call pattern_d with vector inputs */
    v4si vec_input = {1, 2, 3, 4};
    v4si vec_result = {0, 0, 0, 0};
    for (i = 0; i < 5; i++) {
        vec_input[0] = inputs[i];
        vec_input[1] = inputs[i] + 1;
        vec_input[2] = inputs[i] + 2;
        vec_input[3] = inputs[i] + 3;
        v4si temp = pattern_d(vec_input);
        vec_result += temp;
    }
    
    /* Call pattern_e with explicit register variables */
    for (i = 0; i < 10; i++) {
        total += pattern_e(inputs[i] * 7);
    }
    
    /* Mix results to ensure all computations contribute to output */
    total += (long)fp_total;
    total += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
    
    /* Print result to prevent complete optimization */
    printf("Result: %ld (CPU features level: %d)\n", total, cpu_feat);
    
    return (int)(total % 256);
}
