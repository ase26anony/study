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
#define COLD __attribute__((cold))

/* Vector type for SIMD pressure */
typedef int v4si __attribute__((vector_size(16)));

/* ============================================================
   PATTERN A: Integer arithmetic chain with 30+ variables
   Creates massive register pressure in a tight loop
   ============================================================ */
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
    a5 = a4 / (input != 0 ? input : 1) + a1;
    a6 = a5 ^ a4 + input;
    a7 = a6 | a5 + input;
    a8 = a7 & a6 + input;
    a9 = a8 << 2 + input;
    a10 = a9 >> 1 + input;
    
    a11 = a10 + a9 + input;
    a12 = a11 - a10 + input;
    a13 = a12 * a11 + input;
    a14 = a13 / (a12 != 0 ? a12 : 1) + input;
    a15 = a14 ^ a13 + input;
    a16 = a15 | a14 + input;
    a17 = a16 & a15 + input;
    a18 = a17 << 1 + input;
    a19 = a18 >> 2 + input;
    a20 = a19 + a18 + input;
    
    a21 = a20 - a19 + input;
    a22 = a21 * a20 + input;
    a23 = a22 / (a21 != 0 ? a21 : 1) + input;
    a24 = a23 ^ a22 + input;
    a25 = a24 | a23 + input;
    a26 = a25 & a24 + input;
    a27 = a26 << 3 + input;
    a28 = a27 >> 1 + input;
    a29 = a28 + a27 + input;
    a30 = a29 - a28 + input;
    
    a31 = a30 * a29 + input;
    a32 = a31 / (a30 != 0 ? a30 : 1) + input;
    a33 = a32 ^ a31 + input;
    a34 = a33 | a32 + input;
    a35 = a34 & a33 + input;
    a36 = a35 << 2 + input;
    a37 = a36 >> 2 + input;
    a38 = a37 + a36 + input;
    a39 = a38 - a37 + input;
    a40 = a39 * a38 + input;
    
    /* Complex loop with interdependent calculations */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        /* Force register pressure by using many variables in loop */
        a1 = a2 + i;
        a2 = a3 - i;
        a3 = a4 * (i + 1);
        a4 = a5 / ((i % 10) + 1);
        a5 = a6 ^ i;
        a6 = a7 | i;
        a7 = a8 & i;
        a8 = a9 << (i % 4);
        a9 = a10 >> (i % 4);
        a10 = a11 + a1;
        
        a11 = a12 - a2;
        a12 = a13 * a3;
        a13 = a14 / ((a4 % 10) + 1);
        a14 = a15 ^ a5;
        a15 = a16 | a6;
        a16 = a17 & a7;
        a17 = a18 << (i % 3);
        a18 = a19 >> (i % 3);
        a19 = a20 + a8;
        a20 = a21 - a9;
        
        /* Use asm to prevent optimization */
        asm volatile ("" : : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5));
        asm volatile ("" : : "r"(a6), "r"(a7), "r"(a8), "r"(a9), "r"(a10));
        
        sum += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    }
    
    return sum + a40;
}

/* ============================================================
   PATTERN B: Floating-point intensive computation
   Pressures floating-point and vector registers
   ============================================================ */
NOINLINE static double pattern_b_float_pressure(double input) {
    /* 20+ double variables to pressure FP registers */
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    double d11, d12, d13, d14, d15, d16, d17, d18, d19, d20;
    double d21, d22, d23, d24, d25;
    
    d1 = input + 1.0;
    d2 = d1 * 2.0 + input;
    d3 = d2 - d1 + input;
    d4 = d3 * d2 + input;
    d5 = d4 / (input != 0.0 ? input : 1.0) + d1;
    d6 = d5 * 0.5 + input;
    d7 = d6 + d5 + input;
    d8 = d7 - d6 + input;
    d9 = d8 * d7 + input;
    d10 = d9 / (d8 != 0.0 ? d8 : 1.0) + input;
    
    d11 = d10 * 3.14159 + input;
    d12 = d11 - d10 + input;
    d13 = d12 * d11 + input;
    d14 = d13 / (d12 != 0.0 ? d12 : 1.0) + input;
    d15 = d14 * 2.71828 + input;
    d16 = d15 + d14 + input;
    d17 = d16 - d15 + input;
    d18 = d17 * d16 + input;
    d19 = d18 / (d17 != 0.0 ? d17 : 1.0) + input;
    d20 = d19 * 1.41421 + input;
    
    d21 = d20 + d19 + input;
    d22 = d21 - d20 + input;
    d23 = d22 * d21 + input;
    d24 = d23 / (d22 != 0.0 ? d22 : 1.0) + input;
    d25 = d24 * 0.70711 + input;
    
    /* Nested loops with FP operations */
    double result = 0.0;
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 10; j++) {
            /* Complex FP calculations */
            d1 = d2 * (i + 1) + j;
            d2 = d3 / ((j % 5) + 1.0) + i;
            d3 = d4 + d1 - d2;
            d4 = d5 * d3 + i * j;
            d5 = d6 - d4 / ((i % 3) + 1.0);
            
            d6 = d7 * 1.1 + j * 0.1;
            d7 = d8 / 2.0 + i * 0.01;
            d8 = d9 + d6 - d7;
            d9 = d10 * d8 + i + j;
            d10 = d11 - d9 * 0.5;
            
            /* Mix integer and FP */
            d11 = d12 + (i & 0xF) * 0.0625;
            d12 = d13 - (j | 0x1) * 0.125;
            d13 = d14 * ((i ^ j) + 1);
            d14 = d15 / (((i + j) % 7) + 1.0);
            d15 = d16 + d11 * d12;
            
            /* Prevent dead code elimination */
            asm volatile ("" : : "f"(d1), "f"(d2), "f"(d3), "f"(d4), "f"(d5));
            
            result += d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10;
        }
        
        /* Break/continue to create complex CFG */
        if (i % 7 == 0) continue;
        if (i % 13 == 0) break;
    }
    
    return result + d25;
}

