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

/* ============================================
 * PATTERN A: Integer arithmetic chain with 30+ variables
 * Creates massive register pressure in a tight loop
 * ============================================ */
NOINLINE static int pattern_a_int_pressure(int input) {
    /* 30+ integer variables to pressure general purpose registers */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int a11, a12, a13, a14, a15, a16, a17, a18, a19, a20;
    int a21, a22, a23, a24, a25, a26, a27, a28, a29, a30;
    int a31, a32, a33, a34, a35;
    
    /* Complex initialization chain */
    a1 = input + 1;
    a2 = a1 * 2 - input;
    a3 = a2 + a1;
    a4 = a3 ^ a2;
    a5 = a4 * a3;
    a6 = a5 - a4;
    a7 = a6 / (a5 > 0 ? a5 : 1);
    a8 = a7 << 2;
    a9 = a8 | a7;
    a10 = a9 & 0xFF;
    
    a11 = a10 + a9;
    a12 = a11 * a10;
    a13 = a12 - a11;
    a14 = a13 ^ a12;
    a15 = a14 + input;
    a16 = a15 * 3;
    a17 = a16 % 17;
    a18 = a17 << 1;
    a19 = a18 | 0xAA;
    a20 = a19 & 0x55;
    
    a21 = a20 + a19;
    a22 = a21 * a20;
    a23 = a22 - a21;
    a24 = a23 ^ a22;
    a25 = a24 + input;
    a26 = a25 * 5;
    a27 = a26 % 23;
    a28 = a27 << 3;
    a29 = a28 | 0xCC;
    a30 = a29 & 0x33;
    
    a31 = a30 + a29;
    a32 = a31 * a30;
    a33 = a32 - a31;
    a34 = a33 ^ a32;
    a35 = a34 + input;
    
    /* Interdependent loop to prevent optimization */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        a1 = (a1 + i) & 0xFF;
        a2 = a2 ^ a1;
        a3 = a3 + a2;
        a4 = a4 * (a3 > 0 ? a3 : 1);
        a5 = a5 - a4;
        
        /* Use all variables to prevent dead code elimination */
        asm volatile ("" : : "r"(a6), "r"(a7), "r"(a8), "r"(a9), "r"(a10));
        asm volatile ("" : : "r"(a11), "r"(a12), "r"(a13), "r"(a14), "r"(a15));
        asm volatile ("" : : "r"(a16), "r"(a17), "r"(a18), "r"(a19), "r"(a20));
        asm volatile ("" : : "r"(a21), "r"(a22), "r"(a23), "r"(a24), "r"(a25));
        asm volatile ("" : : "r"(a26), "r"(a27), "r"(a28), "r"(a29), "r"(a30));
        asm volatile ("" : : "r"(a31), "r"(a32), "r"(a33), "r"(a34), "r"(a35));
        
        sum += a1 + a35;
    }
    
    return sum;
}

/* ============================================
 * PATTERN B: Floating-point intensive computation
 * Pressures floating-point and MMX/SSE registers
 * ============================================ */
