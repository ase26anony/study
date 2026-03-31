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
#define NORETURN __attribute__((noreturn))
#define COLD __attribute__((cold))

/* Vector type for SIMD pressure */
typedef int v4si __attribute__((vector_size(16)));

/* ============================================
 * PATTERN A: Integer arithmetic chain with 30+ variables
 * Creates massive register pressure in a loop
 * ============================================ */
NOINLINE static int pattern_a_int_pressure(int input) {
    /* Declare 30+ integer variables to pressure integer registers */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int a11, a12, a13, a14, a15, a16, a17, a18, a19, a20;
    int a21, a22, a23, a24, a25, a26, a27, a28, a29, a30;
    int a31, a32, a33, a34, a35, a36, a37, a38, a39, a40;
    
    /* Initialize with input to prevent constant folding */
    a1 = input + 1;
    a2 = a1 * 2 + input;
    a3 = a2 - a1 + input;
    a4 = a3 * a2 + input;
    a5 = a4 / (input + 1) + 1;
    a6 = a5 ^ a4 + input;
    a7 = a6 | a5 + input;
    a8 = a7 & a6 + input;
    a9 = a8 << 1 + input;
    a10 = a9 >> 1 + input;
    
    a11 = a10 + a9 + input;
    a12 = a11 * a10 + input;
    a13 = a12 - a11 + input;
    a14 = a13 * a12 + input;
    a15 = a14 / (input + 2) + 1;
    a16 = a15 ^ a14 + input;
    a17 = a16 | a15 + input;
    a18 = a17 & a16 + input;
    a19 = a18 << 2 + input;
    a20 = a19 >> 2 + input;
    
    a21 = a20 + a19 + input;
    a22 = a21 * a20 + input;
    a23 = a22 - a21 + input;
    a24 = a23 * a22 + input;
    a25 = a24 / (input + 3) + 1;
    a26 = a25 ^ a24 + input;
    a27 = a26 | a25 + input;
    a28 = a27 & a26 + input;
    a29 = a28 << 3 + input;
    a30 = a29 >> 3 + input;
    
    a31 = a30 + a29 + input;
    a32 = a31 * a30 + input;
    a33 = a32 - a31 + input;
    a34 = a33 * a32 + input;
    a35 = a34 / (input + 4) + 1;
    a36 = a35 ^ a34 + input;
    a37 = a36 | a35 + input;
    a38 = a37 & a36 + input;
    a39 = a38 << 4 + input;
    a40 = a39 >> 4 + input;
    
    /* Complex loop with all variables to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        /* Use asm volatile to prevent optimization */
        asm volatile ("" : "+r" (a1), "+r" (a2), "+r" (a3), "+r" (a4), "+r" (a5));
        asm volatile ("" : "+r" (a6), "+r" (a7), "+r" (a8), "+r" (a9), "+r" (a10));
        
        /* Interdependent calculations */
        a1 = a2 + a3;
        a2 = a3 + a4;
        a3 = a4 + a5;
        a4 = a5 + a6;
        a5 = a6 + a7;
        a6 = a7 + a8;
        a7 = a8 + a9;
        a8 = a9 + a10;
        a9 = a10 + a1;
        a10 = a1 + a2;
        
        /* More complex interactions */
        a11 = a12 * a13 + i;
        a12 = a13 * a14 + i;
        a13 = a14 * a15 + i;
        a14 = a15 * a16 + i;
        a15 = a16 * a17 + i;
        a16 = a17 * a18 + i;
        a17 = a18 * a19 + i;
        a18 = a19 * a20 + i;
        a19 = a20 * a21 + i;
        a20 = a21 * a22 + i;
        
        a21 = a22 ^ a23 ^ i;
        a22 = a23 ^ a24 ^ i;
        a23 = a24 ^ a25 ^ i;
        a24 = a25 ^ a26 ^ i;
        a25 = a26 ^ a27 ^ i;
        a26 = a27 ^ a28 ^ i;
        a27 = a28 ^ a29 ^ i;
        a28 = a29 ^ a30 ^ i;
        a29 = a30 ^ a31 ^ i;
        a30 = a31 ^ a32 ^ i;
        
        a31 = a32 | a33 | i;
        a32 = a33 | a34 | i;
        a33 = a34 | a35 | i;
        a34 = a35 | a36 | i;
        a35 = a36 | a37 | i;
        a36 = a37 | a38 | i;
        a37 = a38 | a39 | i;
        a38 = a39 | a40 | i;
        a39 = a40 | a1 | i;
        a40 = a1 | a2 | i;
        
        sum += a1 + a10 + a20 + a30 + a40;
    }
    
    return sum;
}

