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
    
    /* Complex initialization chain */
    a1 = input + 1;
    a2 = a1 * 2;
    a3 = a2 - a1;
    a4 = a3 + input;
    a5 = a4 * a3;
    a6 = a5 / (a1 ? a1 : 1);
    a7 = a6 ^ a5;
    a8 = a7 | a6;
    a9 = a8 & a7;
    a10 = a9 << 2;
    
    a11 = a10 >> 1;
    a12 = a11 + a10;
    a13 = a12 - a11;
    a14 = a13 * a12;
    a15 = a14 % (a13 ? a13 : 1);
    a16 = a15 ^ a14;
    a17 = a16 | a15;
    a18 = a17 & a16;
    a19 = a18 << 3;
    a20 = a19 >> 2;
    
    a21 = a20 + a19;
    a22 = a21 - a20;
    a23 = a22 * a21;
    a24 = a23 / (a22 ? a22 : 1);
    a25 = a24 ^ a23;
    a26 = a25 | a24;
    a27 = a26 & a25;
    a28 = a27 << 1;
    a29 = a28 >> 1;
    a30 = a29 + a28;
    
    a31 = a30 - a29;
    a32 = a31 * a30;
    a33 = a32 % (a31 ? a31 : 1);
    a34 = a33 ^ a32;
    a35 = a34 | a33;
    a36 = a35 & a34;
    a37 = a36 << 2;
    a38 = a37 >> 2;
    a39 = a38 + a37;
    a40 = a39 - a38;
    
    /* Loop with all variables live to create spill pressure */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        /* Force all variables to be used in computation */
        a1 = a2 + i;
        a2 = a3 - i;
        a3 = a4 * a1;
        a4 = a5 ^ a2;
        a5 = a6 | a3;
        a6 = a7 & a4;
        a7 = a8 << (i & 3);
        a8 = a9 >> (i & 3);
        a9 = a10 + a5;
        a10 = a11 - a6;
        
        a11 = a12 * a7;
        a12 = a13 ^ a8;
        a13 = a14 | a9;
        a14 = a15 & a10;
        a15 = a16 << (i & 3);
        a16 = a17 >> (i & 3);
        a17 = a18 + a11;
        a18 = a19 - a12;
        a19 = a20 * a13;
        a20 = a21 ^ a14;
        
        a21 = a22 | a15;
        a22 = a23 & a16;
        a23 = a24 << (i & 3);
        a24 = a25 >> (i & 3);
        a25 = a26 + a17;
        a26 = a27 - a18;
        a27 = a28 * a19;
        a28 = a29 ^ a20;
        a29 = a30 | a21;
        a30 = a31 & a22;
        
        a31 = a32 << (i & 3);
        a32 = a33 >> (i & 3);
        a33 = a34 + a23;
        a34 = a35 - a24;
        a35 = a36 * a25;
        a36 = a37 ^ a26;
        a37 = a38 | a27;
        a38 = a39 & a28;
        a39 = a40 << (i & 3);
        a40 = a1 >> (i & 3);
        
        sum += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
               a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19 + a20 +
               a21 + a22 + a23 + a24 + a25 + a26 + a27 + a28 + a29 + a30 +
               a31 + a32 + a33 + a34 + a35 + a36 + a37 + a38 + a39 + a40;
    }
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(sum), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5));
    
    return sum;
}

/* ============================================
 * PATTERN B: Floating-point intensive computation
 * Pressures floating-point and vector registers
 * ============================================ */
