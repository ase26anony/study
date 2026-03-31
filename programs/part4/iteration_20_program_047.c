/* test_mcf_coverage.c
 * 
 * This test program is designed to trigger GCC's Min-Cost Flow pass
 * in register allocation, specifically to cover the special node
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

/* ================================================================
 * PATTERN A: Integer arithmetic chain with 30+ variables
 * Creates massive register pressure in a tight loop
 * ================================================================ */
NOINLINE static int pattern_a_int_pressure(int input) {
    /* Declare 30+ integer variables to pressure integer registers */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int a11, a12, a13, a14, a15, a16, a17, a18, a19, a20;
    int a21, a22, a23, a24, a25, a26, a27, a28, a29, a30;
    int a31, a32, a33, a34, a35, a36, a37, a38, a39, a40;
    
    /* Initialize with input to prevent constant folding */
    a1 = input;
    a2 = a1 + 1;
    a3 = a2 * a1;
    a4 = a3 - a2;
    a5 = a4 ^ a3;
    a6 = a5 | a4;
    a7 = a6 & a5;
    a8 = a7 << 2;
    a9 = a8 >> 1;
    a10 = a9 + a8;
    
    a11 = a10 * 3;
    a12 = a11 / 2;
    a13 = a12 % 7;
    a14 = a13 + a12;
    a15 = a14 - a13;
    a16 = a15 * a14;
    a17 = a16 ^ a15;
    a18 = a17 | a16;
    a19 = a18 & a17;
    a20 = a19 << 3;
    
    a21 = a20 >> 2;
    a22 = a21 + a20;
    a23 = a22 * 5;
    a24 = a23 / 3;
    a25 = a24 % 11;
    a26 = a25 + a24;
    a27 = a26 - a25;
    a28 = a27 * a26;
    a29 = a28 ^ a27;
    a30 = a29 | a28;
    
    a31 = a30 & a29;
    a32 = a31 << 1;
    a33 = a32 >> 1;
    a34 = a33 + a32;
    a35 = a34 * 7;
    a36 = a35 / 5;
    a37 = a36 % 13;
    a38 = a37 + a36;
    a39 = a38 - a37;
    a40 = a39 * a38;
    
    /* Complex loop with all variables to force spill decisions */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        /* Use all variables in interdependent computations */
        a1 = (a1 + a2 + i) & 0xFF;
        a2 = (a2 + a3 + i) & 0xFF;
        a3 = (a3 + a4 + i) & 0xFF;
        a4 = (a4 + a5 + i) & 0xFF;
        a5 = (a5 + a6 + i) & 0xFF;
        a6 = (a6 + a7 + i) & 0xFF;
        a7 = (a7 + a8 + i) & 0xFF;
        a8 = (a8 + a9 + i) & 0xFF;
        a9 = (a9 + a10 + i) & 0xFF;
        a10 = (a10 + a11 + i) & 0xFF;
        
        a11 = (a11 + a12 + i) & 0xFF;
        a12 = (a12 + a13 + i) & 0xFF;
        a13 = (a13 + a14 + i) & 0xFF;
        a14 = (a14 + a15 + i) & 0xFF;
        a15 = (a15 + a16 + i) & 0xFF;
        a16 = (a16 + a17 + i) & 0xFF;
        a17 = (a17 + a18 + i) & 0xFF;
        a18 = (a18 + a19 + i) & 0xFF;
        a19 = (a19 + a20 + i) & 0xFF;
        a20 = (a20 + a21 + i) & 0xFF;
        
        a21 = (a21 + a22 + i) & 0xFF;
        a22 = (a22 + a23 + i) & 0xFF;
        a23 = (a23 + a24 + i) & 0xFF;
        a24 = (a24 + a25 + i) & 0xFF;
        a25 = (a25 + a26 + i) & 0xFF;
        a26 = (a26 + a27 + i) & 0xFF;
        a27 = (a27 + a28 + i) & 0xFF;
        a28 = (a28 + a29 + i) & 0xFF;
        a29 = (a29 + a30 + i) & 0xFF;
        a30 = (a30 + a31 + i) & 0xFF;
        
        a31 = (a31 + a32 + i) & 0xFF;
        a32 = (a32 + a33 + i) & 0xFF;
        a33 = (a33 + a34 + i) & 0xFF;
        a34 = (a34 + a35 + i) & 0xFF;
        a35 = (a35 + a36 + i) & 0xFF;
        a36 = (a36 + a37 + i) & 0xFF;
        a37 = (a37 + a38 + i) & 0xFF;
        a38 = (a38 + a39 + i) & 0xFF;
        a39 = (a39 + a40 + i) & 0xFF;
        a40 = (a40 + a1 + i) & 0xFF;
        
        sum += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
               a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19 + a20 +
               a21 + a22 + a23 + a24 + a25 + a26 + a27 + a28 + a29 + a30 +
               a31 + a32 + a33 + a34 + a35 + a36 + a37 + a38 + a39 + a40;
    }
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(sum), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5));
    
    return sum;
}