NOINLINE static double pattern_b_float_pressure(double input) {
    /* 20+ double variables */
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    double d11, d12, d13, d14, d15, d16, d17, d18, d19, d20;
    double d21, d22, d23, d24, d25;
    
    d1 = input + 1.0;
    d2 = d1 * 1.5;
    d3 = d2 / (d1 + 0.1);
    d4 = d3 - d2;
    d5 = d4 * d3;
    d6 = d5 + d4;
    d7 = d6 / (d5 + 1.0);
    d8 = d7 * 2.0;
    d9 = d8 - d7;
    d10 = d9 * d8;
    
    d11 = d10 + 3.14159;
    d12 = d11 * 2.71828;
    d13 = d12 / d11;
    d14 = d13 - d12;
    d15 = d14 * 1.41421;
    d16 = d15 + d14;
    d17 = d16 / 3.0;
    d18 = d17 * 4.0;
    d19 = d18 - d17;
    d20 = d19 * 5.0;
    
    d21 = d20 + input;
    d22 = d21 * 0.5;
    d23 = d22 / (d21 + 0.01);
    d24 = d23 - d22;
    d25 = d24 * 100.0;
    
    /* Complex floating-point loop */
    double result = 0.0;
    for (int i = 0; i < 50; i++) {
        d1 = d1 * 0.99 + 0.01;
        d2 = d2 / (d1 + 0.001);
        d3 = d3 + sin((double)i * 0.1); /* Use math to prevent optimization */
        d4 = d4 * cos((double)i * 0.05);
        
        /* Mix all variables */
        d5 = d5 + d1 - d2 + d3 * d4;
        d6 = d6 * 0.95 + d5 * 0.05;
        d7 = d7 / (d6 > 0.0 ? d6 : 1.0);
        
        /* Force register usage */
        asm volatile ("" : : "x"(d8), "x"(d9), "x"(d10));
        asm volatile ("" : : "x"(d11), "x"(d12), "x"(d13));
        asm volatile ("" : : "x"(d14), "x"(d15), "x"(d16));
        asm volatile ("" : : "x"(d17), "x"(d18), "x"(d19));
        asm volatile ("" : : "x"(d20), "x"(d21), "x"(d22));
        asm volatile ("" : : "x"(d23), "x"(d24), "x"(d25));
        
        result += d25;
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
    for (int i = 0; i < 20; i++) {
        if (i % 3 == 0) continue;
        
        for (int j = 0; j < 15; j++) {
            if (j == i) break;
            
            /* Switch with 20+ cases */
            switch ((i + j) % 23) {
                case 0:  result += 1; break;
                case 1:  result *= 2; break;
                case 2:  result ^= 0xAA; break;
                case 3:  result -= 3; break;
                case 4:  result |= 0x55; break;
                case 5:  result &= 0xFF; break;
                case 6:  result <<= 1; break;
                case 7:  result >>= 1; break;
                case 8:  result = ~result; break;
                case 9:  result += i; break;
                case 10: result *= j; break;
                case 11: result ^= j; break;
                case 12: result -= i; break;
                case 13: result |= i; break;
                case 14: result &= j; break;
                case 15: result <<= (i & 3); break;
                case 16: result >>= (j & 3); break;
                case 17: result = result * 3 / 2; break;
                case 18: result = result ^ 0x1234; break;
                case 19: result = result | 0xABCD; break;
                case 20: result = result & 0xDCBA; break;
                case 21: result = result + 0x1000; break;
                case 22: result = result - 0x2000; break;
                default: result = 0; break;
            }
            
            /* Early exit with goto to create irregular CFG */
            if (result > 1000000) goto cleanup;
        }
    }
    
cleanup:
    return result;
}

/* ============================================
 * PATTERN D: Vector extensions for SIMD register pressure
 * ============================================ */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

NOINLINE static v4si pattern_d_vector_pressure(v4si input) {
    v4si v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    v4si v11, v12, v13, v14, v15;
    
    /* Vector operations */
    v1 = input + (v4si){1, 2, 3, 4};
    v2 = v1 * (v4si){2, 2, 2, 2};
    v3 = v2 - v1;
    v4 = v3 & (v4si){0xFF, 0xFF, 0xFF, 0xFF};
    v5 = v4 | (v4si){0xAA, 0xBB, 0xCC, 0xDD};
    v6 = v5 ^ (v4si){0x55, 0x66, 0x77, 0x88};
    v7 = v6 << (v4si){1, 2, 3, 4};
    v8 = v7 >> (v4si){4, 3, 2, 1};
    v9 = v8 + v7;
    v10 = v9 * v8;
    
    v11 = v10 - v9;
    v12 = v11 & v10;
    v13 = v12 | v11;
    v14 = v13 ^ v12;
    v15 = v14 + input;
    
    /* Vector loop */
    for (int i = 0; i < 25; i++) {
        v1 = v1 + (v4si){i, i+1, i+2, i+3};
        v2 = v2 * (v4si){i%2+1, i%3+1, i%4+1, i%5+1};
        v3 = v3 - v1;
        v4 = v4 & v2;
        v5 = v5 | v3;
        
        /* Use all vector variables */
        asm volatile ("" : : "x"(v6), "x"(v7), "x"(v8));
        asm volatile ("" : : "x"(v9), "x"(v10), "x"(v11));
        asm volatile ("" : : "x"(v12), "x"(v13), "x"(v14));
        asm volatile ("" : : "x"(v15));
        
        v15 = v15 + v1;
    }
    
    return v15;
}

/* ============================================
 * PATTERN E: Explicit register variables
 * Conflicts with allocator's choices
 * ============================================ */
NOINLINE static int pattern_e_explicit_registers(int input) {
    /* Try to explicitly allocate registers */
    register int r1 asm ("r12");
    register int r2 asm ("r13");
    register int r3 asm ("r14");
    register int r4 asm ("r15");
    
    r1 = input;
    r2 = r1 * 2;
    r3 = r2 + r1;
    r4 = r3 ^ r2;
    
    /* Force spills by using many other variables */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    
    v1 = r4 + 1;
    v2 = v1 * r1;
    v3 = v2 - r2;
    v4 = v3 ^ r3;
    v5 = v4 | r4;
    v6 = v5 & v1;
    v7 = v6 << r1;
    v8 = v7 >> r2;
    v9 = v8 + r3;
    v10 = v9 * r4;
    
    v11 = v10 - v1;
    v12 = v11 ^ v2;
    v13 = v12 | v3;
    v14 = v13 & v4;
    v15 = v14 + v5;
    v16 = v15 * v6;
    v17 = v16 - v7;
    v18 = v17 ^ v8;
    v19 = v18 | v9;
    v20 = v19 & v10;
    
    /* Complex computation using all variables */
    for (int i = 0; i < 30; i++) {
        r1 = (r1 + i) & 0xFF;
        r2 = r2 ^ r1;
        r3 = r3 + r2;
        r4 = r4 * (r3 > 0 ? r3 : 1);
        
        v1 = v1 + r1;
        v2 = v2 * r2;
        v3 = v3 - r3;
        v4 = v4 ^ r4;
        
        /* Prevent optimization */
        asm volatile ("" : : "r"(v5), "r"(v6), "r"(v7), "r"(v8), "r"(v9), "r"(v10));
        asm volatile ("" : : "r"(v11), "r"(v12), "r"(v13), "r"(v14), "r"(v15));
        asm volatile ("" : : "r"(v16), "r"(v17), "r"(v18), "r"(v19), "r"(v20));
    }
    
    return r1 + r2 + r3 + r4 + v20;
}

/* ============================================
 * Main function - calls all patterns
 * ============================================ */
COLD int main(int argc, char **argv) {
    int total = 0;
    
    /* Use CPU feature check to engage target-specific optimizations */
    if (__builtin_cpu_supports("avx2") || 
        __builtin_cpu_supports("sse2") ||
        __builtin_cpu_supports("mmx")) {
        /* This ensures architecture-specific register allocation paths */
    }
    
    /* Call each pattern multiple times with different inputs */
    int inputs[] = {1, 2, 3, 5, 7, 11, 13, 17, 19, 23};
    
    for (int i = 0; i < 10; i++) {
        total += pattern_a_int_pressure(inputs[i]);
        total += (int)pattern_b_float_pressure((double)inputs[i]);
        total += pattern_c_complex_cfg(inputs[i]);
        
        v4si vec_input = {inputs[i], inputs[i]+1, inputs[i]+2, inputs[i]+3};
        v4si vec_result = pattern_d_vector_pressure(vec_input);
        total += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
        
        total += pattern_e_explicit_registers(inputs[i]);
    }
    
    /* Prevent dead code elimination of total */
    asm volatile ("" : : "r"(total));
    
    /* Optional debug output - doesn't affect coverage */
    if (argc > 1) {
        printf("Total: %d\n", total);
    }
    
    return total > 0 ? 0 : 1;
}