NOINLINE static double pattern_b_fp_pressure(double input) {
    /* 20+ double variables */
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    double d11, d12, d13, d14, d15, d16, d17, d18, d19, d20;
    double d21, d22, d23, d24, d25;
    
    d1 = input + 1.0;
    d2 = d1 * 2.0;
    d3 = d2 - d1;
    d4 = d3 + input;
    d5 = d4 * d3;
    d6 = d5 / (d1 != 0.0 ? d1 : 1.0);
    d7 = d6 * d5;
    d8 = d7 - d6;
    d9 = d8 + d7;
    d10 = d9 * d8;
    
    d11 = d10 / (d9 != 0.0 ? d9 : 1.0);
    d12 = d11 * d10;
    d13 = d12 - d11;
    d14 = d13 + d12;
    d15 = d14 * d13;
    d16 = d15 / (d14 != 0.0 ? d14 : 1.0);
    d17 = d16 * d15;
    d18 = d17 - d16;
    d19 = d18 + d17;
    d20 = d19 * d18;
    
    d21 = d20 / (d19 != 0.0 ? d19 : 1.0);
    d22 = d21 * d20;
    d23 = d22 - d21;
    d24 = d23 + d22;
    d25 = d24 * d23;
    
    /* Complex loop with trigonometric operations */
    double sum = 0.0;
    for (int i = 0; i < 50; i++) {
        double t = i * 0.1;
        
        /* Mix all variables with transcendental functions */
        d1 = d2 + sin(t);
        d2 = d3 - cos(t);
        d3 = d4 * sin(t * 0.5);
        d4 = d5 + cos(t * 0.5);
        d5 = d6 * tan(t);
        d6 = d7 - sin(t * 2.0);
        d7 = d8 + cos(t * 2.0);
        d8 = d9 * exp(-t);
        d9 = d10 - log(1.0 + t);
        d10 = d11 + sin(d1);
        
        d11 = d12 - cos(d2);
        d12 = d13 * sin(d3);
        d13 = d14 + cos(d4);
        d14 = d15 * tan(d5);
        d15 = d16 - sin(d6);
        d16 = d17 + cos(d7);
        d17 = d18 * exp(-d8);
        d18 = d19 - log(1.0 + d9);
        d19 = d20 + sin(d10);
        d20 = d21 - cos(d11);
        
        d21 = d22 * sin(d12);
        d22 = d23 + cos(d13);
        d23 = d24 * tan(d14);
        d24 = d25 - sin(d15);
        d25 = d1 + cos(d16);
        
        sum += d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10 +
               d11 + d12 + d13 + d14 + d15 + d16 + d17 + d18 + d19 + d20 +
               d21 + d22 + d23 + d24 + d25;
    }
    
    /* Prevent optimization */
    asm volatile("" : : "r"(sum), "r"(d1), "r"(d2), "r"(d3));
    
    return sum;
}

/* ============================================
 * PATTERN C: Complex control flow with switch statement
 * Creates many basic blocks for CFG complexity
 * ============================================ */
NOINLINE static int pattern_c_cfg_complexity(int input) {
    int result = input;
    
    /* Nested loops with breaks/continues */
    for (int i = 0; i < 10; i++) {
        if (i % 3 == 0) continue;
        
        for (int j = 0; j < 10; j++) {
            if (j % 4 == 0) break;
            
            /* Switch with many cases - creates many basic blocks */
            switch ((i + j) % 20) {
                case 0:  result += 1; break;
                case 1:  result -= 2; break;
                case 2:  result *= 3; break;
                case 3:  result /= 4; break;
                case 4:  result ^= 5; break;
                case 5:  result |= 6; break;
                case 6:  result &= 7; break;
                case 7:  result <<= 1; break;
                case 8:  result >>= 2; break;
                case 9:  result = ~result; break;
                case 10: result += j * 2; break;
                case 11: result -= i * 3; break;
                case 12: result *= (i + j); break;
                case 13: result /= (j ? j : 1); break;
                case 14: result ^= i ^ j; break;
                case 15: result |= (i | j); break;
                case 16: result &= (i & j); break;
                case 17: result <<= (i & 3); break;
                case 18: result >>= (j & 3); break;
                case 19: result = -result; break;
                default: result += 100; break;
            }
            
            /* Additional control flow */
            if (result > 1000) {
                goto early_exit;
            }
        }
        
        /* Another control flow twist */
        while (result < 0) {
            result += 10;
            if (result > 50) {
                break;
            }
        }
    }
    
early_exit:
    
    /* Computed goto (GCC extension) for extra CFG complexity */
    void* labels[] = { &&label0, &&label1, &&label2, &&label3, &&label4 };
    int idx = result % 5;
    goto *labels[idx];
    
label0:
    result += 10;
    goto done;
label1:
    result += 20;
    goto done;
label2:
    result += 30;
    goto done;
label3:
    result += 40;
    goto done;
label4:
    result += 50;
    goto done;
    
done:
    return result;
}

/* ============================================
 * PATTERN D: SIMD vector operations
 * Pressures vector/SIMD registers
 * ============================================ */