/* ================================================================
 * PATTERN B: Floating-point intensive computation
 * Pressures floating-point and vector registers
 * ================================================================ */
NOINLINE static double pattern_b_fp_pressure(double input) {
    /* 20 double variables to pressure FP registers */
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    double d11, d12, d13, d14, d15, d16, d17, d18, d19, d20;
    
    d1 = input;
    d2 = d1 * 1.1;
    d3 = d2 + 2.2;
    d4 = d3 * 0.9;
    d5 = d4 - 1.5;
    d6 = d5 / 2.0;
    d7 = d6 * 3.14159;
    d8 = d7 + 2.71828;
    d9 = d8 * 1.41421;
    d10 = d9 / 1.73205;
    
    d11 = d10 + d1;
    d12 = d11 * d2;
    d13 = d12 - d3;
    d14 = d13 / d4;
    d15 = d14 + d5;
    d16 = d15 * d6;
    d17 = d16 - d7;
    d18 = d17 / d8;
    d19 = d18 + d9;
    d20 = d19 * d10;
    
    /* Nested loops with complex control flow */
    double result = 0.0;
    for (int i = 0; i < 50; i++) {
        if (i % 3 == 0) {
            for (int j = 0; j < 10; j++) {
                d1 = d1 * 0.99 + d2;
                d2 = d2 * 0.98 + d3;
                d3 = d3 * 0.97 + d4;
                d4 = d4 * 0.96 + d5;
                d5 = d5 * 0.95 + d6;
                
                if (j % 2 == 0) {
                    d6 = d6 * 1.01 - d7;
                    d7 = d7 * 1.02 - d8;
                    d8 = d8 * 1.03 - d9;
                    d9 = d9 * 1.04 - d10;
                    d10 = d10 * 1.05 - d11;
                } else {
                    d11 = d11 * 0.90 + d12;
                    d12 = d12 * 0.91 + d13;
                    d13 = d13 * 0.92 + d14;
                    d14 = d14 * 0.93 + d15;
                    d15 = d15 * 0.94 + d16;
                }
                
                /* Break/continue to create complex CFG */
                if (j == 5) continue;
                if (j == 8) break;
                
                d16 = d16 * 1.10 - d17;
                d17 = d17 * 1.11 - d18;
                d18 = d18 * 1.12 - d19;
                d19 = d19 * 1.13 - d20;
                d20 = d20 * 1.14 - d1;
            }
        }
        
        result += d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10 +
                  d11 + d12 + d13 + d14 + d15 + d16 + d17 + d18 + d19 + d20;
    }
    
    /* Prevent optimization */
    asm volatile ("" : : "r"(result), "r"(d1), "r"(d2));
    
    return result;
}

/* ================================================================
 * PATTERN C: Complex switch with many basic blocks
 * Creates complex control flow graph for MCF fixup
 * ================================================================ */