/* ============================================================
   PATTERN C: Complex control flow with switch statement
   Creates many basic blocks for MCF fixup graph
   ============================================================ */
NOINLINE static int pattern_c_complex_cfg(int input) {
    int result = input;
    
    /* Outer loop with switch inside */
    for (int i = 0; i < 100; i++) {
        /* Switch with 20+ cases creates many basic blocks */
        switch ((i + input) % 23) {
            case 0:
                result += i * 2;
                break;
            case 1:
                result -= i * 3;
                break;
            case 2:
                result |= i & 0xFF;
                break;
            case 3:
                result ^= i << 2;
                break;
            case 4:
                result = (result * i) & 0xFFFF;
                break;
            case 5:
                result = result / ((i % 10) + 1);
                break;
            case 6:
                result = ~result & i;
                break;
            case 7:
                result = result << (i % 4);
                break;
            case 8:
                result = result >> (i % 4);
                break;
            case 9:
                result = result + (i * i);
                break;
            case 10:
                result = result - (i | 0xAA);
                break;
            case 11:
                result = result & (i ^ 0x55);
                break;
            case 12:
                result = result | (i << 8);
                break;
            case 13:
                result = result ^ (i >> 1);
                break;
            case 14:
                result = result * ((i % 7) + 1);
                break;
            case 15:
                result = result / ((i % 5) + 1);
                break;
            case 16:
                result = result % ((i % 6) + 1);
                break;
            case 17:
                result = -result + i;
                break;
            case 18:
                result = (result + i) * 2;
                break;
            case 19:
                result = (result - i) / 2;
                break;
            case 20:
                result = result & ~i;
                break;
            case 21:
                result = result | (i << 16);
                break;
            case 22:
                result = result ^ (i << 24);
                break;
            default:
                result = 0;
        }
        
        /* Nested if-else chain */
        if (i % 3 == 0) {
            result += 1;
            if (i % 6 == 0) {
                result *= 2;
            } else if (i % 9 == 0) {
                result /= 2;
            } else {
                result -= 1;
            }
        } else if (i % 5 == 0) {
            result |= 0xAA;
            if (i % 10 == 0) {
                result &= 0x55;
            }
        } else if (i % 7 == 0) {
            result ^= 0xFF;
        }
        
        /* Early exit creates exit block */
        if (result > 1000000) {
            break;
        }
        
        /* Continue creates back edge */
        if (result < 0) {
            continue;
        }
    }
    
    return result;
}

/* ============================================================
   PATTERN D: SIMD vector operations
   Pressures vector registers and creates vector spill code
   ============================================================ */