NOINLINE static v4si pattern_d_simd_pressure(v4si input) {
    v4si v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    v4si v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    
    /* Initialize vectors */
    v1 = input + (v4si){1, 2, 3, 4};
    v2 = v1 * (v4si){2, 3, 4, 5};
    v3 = v2 - v1;
    v4 = v3 + input;
    v5 = v4 * v3;
    v6 = v5 + v4;
    v7 = v6 - v5;
    v8 = v7 * v6;
    v9 = v8 + v7;
    v10 = v9 - v8;
    
    v11 = v10 * v9;
    v12 = v11 + v10;
    v13 = v12 - v11;
    v14 = v13 * v12;
    v15 = v14 + v13;
    v16 = v15 - v14;
    v17 = v16 * v15;
    v18 = v17 + v16;
    v19 = v18 - v17;
    v20 = v19 * v18;
    
    /* Loop with vector operations */
    v4si sum = {0, 0, 0, 0};
    for (int i = 0; i < 20; i++) {
        v4si idx = {i, i+1, i+2, i+3};
        
        /* Chain of vector operations */
        v1 = v2 + idx;
        v2 = v3 - idx;
        v3 = v4 * v1;
        v4 = v5 + v2;
        v5 = v6 - v3;
        v6 = v7 * v4;
        v7 = v8 + v5;
        v8 = v9 - v6;
        v9 = v10 * v7;
        v10 = v11 + v8;
        
        v11 = v12 - v9;
        v12 = v13 * v10;
        v13 = v14 + v11;
        v14 = v15 - v12;
        v15 = v16 * v13;
        v16 = v17 + v14;
        v17 = v18 - v15;
        v18 = v19 * v16;
        v19 = v20 + v17;
        v20 = v1 - v18;
        
        sum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
               v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    }
    
    return sum;
}

/* ============================================
 * PATTERN E: Explicit register variables
 * Conflicts with allocator's choices
 * ============================================ */
NOINLINE static int pattern_e_register_conflict(int input) {
    /* Explicit register variables that may conflict */
    register int r1 asm ("r12") = input + 1;
    register int r2 asm ("r13") = r1 * 2;
    register int r3 asm ("r14") = r2 - 1;
    register int r4 asm ("r15") = r3 + input;
    
    /* Force these to be used throughout */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        /* Mix explicit and implicit registers */
        int temp1 = r1 + i;
        int temp2 = r2 - i;
        int temp3 = r3 * i;
        int temp4 = r4 ^ i;
        
        /* Force spills by using many temporaries */
        int t1 = temp1 + temp2;
        int t2 = temp3 - temp4;
        int t3 = t1 * t2;
        int t4 = t3 / (t2 ? t2 : 1);
        int t5 = t4 ^ t3;
        int t6 = t5 | t4;
        int t7 = t6 & t5;
        int t8 = t7 << (i & 3);
        int t9 = t8 >> (i & 3);
        int t10 = t9 + t8;
        
        /* Update register variables */
        r1 = t1 + r4;
        r2 = t2 + r3;
        r3 = t3 + r2;
        r4 = t4 + r1;
        
        sum += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
               r1 + r2 + r3 + r4;
    }
    
    /* Ensure register variables are live at end */
    asm volatile("" : : "r"(r1), "r"(r2), "r"(r3), "r"(r4));
    
    return sum;
}

/* ============================================
 * Main function - calls all patterns
 * ============================================ */
COLD int main(int argc, char** argv) {
    int result = 0;
    
    /* Use CPU feature check to engage target-specific optimizations */
    if (__builtin_cpu_supports("avx2")) {
        /* This may affect register allocation decisions */
        result += 1;
    }
    
    /* Array of inputs to prevent constant folding */
    int int_inputs[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    double fp_inputs[] = {1.1, 2.2, 3.3, 4.4, 5.5};
    
    /* Call pattern A multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        result += pattern_a_int_pressure(int_inputs[i % 10]);
    }
    
    /* Call pattern B */
    for (int i = 0; i < 5; i++) {
        double fp_res = pattern_b_fp_pressure(fp_inputs[i % 5]);
        result += (int)fp_res;
    }
    
    /* Call pattern C with various inputs */
    for (int i = 0; i < 20; i++) {
        result += pattern_c_cfg_complexity(i);
    }
    
    /* Call pattern D with SIMD */
    v4si vec_input = {1, 2, 3, 4};
    v4si vec_result = pattern_d_simd_pressure(vec_input);
    result += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
    
    /* Call pattern E */
    for (int i = 0; i < 10; i++) {
        result += pattern_e_register_conflict(i);
    }
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %d\n", result);
    
    return result % 256;
}