NOINLINE static int pattern_c_switch_cfg(int input) {
    int result = input;
    
    /* Switch with 20+ cases to create many basic blocks */
    for (int i = 0; i < 100; i++) {
        switch ((result + i) % 25) {
            case 0:
                result = result * 3 + 1;
                break;
            case 1:
                result = result ^ 0x55AA55AA;
                break;
            case 2:
                result = result >> 1;
                break;
            case 3:
                result = result << 2;
                break;
            case 4:
                result = result | 0xFF00FF00;
                break;
            case 5:
                result = result & 0x00FF00FF;
                break;
            case 6:
                result = result + 0x12345678;
                break;
            case 7:
                result = result - 0x87654321;
                break;
            case 8:
                result = result * 7;
                break;
            case 9:
                result = result / 3;
                break;
            case 10:
                result = result % 17;
                break;
            case 11:
                result = ~result;
                break;
            case 12:
                result = result ^ result;
                break;
            case 13:
                result = result | (result << 8);
                break;
            case 14:
                result = result & (result >> 4);
                break;
            case 15:
                result = result + (result * 2);
                break;
            case 16:
                result = result - (result / 2);
                break;
            case 17:
                result = result * result;
                break;
            case 18:
                result = result ^ (i * 0x11111111);
                break;
            case 19:
                result = result | (i * 0x22222222);
                break;
            case 20:
                result = result & (i * 0x33333333);
                break;
            case 21:
                result = result + (i * 0x44444444);
                break;
            case 22:
                result = result - (i * 0x55555555);
                break;
            case 23:
                result = result * (i % 10 + 1);
                break;
            case 24:
                result = result / ((i % 5) + 1);
                break;
            default:
                result = result + 1;
                break;
        }
        
        /* Nested conditional to add more CFG edges */
        if (result % 7 == 0) {
            result = result + 1000;
        } else if (result % 13 == 0) {
            result = result - 500;
        } else {
            result = result * 2;
        }
    }
    
    return result;
}

/* ================================================================
 * PATTERN D: SIMD vector operations
 * Pressures vector/SIMD registers
 * ================================================================ */
NOINLINE static v4si pattern_d_simd_pressure(v4si input) {
    v4si v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    
    v1 = input;
    v2 = v1 + (v4si){1, 2, 3, 4};
    v3 = v2 * (v4si){2, 3, 4, 5};
    v4 = v3 - (v4si){1, 1, 1, 1};
    v5 = v4 & (v4si){0xFF, 0xFF00, 0xFF0000, 0xFF000000};
    v6 = v5 | (v4si){0x11, 0x22, 0x33, 0x44};
    v7 = v6 ^ (v4si){0x55, 0xAA, 0x55, 0xAA};
    v8 = v7 << (v4si){1, 2, 3, 4};
    v9 = v8 >> (v4si){4, 3, 2, 1};
    v10 = v9 + v1;
    
    /* Loop with vector operations */
    for (int i = 0; i < 50; i++) {
        v1 = v1 + v2;
        v2 = v2 * v3;
        v3 = v3 - v4;
        v4 = v4 & v5;
        v5 = v5 | v6;
        v6 = v6 ^ v7;
        v7 = v7 << (v4si){i % 4, (i + 1) % 4, (i + 2) % 4, (i + 3) % 4};
        v8 = v8 >> (v4si){(i + 3) % 4, (i + 2) % 4, (i + 1) % 4, i % 4};
        v9 = v9 + v10;
        v10 = v10 * v1;
        
        /* Conditional vector operations */
        if (i % 3 == 0) {
            v1 = v1 + (v4si){i, i*2, i*3, i*4};
        } else if (i % 3 == 1) {
            v2 = v2 * (v4si){i, i+1, i+2, i+3};
        } else {
            v3 = v3 - (v4si){i*4, i*3, i*2, i};
        }
    }
    
    /* Mix all vectors */
    v4si result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    
    /* Prevent optimization */
    asm volatile ("" : : "r"(result), "r"(v1), "r"(v2));
    
    return result;
}

