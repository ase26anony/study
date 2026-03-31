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
    int sum = 0;
    
    /* Complex interdependent calculations */
    a1 = input + 1;
    a2 = a1 * 2;
    a3 = a2 - a1;
    a4 = a3 + input;
    a5 = a4 * a2;
    a6 = a5 / (a1 + 1);
    a7 = a6 ^ a5;
    a8 = a7 | a4;
    a9 = a8 & a3;
    a10 = a9 << 2;
    
    a11 = a10 >> 1;
    a12 = a11 + a9;
    a13 = a12 * a8;
    a14 = a13 - a7;
    a15 = a14 + a6;
    a16 = a15 * a5;
    a17 = a16 / (a4 + 1);
    a18 = a17 ^ a16;
    a19 = a18 | a15;
    a20 = a19 & a14;
    
    a21 = a20 << 3;
    a22 = a21 >> 2;
    a23 = a22 + a20;
    a24 = a23 * a19;
    a25 = a24 - a18;
    a26 = a25 + a17;
    a27 = a26 * a16;
    a28 = a27 / (a15 + 1);
    a29 = a28 ^ a27;
    a30 = a29 | a26;
    
    /* Use all variables in a loop to prevent optimization */
    for (int i = 0; i < 10; i++) {
        a1 += i; a2 += a1; a3 += a2; a4 += a3; a5 += a4;
        a6 += i; a7 += a6; a8 += a7; a9 += a8; a10 += a9;
        a11 += i; a12 += a11; a13 += a12; a14 += a13; a15 += a14;
        a16 += i; a17 += a16; a18 += a17; a19 += a18; a20 += a19;
        a21 += i; a22 += a21; a23 += a22; a24 += a23; a25 += a24;
        a26 += i; a27 += a26; a28 += a27; a29 += a28; a30 += a29;
    }
    
    /* Force use of all variables */
    asm volatile ("" : : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5));
    asm volatile ("" : : "r"(a6), "r"(a7), "r"(a8), "r"(a9), "r"(a10));
    asm volatile ("" : : "r"(a11), "r"(a12), "r"(a13), "r"(a14), "r"(a15));
    asm volatile ("" : : "r"(a16), "r"(a17), "r"(a18), "r"(a19), "r"(a20));
    asm volatile ("" : : "r"(a21), "r"(a22), "r"(a23), "r"(a24), "r"(a25));
    asm volatile ("" : : "r"(a26), "r"(a27), "r"(a28), "r"(a29), "r"(a30));
    
    sum = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
          a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19 + a20 +
          a21 + a22 + a23 + a24 + a25 + a26 + a27 + a28 + a29 + a30;
    
    return sum;
}

/* ============================================
 * PATTERN B: Floating-point intensive computation
 * Pressures floating-point and SIMD registers
 * ============================================ */
NOINLINE static double pattern_b_fp_pressure(double input) {
    /* 20+ double variables */
    double b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    double b11, b12, b13, b14, b15, b16, b17, b18, b19, b20;
    
    /* Complex FP calculations */
    b1 = input + 1.0;
    b2 = b1 * 2.0;
    b3 = b2 - b1;
    b4 = b3 * 3.14159;
    b5 = b4 / (b1 + 1.0);
    b6 = b5 * b4;
    b7 = b6 - b3;
    b8 = b7 + b2;
    b9 = b8 * b1;
    b10 = b9 / (input + 0.5);
    
    b11 = b10 * 2.71828;
    b12 = b11 - b9;
    b13 = b12 + b8;
    b14 = b13 * b7;
    b15 = b14 / (b6 + 1.0);
    b16 = b15 * b5;
    b17 = b16 - b4;
    b18 = b17 + b3;
    b19 = b18 * b2;
    b20 = b19 / (b1 + 1.0);
    
    /* Loop with FP operations */
    for (int i = 0; i < 8; i++) {
        b1 += 0.1 * i; b2 += b1; b3 += b2; b4 += b3; b5 += b4;
        b6 += 0.2 * i; b7 += b6; b8 += b7; b9 += b8; b10 += b9;
        b11 += 0.3 * i; b12 += b11; b13 += b12; b14 += b13; b15 += b14;
        b16 += 0.4 * i; b17 += b16; b18 += b17; b19 += b18; b20 += b19;
    }
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(b1), "r"(b2), "r"(b3), "r"(b4), "r"(b5));
    asm volatile ("" : : "r"(b6), "r"(b7), "r"(b8), "r"(b9), "r"(b10));
    asm volatile ("" : : "r"(b11), "r"(b12), "r"(b13), "r"(b14), "r"(b15));
    asm volatile ("" : : "r"(b16), "r"(b17), "r"(b18), "r"(b19), "r"(b20));
    
    return b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9 + b10 +
           b11 + b12 + b13 + b14 + b15 + b16 + b17 + b18 + b19 + b20;
}

/* ============================================
 * PATTERN C: Complex control flow with switch
 * Creates many basic blocks for CFG complexity
 * ============================================ */
