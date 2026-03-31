/* test_mcf_coverage.c
 * 
 * This test stresses GCC's Min-Cost Flow register allocator to trigger
 * the special node printing logic in mcf.cc's print_node function.
 * The goal is to cover lines that print ENTRY, EXIT, NEW_ENTRY, and NEW_EXIT
 * labels when n matches special indices in the fixup graph.
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure each function is compiled independently */
#define NOINLINE __attribute__((noinline))
#define COLD __attribute__((cold))

/* Vector type for SIMD pressure */
typedef int v4si __attribute__((vector_size(16)));

/* ============================================
 * PATTERN A: Integer arithmetic chain with 30+ variables
 * Creates massive register pressure in a loop
 * ============================================ */
NOINLINE static long pattern_a_intensive(int input) {
    /* Declare 30+ integer variables to pressure integer registers */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int a11, a12, a13, a14, a15, a16, a17, a18, a19, a20;
    int a21, a22, a23, a24, a25, a26, a27, a28, a29, a30;
    int a31, a32, a33, a34, a35;
    
    long sum = 0;
    
    /* Complex loop with interdependent calculations */
    for (int i = 0; i < 100; i++) {
        /* Chain of dependent operations - forces sequential allocation */
        a1 = input + i;
        a2 = a1 * 3;
        a3 = a2 - a1;
        a4 = a3 / 2;
        a5 = a4 + a2;
        a6 = a5 ^ a3;
        a7 = a6 | a4;
        a8 = a7 & a5;
        a9 = a8 << 2;
        a10 = a9 >> 1;
        
        a11 = a10 + a1;
        a12 = a11 * a2;
        a13 = a12 - a3;
        a14 = a13 / a4;
        a15 = a14 + a5;
        a16 = a15 ^ a6;
        a17 = a16 | a7;
        a18 = a17 & a8;
        a19 = a18 << a9;
        a20 = a19 >> a10;
        
        a21 = a20 + a11;
        a22 = a21 * a12;
        a23 = a22 - a13;
        a24 = a23 / a14;
        a25 = a24 + a15;
        a26 = a25 ^ a16;
        a27 = a26 | a17;
        a28 = a27 & a18;
        a29 = a28 << 3;
        a30 = a29 >> 2;
        
        a31 = a30 + a21;
        a32 = a31 * a22;
        a33 = a32 - a23;
        a34 = a33 / a24;
        a35 = a34 + a25;
        
        /* Mix all results to prevent dead code elimination */
        sum += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
               a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19 + a20 +
               a21 + a22 + a23 + a24 + a25 + a26 + a27 + a28 + a29 + a30 +
               a31 + a32 + a33 + a34 + a35;
        
        /* Complex control flow with break/continue */
        if (i % 7 == 0) continue;
        if (i > 75 && sum > 1000000) break;
        if (i == 50) {
            /* Nested loop to create more basic blocks */
            for (int j = 0; j < 5; j++) {
                sum += j * input;
            }
        }
    }
    
    /* Use asm to prevent optimization */
    asm volatile ("" : : "r"(sum));
    return sum;
}

/* ============================================
 * PATTERN B: Floating-point intensive computation
 * Pressures floating-point registers
 * ============================================ */
NOINLINE static double pattern_b_fp_intensive(double input) {
    /* Many double variables */
    double b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    double b11, b12, b13, b14, b15, b16, b17, b18, b19, b20;
    double b21, b22, b23, b24, b25;
    
    double result = input;
    
    /* FP-intensive loop */
    for (int i = 0; i < 50; i++) {
        b1 = result * 1.1;
        b2 = b1 + 2.3;
        b3 = b2 * 0.7;
        b4 = b3 / 1.5;
        b5 = b4 - 0.3;
        b6 = b5 * b1;
        b7 = b6 / b2;
        b8 = b7 + b3;
        b9 = b8 - b4;
        b10 = b9 * b5;
        
        b11 = b10 + 3.14159;
        b12 = b11 * 2.71828;
        b13 = b12 / 1.41421;
        b14 = b13 - b6;
        b15 = b14 * b7;
        b16 = b15 / b8;
        b17 = b16 + b9;
        b18 = b17 - b10;
        b19 = b18 * b11;
        b20 = b19 / b12;
        
        b21 = b20 + b13;
        b22 = b21 * b14;
        b23 = b22 / b15;
        b24 = b23 - b16;
        b25 = b24 * b17;
        
        result = b25;
        
        /* Mix integer and FP operations */
        if (i % 11 == 0) {
            result += (double)i * 0.01;
        }
    }
    
    asm volatile ("" : : "r"(result));
    return result;
}

/* ============================================
 * PATTERN C: Complex control flow with switch
 * Creates many basic blocks for CFG complexity
 * ============================================ */
NOINLINE static int pattern_c_cfg_complex(int input) {
    int result = input;
    
    /* Switch with many cases creates many basic blocks */
    for (int i = 0; i < 100; i++) {
        switch (i % 23) {
            case 0: result += i * 2; break;
            case 1: result -= i * 3; break;
            case 2: result ^= i; break;
            case 3: result |= 0xFF; break;
            case 4: result &= 0xF0; break;
            case 5: result <<= 2; break;
            case 6: result >>= 1; break;
            case 7: result = result * 7 + 1; break;
            case 8: result = result / 3; break;
            case 9: result = -result; break;
            case 10: result = ~result; break;
            case 11: result = result % 17; break;
            case 12: result = result ^ 0xAAAA; break;
            case 13: result = result | 0x5555; break;
            case 14: result = result & 0x3333; break;
            case 15: result = result << (i % 4); break;
            case 16: result = result >> (i % 4); break;
            case 17: result = result + (i * i); break;
            case 18: result = result - (i << 2); break;
            case 19: result = result * 3 / 2; break;
            case 20: result = result ^ result; break;
            case 21: result = result | (result << 8); break;
            case 22: result = result & (result >> 4); break;
        }
        
        /* Nested control flow */
        if (i % 7 == 0) {
            for (int j = 0; j < 3; j++) {
                result += j;
                if (j == 1) continue;
                result -= 1;
            }
        } else if (i % 13 == 0) {
            int k = 0;
            while (k < 5) {
                result ^= k;
                k++;
                if (k == 3) break;
            }
        }
    }
    
    asm volatile ("" : : "r"(result));
    return result;
}