NOINLINE static v4si pattern_d_vector_pressure(v4si input) {
    /* Multiple vector variables */
    v4si v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    v4si v11, v12, v13, v14, v15;
    
    /* Initialize vectors */
    v1 = input + (v4si){1, 2, 3, 4};
    v2 = v1 * (v4si){2, 3, 4, 5};
    v3 = v2 - v1;
    v4 = v3 * v2;
    v5 = v4 + (v4si){5, 6, 7, 8};
    
    v6 = v5 << (v4si){1, 2, 1, 2};
    v7 = v6 >> (v4si){1, 1, 2, 2};
    v8 = v7 | v6;
    v9 = v8 & v7;
    v10 = v9 ^ v8;
    
    v11 = v10 + (v4si){9, 10, 11, 12};
    v12 = v11 * (v4si){3, 4, 5, 6};
    v13 = v12 - v11;
    v14 = v13 * v12;
    v15 = v14 + (v4si){13, 14, 15, 16};
    
    /* Loop with vector operations */
    v4si accum = {0, 0, 0, 0};
    for (int i = 0; i < 50; i++) {
        /* Complex vector calculations */
        v1 = v2 + (v4si){i, i+1, i+2, i+3};
        v2 = v3 * (v4si){i%4, (i+1)%4, (i+2)%4, (i+3)%4};
        v3 = v4 - v1;
        v4 = v5 * v2;
        v5 = v6 + v3;
        
        v6 = v7 << (v4si){i%2, (i+1)%2, i%2, (i+1)%2};
        v7 = v8 >> (v4si){i%3, (i+1)%3, (i+2)%3, i%3};
        v8 = v9 | v6;
        v9 = v10 & v7;
        v10 = v11 ^ v8;
        
        /* Mix with scalar */
        v11 = v12 + (v4si){i&1, i&2, i&3, i&4};
        v12 = v13 * (v4si){i|1, i|2, i|3, i|4};
        v13 = v14 - (v4si){i^1, i^2, i^3, i^4};
        v14 = v15 * v11;
        v15 = v1 + v12;
        
        /* Prevent optimization */
        asm volatile ("" : : "x"(v1), "x"(v2), "x"(v3), "x"(v4), "x"(v5));
        
        accum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    }
    
    return accum + v15;
}

/* ============================================================
   PATTERN E: Explicit register variables
   Conflicts with allocator's choices, prompting fixup
   ============================================================ */
NOINLINE static int pattern_e_explicit_registers(int input) {
    /* Try to use specific registers */
    register int r1 asm ("r12") = input + 1;
    register int r2 asm ("r13") = r1 * 2;
    register int r3 asm ("r14") = r2 - r1;
    register int r4 asm ("r15") = r3 * r2;
    
    /* More variables that will conflict */
    int v1 = r4 + 1;
    int v2 = v1 * r1;
    int v3 = v2 - r2;
    int v4 = v3 * r3;
    int v5 = v4 / (r4 != 0 ? r4 : 1);
    int v6 = v5 ^ r1;
    int v7 = v6 | r2;
    int v8 = v7 & r3;
    int v9 = v8 << 2;
    int v10 = v9 >> 1;
    
    /* Computed goto for complex CFG (GCC extension) */
    void* labels[] = {&&L0, &&L1, &&L2, &&L3, &&L4, &&L5, 
                     &&L6, &&L7, &&L8, &&L9, &&L10};
    
    int result = 0;
    for (int i = 0; i < 100; i++) {
        /* Use computed goto */
        goto *labels[i % 11];
        
        L0:
            result += r1 + i;
            continue;
        L1:
            result += r2 - i;
            continue;
        L2:
            result += r3 * i;
            continue;
        L3:
            result += r4 / ((i % 5) + 1);
            continue;
        L4:
            result += v1 ^ i;
            continue;
        L5:
            result += v2 | i;
            continue;
        L6:
            result += v3 & i;
            continue;
        L7:
            result += v4 << (i % 4);
            continue;
        L8:
            result += v5 >> (i % 4);
            continue;
        L9:
            result += v6 + v1;
            continue;
        L10:
            result += v7 - v2;
            continue;
    }
    
    /* Force use of all register variables */
    asm volatile ("" : : "r"(r1), "r"(r2), "r"(r3), "r"(r4));
    
    return result + v8 + v9 + v10;
}

/* ============================================================
   Main function - calls all patterns
   ============================================================ */
COLD int main(int argc, char** argv) {
    int total = 0;
    
    /* Use CPU features to engage target-specific optimizations */
#ifdef __GNUC__
    if (__builtin_cpu_supports("avx2")) {
        /* This may affect register allocation decisions */
        total += 1;
    }
    if (__builtin_cpu_supports("sse4.2")) {
        total += 2;
    }
#endif
    
    /* Array of inputs to prevent constant folding */
    int int_inputs[] = {1, 2, 3, 5, 7, 11, 13, 17, 19, 23};
    double float_inputs[] = {1.1, 2.2, 3.3, 5.5, 7.7, 11.11};
    v4si vector_input = {1, 2, 3, 4};
    
    /* Call each pattern multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        total += pattern_a_int_pressure(int_inputs[i % 10]);
        
        if (i < 6) {
            double fp_result = pattern_b_float_pressure(float_inputs[i]);
            total += (int)fp_result;
        }
        
        total += pattern_c_complex_cfg(int_inputs[i % 10]);
        
        if (i % 3 == 0) {
            v4si vec_result = pattern_d_vector_pressure(vector_input);
            total += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
            vector_input += (v4si){1, 1, 1, 1};
        }
        
        if (i % 2 == 0) {
            total += pattern_e_explicit_registers(int_inputs[i % 10]);
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return total > 0 ? 0 : 1;
}