/* ================================================================
 * PATTERN E: Explicit register variables
 * Conflicts with register allocator's choices
 * ================================================================ */
NOINLINE static int pattern_e_explicit_registers(int input) {
    /* Explicit register variables that conflict with allocator */
    register int r1 asm ("r12") = input;
    register int r2 asm ("r13") = input * 2;
    register int r3 asm ("r14") = input * 3;
    register int r4 asm ("r15") = input * 4;
    
    int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    
    /* Force spills by using many temporaries */
    for (int i = 0; i < 100; i++) {
        temp1 = r1 + i;
        temp2 = r2 + temp1;
        temp3 = r3 + temp2;
        temp4 = r4 + temp3;
        temp5 = temp1 * temp2;
        temp6 = temp3 * temp4;
        temp7 = temp5 + temp6;
        temp8 = temp7 - i;
        
        /* Update register variables */
        r1 = (r1 + temp1) & 0xFFF;
        r2 = (r2 + temp2) & 0xFFF;
        r3 = (r3 + temp3) & 0xFFF;
        r4 = (r4 + temp4) & 0xFFF;
        
        /* Complex conditional with gotos (GCC extension) */
        if (i % 7 == 0) {
            void *labels[] = {&&L0, &&L1, &&L2, &&L3, &&L4};
            goto *labels[i % 5];
            
            L0:
                r1 = r1 ^ 0xAAAA;
                continue;
            L1:
                r2 = r2 ^ 0xBBBB;
                continue;
            L2:
                r3 = r3 ^ 0xCCCC;
                continue;
            L3:
                r4 = r4 ^ 0xDDDD;
                continue;
            L4:
                r1 = r1 + r2 + r3 + r4;
                continue;
        }
    }
    
    int result = r1 + r2 + r3 + r4;
    
    /* Force register variable usage */
    asm volatile ("" : : "r"(r1), "r"(r2), "r"(r3), "r"(r4));
    
    return result;
}

/* ================================================================
 * Main function - calls all patterns with varying inputs
 * ================================================================ */
COLD int main(int argc, char **argv) {
    int int_results = 0;
    double fp_results = 0.0;
    v4si vec_result = {0};
    int switch_result = 0;
    int explicit_result = 0;
    
    /* Use varying inputs to prevent constant folding */
    int inputs[] = {1, 2, 3, 5, 7, 11, 13, 17, 19, 23};
    
    /* Call each pattern multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        int_results += pattern_a_int_pressure(inputs[i]);
        fp_results += pattern_b_fp_pressure((double)inputs[i] * 0.1);
        switch_result += pattern_c_switch_cfg(inputs[i]);
        explicit_result += pattern_e_explicit_registers(inputs[i]);
        
        /* Vector pattern */
        v4si vec_input = {inputs[i], inputs[i] + 1, inputs[i] + 2, inputs[i] + 3};
        v4si vec_temp = pattern_d_simd_pressure(vec_input);
        vec_result = vec_result + vec_temp;
    }
    
    /* Use CPU features to engage target-specific optimizations */
#ifdef __GNUC__
    if (__builtin_cpu_supports("avx2")) {
        int_results += 1000;
    }
    if (__builtin_cpu_supports("sse4.2")) {
        fp_results += 2000.0;
    }
#endif
    
    /* Prevent dead code elimination of all results */
    int final_int = int_results + switch_result + explicit_result;
    double final_fp = fp_results;
    int final_vec = vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
    
    /* Use results to prevent optimization */
    asm volatile ("" : : "r"(final_int), "r"(final_fp), "r"(final_vec));
    
    /* Optional: Print something to verify execution */
    printf("Test completed. Results: %d, %f, %d\n", 
           final_int & 0xFF, final_fp, final_vec & 0xFF);
    
    return 0;
}