/* ============================================
 * PATTERN D: SIMD vector operations
 * Pressures vector/SIMD registers
 * ============================================ */
NOINLINE static v4si pattern_d_simd_pressure(v4si input) {
    v4si v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    v4si result = input;
    
    v4si mask1 = {0xFF, 0xFF00, 0xFF0000, 0xFF000000};
    v4si mask2 = {0xAA, 0xAA00, 0xAA0000, 0xAA000000};
    v4si ones = {1, 1, 1, 1};
    v4si twos = {2, 2, 2, 2};
    
    for (int i = 0; i < 40; i++) {
        v1 = result + ones;
        v2 = v1 * twos;
        v3 = v2 & mask1;
        v4 = v3 | mask2;
        v5 = v4 << 1;
        v6 = v5 >> 2;
        v7 = v6 + v1;
        v8 = v7 * v2;
        v9 = v8 & v3;
        v10 = v9 | v4;
        
        result = v10;
        
        /* Conditional vector operations */
        if (i % 5 == 0) {
            result = result + (v4si){i, i, i, i};
        }
    }
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(result));
    return result;
}

/* ============================================
 * PATTERN E: Explicit register variables
 * Conflicts with allocator's choices
 * ============================================ */
NOINLINE static int pattern_e_register_conflict(int input) {
    /* Explicit register variables that may conflict */
    register int r1 asm ("r12") = input;
    register int r2 asm ("r13") = input * 2;
    register int r3 asm ("r14") = input + 1;
    register int r4 asm ("r15") = input - 1;
    
    int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    
    /* Force use of explicit registers with many operations */
    for (int i = 0; i < 50; i++) {
        temp1 = r1 + r2;
        temp2 = r3 * r4;
        temp3 = temp1 ^ temp2;
        temp4 = r1 | r3;
        temp5 = r2 & r4;
        temp6 = temp3 + temp4;
        temp7 = temp5 * temp6;
        temp8 = temp7 - temp1;
        
        /* Rotate register values */
        r1 = r2;
        r2 = r3;
        r3 = r4;
        r4 = temp8;
        
        /* Complex loop with computed goto (GCC extension) */
        if (i % 13 == 0) {
            static void *labels[] = {&&L0, &&L1, &&L2, &&L3, &&L4};
            goto *labels[i % 5];
            
            L0: r1 += 1; goto END;
            L1: r2 -= 1; goto END;
            L2: r3 *= 2; goto END;
            L3: r4 /= 2; goto END;
            L4: r1 ^= r2; goto END;
            END:;
        }
    }
    
    int result = r1 + r2 + r3 + r4;
    asm volatile ("" : : "r"(result));
    return result;
}

/* ============================================
 * Helper to use CPU features (triggers target-specific RA)
 * ============================================ */
static int check_cpu_features(void) {
    int has_avx2 = 0;
    int has_sse4 = 0;
    
#ifdef __GNUC__
    /* These builtins engage target-specific register allocation */
    if (__builtin_cpu_supports("avx2")) has_avx2 = 1;
    if (__builtin_cpu_supports("sse4.2")) has_sse4 = 1;
#else
    /* Stubs for non-GCC compilers */
    has_avx2 = 0;
    has_sse4 = 0;
#endif
    
    return has_avx2 | has_sse4;
}

/* ============================================
 * Main function - calls all patterns
 * ============================================ */
COLD int main(int argc, char **argv) {
    int cpu_features = check_cpu_features();
    
    /* Array of inputs to prevent constant folding */
    int int_inputs[] = {1, 2, 3, 5, 7, 11, 13, 17, 19, 23};
    double fp_inputs[] = {1.0, 2.5, 3.14159, 2.71828, 1.41421};
    
    long total = 0;
    
    /* Call pattern A multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        total += pattern_a_intensive(int_inputs[i % 10]);
    }
    
    /* Call pattern B */
    double fp_result = 0.0;
    for (int i = 0; i < 5; i++) {
        fp_result += pattern_b_fp_intensive(fp_inputs[i % 5]);
    }
    
    /* Call pattern C */
    int cfg_result = 0;
    for (int i = 0; i < 10; i++) {
        cfg_result += pattern_c_cfg_complex(int_inputs[i % 10]);
    }
    
    /* Call pattern D */
    v4si vec_input = {1, 2, 3, 4};
    v4si vec_result = pattern_d_simd_pressure(vec_input);
    
    /* Extract vector result */
    int vec_sum = 0;
    for (int i = 0; i < 4; i++) {
        vec_sum += vec_result[i];
    }
    
    /* Call pattern E */
    int reg_result = 0;
    for (int i = 0; i < 10; i++) {
        reg_result += pattern_e_register_conflict(int_inputs[i % 10]);
    }
    
    /* Combine all results to prevent dead code elimination */
    long final_result = total + (long)fp_result + cfg_result + vec_sum + reg_result;
    
    /* Use result to prevent optimization */
    asm volatile ("" : : "r"(final_result));
    
    /* Optional debug output */
    if (argc > 1) {
        printf("Result: %ld (CPU features: %d)\n", final_result, cpu_features);
    }
    
    return (final_result > 0) ? 0 : 1;
}