/* ============================================
 * PATTERN B: Floating-point intensive computation
 * Pressures floating-point registers
 * ============================================ */
NOINLINE static double pattern_b_float_pressure(double input) {
    /* 20+ double variables */
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    double d11, d12, d13, d14, d15, d16, d17, d18, d19, d20;
    double d21, d22, d23, d24, d25, d26, d27, d28, d29, d30;
    
    d1 = input + 1.0;
    d2 = d1 * 1.1;
    d3 = d2 / 1.2;
    d4 = d3 - 0.5;
    d5 = d4 * d3;
    d6 = d5 + d4;
    d7 = d6 - d5;
    d8 = d7 * d6;
    d9 = d8 / d7;
    d10 = d9 + d8;
    
    d11 = d10 * 1.3;
    d12 = d11 / 1.4;
    d13 = d12 - 0.6;
    d14 = d13 * d12;
    d15 = d14 + d13;
    d16 = d15 - d14;
    d17 = d16 * d15;
    d18 = d17 / d16;
    d19 = d18 + d17;
    d20 = d19 * 1.5;
    
    d21 = d20 / 1.6;
    d22 = d21 - 0.7;
    d23 = d22 * d21;
    d24 = d23 + d22;
    d25 = d24 - d23;
    d26 = d25 * d24;
    d27 = d26 / d25;
    d28 = d27 + d26;
    d29 = d28 * 1.7;
    d30 = d29 / 1.8;
    
    /* Complex floating-point loop */
    double result = 0.0;
    for (int i = 0; i < 50; i++) {
        double t = (double)i;
        
        /* Trigonometric calculations to use FPU heavily */
        d1 = d2 * d3 + t;
        d2 = d3 * d4 - t;
        d3 = d4 * d5 + t * 0.1;
        d4 = d5 * d6 - t * 0.2;
        d5 = d6 * d7 + t * 0.3;
        d6 = d7 * d8 - t * 0.4;
        d7 = d8 * d9 + t * 0.5;
        d8 = d9 * d10 - t * 0.6;
        d9 = d10 * d11 + t * 0.7;
        d10 = d11 * d12 - t * 0.8;
        
        d11 = d12 * d13 + t * 0.9;
        d12 = d13 * d14 - t * 1.0;
        d13 = d14 * d15 + t * 1.1;
        d14 = d15 * d16 - t * 1.2;
        d15 = d16 * d17 + t * 1.3;
        d16 = d17 * d18 - t * 1.4;
        d17 = d18 * d19 + t * 1.5;
        d18 = d19 * d20 - t * 1.6;
        d19 = d20 * d21 + t * 1.7;
        d20 = d21 * d22 - t * 1.8;
        
        d21 = d22 * d23 + t * 1.9;
        d22 = d23 * d24 - t * 2.0;
        d23 = d24 * d25 + t * 2.1;
        d24 = d25 * d26 - t * 2.2;
        d25 = d26 * d27 + t * 2.3;
        d26 = d27 * d28 - t * 2.4;
        d27 = d28 * d29 + t * 2.5;
        d28 = d29 * d30 - t * 2.6;
        d29 = d30 * d1 + t * 2.7;
        d30 = d1 * d2 - t * 2.8;
        
        /* Prevent dead code elimination */
        asm volatile ("" : "+x" (d1), "+x" (d2), "+x" (d3), "+x" (d4), "+x" (d5));
        
        result += d1 + d10 + d20 + d30;
    }
    
    return result;
}