NOINLINE static int pattern_c_cfg_complexity(int input) {
    int result = input;
    
    /* Nested loops with breaks/continues */
    for (int i = 0; i < 5; i++) {
        if (i % 2 == 0) continue;
        
        for (int j = 0; j < 10; j++) {
            if (j == 5) break;
            result += j * i;
        }
        
        /* Switch with many cases */
        switch (i) {
            case 0: result += 100; break;
            case 1: result += 200; break;
            case 2: result += 300; break;
            case 3: result += 400; break;
            case 4: result += 500; break;
            case 5: result += 600; break;
            case 6: result += 700; break;
            case 7: result += 800; break;
            case 8: result += 900; break;
            case 9: result += 1000; break;
            case 10: result += 1100; break;
            case 11: result += 1200; break;
            case 12: result += 1300; break;
            case 13: result += 1400; break;
            case 14: result += 1500; break;
            case 15: result += 1600; break;
            case 16: result += 1700; break;
            case 17: result += 1800; break;
            case 18: result += 1900; break;
            case 19: result += 2000; break;
            default: result -= 500; break;
        }
    }
    
    /* Computed goto (GCC extension) for extra CFG complexity */
    void* labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4, &&L5 };
    int idx = result % 6;
    goto *labels[idx];
    
L0: result += 1; goto end;
L1: result += 2; goto end;
L2: result += 3; goto end;
L3: result += 4; goto end;
L4: result += 5; goto end;
L5: result += 6; goto end;
    
end:
    return result;
}

/* ============================================
 * PATTERN D: Vector/SIMD operations
 * Pressures vector registers
 * ============================================ */
NOINLINE static v4si pattern_d_vector_pressure(v4si input) {
    v4si v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    
    /* Vector operations */
    v1 = input + (v4si){1, 2, 3, 4};
    v2 = v1 * (v4si){2, 2, 2, 2};
    v3 = v2 - v1;
    v4 = v3 + input;
    v5 = v4 * v2;
    v6 = v5 + v1;
    v7 = v6 - v3;
    v8 = v7 * v4;
    v9 = v8 + v5;
    v10 = v9 - v6;
    
    /* Loop with vector operations */
    for (int i = 0; i < 8; i++) {
        v1 += (v4si){i, i, i, i};
        v2 = v1 * v2;
        v3 = v2 - v3;
        v4 = v3 + v4;
        v5 = v4 * v5;
        v6 = v5 + v6;
        v7 = v6 - v7;
        v8 = v7 * v8;
        v9 = v8 + v9;
        v10 = v9 - v10;
    }
    
    /* Prevent optimization */
    asm volatile ("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4), "r"(v5));
    asm volatile ("" : : "r"(v6), "r"(v7), "r"(v8), "r"(v9), "r"(v10));
    
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
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
    
    int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    
    /* Force spills by using many other variables */
    temp1 = r1 + r2;
    temp2 = r2 + r3;
    temp3 = r3 + r4;
    temp4 = r4 + r1;
    temp5 = temp1 * temp2;
    temp6 = temp2 * temp3;
    temp7 = temp3 * temp4;
    temp8 = temp4 * temp1;
    
    /* Complex calculations using both register and stack variables */
    for (int i = 0; i < 10; i++) {
        r1 += i; r2 += r1; r3 += r2; r4 += r3;
        temp1 += r1; temp2 += r2; temp3 += r3; temp4 += r4;
        temp5 = temp5 * 2 - temp1;
        temp6 = temp6 / 2 + temp2;
        temp7 = temp7 ^ temp3;
        temp8 = temp8 | temp4;
    }
    
    /* Force all variables to be live */
    asm volatile ("" : : "r"(r1), "r"(r2), "r"(r3), "r"(r4));
    asm volatile ("" : : "r"(temp1), "r"(temp2), "r"(temp3), "r"(temp4));
    asm volatile ("" : : "r"(temp5), "r"(temp6), "r"(temp7), "r"(temp8));
    
    return r1 + r2 + r3 + r4 + temp1 + temp2 + temp3 + temp4 + temp5 + temp6 + temp7 + temp8;
}

/* ============================================
 * Main function - calls all patterns
 * ============================================ */
COLD int main(int argc, char *argv[]) {
    int int_results[5] = {0};
    double fp_result = 0.0;
    v4si vec_result = {0};
    
    /* Use CPU feature check to engage target-specific optimizations */
    if (__builtin_cpu_supports("avx2") || 
        __builtin_cpu_supports("sse4.2") ||
        __builtin_cpu_supports("popcnt")) {
        /* This may affect register allocation decisions */
    }
    
    /* Call each pattern multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        int_results[0] += pattern_a_int_pressure(i);
        fp_result += pattern_b_fp_pressure(i * 0.5);
        int_results[1] += pattern_c_cfg_complexity(i);
        
        v4si vec_input = {i, i+1, i+2, i+3};
        vec_result += pattern_d_vector_pressure(vec_input);
        
        int_results[2] += pattern_e_register_conflict(i);
        
        /* Additional mixed calls */
        int_results[3] += pattern_a_int_pressure(int_results[0] % 100);
        int_results[4] += pattern_c_cfg_complexity(int_results[1] % 50);
    }
    
    /* Prevent dead code elimination of results */
    int total = 0;
    for (int i = 0; i < 5; i++) {
        total += int_results[i];
    }
    
    /* Use results to prevent optimization */
    asm volatile ("" : : "r"(total), "r"(fp_result), "r"(vec_result));
    
    /* Optional debug output */
    if (argc > 1) {
        printf("Total: %d\n", total);
    }
    
    return total % 256;
}