/* ============================================
 * PATTERN C: Complex control flow with switch statement
 * Creates many basic blocks for CFG complexity
 * ============================================ */
NOINLINE static int pattern_c_complex_cfg(int input) {
    int result = input;
    
    /* Nested loops with breaks/continues */
    for (int i = 0; i < 100; i++) {
        if (i % 3 == 0) {
            for (int j = 0; j < 20; j++) {
                if (j == 10) break;
                if (j % 2 == 0) continue;
                result += j;
            }
        } else if (i % 3 == 1) {
            int k = 0;
            while (k < 15) {
                if (k == 7) break;
                result -= k;
                k++;
            }
        } else {
            int m = 0;
            do {
                if (m == 5) continue;
                result *= (m + 1);
                m++;
            } while (m < 10);
        }
    }
    
    /* Large switch statement with 20+ cases */
    switch (input % 23) {
        case 0: result += 1; break;
        case 1: result -= 2; break;
        case 2: result *= 3; break;
        case 3: result /= 4; break;
        case 4: result ^= 5; break;
        case 5: result |= 6; break;
        case 6: result &= 7; break;
        case 7: result <<= 1; break;
        case 8: result >>= 2; break;
        case 9: result = ~result; break;
        case 10: result += result * 2; break;
        case 11: result -= result / 2; break;
        case 12: result = result * result; break;
        case 13: result = result / (input + 1); break;
        case 14: result = result ^ input; break;
        case 15: result = result | (input << 1); break;
        case 16: result = result & (input | 0xFF); break;
        case 17: result = result << (input % 4); break;
        case 18: result = result >> (input % 4); break;
        case 19: result = -result; break;
        case 20: result = result + 100; break;
        case 21: result = result - 50; break;
        case 22: result = result * 3 / 2; break;
        default: result = 0; break;
    }
    
    /* Computed goto (GCC extension) for extra CFG complexity */
    void* labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4, &&L5 };
    int idx = result % 6;
    goto *labels[idx];
    
L0: result += 1000; goto END;
L1: result += 2000; goto END;
L2: result += 3000; goto END;
L3: result += 4000; goto END;
L4: result += 5000; goto END;
L5: result += 6000; goto END;
END:
    
    return result;
}

/* ============================================
 * PATTERN D: SIMD vector operations
 * Pressures vector/SIMD registers
 * ============================================ */
NOINLINE static v4si pattern_d_simd_pressure(v4si input) {
    v4si v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    v4si v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    
    v1 = input + (v4si){1, 2, 3, 4};
    v2 = v1 * (v4si){2, 3, 4, 5};
    v3 = v2 - (v4si){1, 1, 1, 1};
    v4 = v3 * v2;
    v5 = v4 + v3;
    v6 = v5 - v4;
    v7 = v6 * v5;
    v8 = v7 + v6;
    v9 = v8 - v7;
    v10 = v9 * v8;
    
    v11 = v10 + (v4si){10, 20, 30, 40};
    v12 = v11 * (v4si){3, 4, 5, 6};
    v13 = v12 - (v4si){2, 2, 2, 2};
    v14 = v13 * v12;
    v15 = v14 + v13;
    v16 = v15 - v14;
    v17 = v16 * v15;
    v18 = v17 + v16;
    v19 = v18 - v17;
    v20 = v19 * v18;
    
    /* Vector loop with many live variables */
    v4si accum = {0, 0, 0, 0};
    for (int i = 0; i < 40; i++) {
        v4si idx = {i, i+1, i+2, i+3};
        
        v1 = v2 + v3 + idx;
        v2 = v3 + v4 + idx;
        v3 = v4 + v5 + idx;
        v4 = v5 + v6 + idx;
        v5 = v6 + v7 + idx;
        v6 = v7 + v8 + idx;
        v7 = v8 + v9 + idx;
        v8 = v9 + v10 + idx;
        v9 = v10 + v11 + idx;
        v10 = v11 + v12 + idx;
        
        v11 = v12 * v13 + idx;
        v12 = v13 * v14 + idx;
        v13 = v14 * v15 + idx;
        v14 = v15 * v16 + idx;
        v15 = v16 * v17 + idx;
        v16 = v17 * v18 + idx;
        v17 = v18 * v19 + idx;
        v18 = v19 * v20 + idx;
        v19 = v20 * v1 + idx;
        v20 = v1 * v2 + idx;
        
        /* Prevent optimization */
        asm volatile ("" : "+x" (v1), "+x" (v2), "+x" (v3), "+x" (v4));
        
        accum += v1 + v10 + v20;
    }
    
    return accum;
}

/* ============================================
 * PATTERN E: Explicit register variables
 * Conflicts with allocator's choices
 * ============================================ */
NOINLINE static int pattern_e_explicit_registers(int input) {
    /* Try to use specific registers that might conflict */
    register int r0 asm ("r12") = input + 1;
    register int r1 asm ("r13") = r0 * 2;
    register int r2 asm ("r14") = r1 + 3;
    register int r3 asm ("r15") = r2 - 4;
    
    /* Force these to be live across calls/computations */
    int arr[4];
    for (int i = 0; i < 4; i++) {
        arr[i] = i * 10;
        /* Use explicit registers in asm to force conflicts */
        asm volatile ("# Force register use %0 %1 %2 %3" 
                     : "+r" (r0), "+r" (r1), "+r" (r2), "+r" (r3));
        r0 = r0 + arr[i];
        r1 = r1 * (arr[i] + 1);
        r2 = r2 - arr[i];
        r3 = r3 ^ arr[i];
    }
    
    /* Complex expression using all explicit registers */
    int result = 0;
    for (int i = 0; i < 50; i++) {
        r0 = r1 + r2;
        r1 = r2 + r3;
        r2 = r3 + r0;
        r3 = r0 + r1;
        
        /* Force spilling by using many temporaries */
        int t1 = r0 * i;
        int t2 = r1 * (i + 1);
        int t3 = r2 * (i + 2);
        int t4 = r3 * (i + 3);
        int t5 = t1 + t2;
        int t6 = t3 + t4;
        int t7 = t5 * t6;
        int t8 = t7 / (i + 1);
        int t9 = t8 ^ t7;
        int t10 = t9 | t8;
        
        result += t10;
        
        /* Keep explicit registers live */
        asm volatile ("" : "+r" (r0), "+r" (r1), "+r" (r2), "+r" (r3));
    }
    
    return result + r0 + r1 + r2 + r3;
}

/* ============================================
 * Main function - calls all patterns
 * ============================================ */
COLD int main(int argc, char **argv) {
    int total = 0;
    
    /* Use CPU feature check to engage target-specific optimizations */
#ifdef __GNUC__
    if (__builtin_cpu_supports("avx2")) {
        /* This may affect register allocation strategies */
        total += 1;
    }
#endif
    
    /* Call each pattern multiple times with different inputs */
    int inputs[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    for (int i = 0; i < 10; i++) {
        total += pattern_a_int_pressure(inputs[i]);
        total += (int)pattern_b_float_pressure((double)inputs[i]);
        total += pattern_c_complex_cfg(inputs[i]);
        
        v4si vec_input = {inputs[i], inputs[i]+1, inputs[i]+2, inputs[i]+3};
        v4si vec_result = pattern_d_simd_pressure(vec_input);
        total += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
        
        total += pattern_e_explicit_registers(inputs[i]);
    }
    
    /* Prevent dead code elimination of total */
    asm volatile ("" : "+r" (total));
    
    /* Simple output to prevent optimization of entire program */
    if (argc > 1) {
        printf("Result: %d\n", total);
    }
    
    return total % 256;
}
