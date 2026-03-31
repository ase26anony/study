/* test_mcf_coverage.c
 * 
 * This test program is designed to trigger GCC's Min-Cost Flow pass
 * during register allocation, specifically to cover the special node
 * printing logic in mcf.cc's print_node function.
 *
 * Compile with: gcc -O3 -fprofile-arcs -ftest-coverage -fdump-rtl-all test_mcf_coverage.c -o test_mcf
 * Run with: ./test_mcf
 * Generate coverage: gcov test_mcf_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure each function is compiled independently */
#define NOINLINE __attribute__((noinline))
#define NORETURN __attribute__((noreturn))
#define COLD __attribute__((cold))

/* Vector type for SIMD pressure */
typedef int v4si __attribute__((vector_size(16)));

/* Pattern A: Integer arithmetic chain with 30+ variables in a loop
 * Creates massive register pressure forcing MCF spill decisions */
NOINLINE static int pattern_a_int_pressure(int input) {
    /* Declare 30+ integer variables to pressure general purpose registers */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int a11, a12, a13, a14, a15, a16, a17, a18, a19, a20;
    int a21, a22, a23, a24, a25, a26, a27, a28, a29, a30;
    int a31, a32, a33, a34, a35, a36, a37, a38, a39, a40;
    
    /* Initialize with input to prevent constant folding */
    a1 = input + 1;
    a2 = a1 * 2 + input;
    a3 = a2 - a1 + input;
    a4 = a3 * a2 + input;
    a5 = a4 / (a1 + 1) + input;
    a6 = a5 ^ a4 + input;
    a7 = a6 | a5 + input;
    a8 = a7 & a6 + input;
    a9 = a8 << 2 + input;
    a10 = a9 >> 1 + input;
    
    a11 = a10 + a9 + input;
    a12 = a11 - a10 + input;
    a13 = a12 * a11 + input;
    a14 = a13 / (a10 + 1) + input;
    a15 = a14 ^ a13 + input;
    a16 = a15 | a14 + input;
    a17 = a16 & a15 + input;
    a18 = a17 << 3 + input;
    a19 = a18 >> 2 + input;
    a20 = a19 + a18 + input;
    
    a21 = a20 - a19 + input;
    a22 = a21 * a20 + input;
    a23 = a22 / (a19 + 1) + input;
    a24 = a23 ^ a22 + input;
    a25 = a24 | a23 + input;
    a26 = a25 & a24 + input;
    a27 = a26 << 4 + input;
    a28 = a27 >> 3 + input;
    a29 = a28 + a27 + input;
    a30 = a29 - a28 + input;
    
    a31 = a30 * a29 + input;
    a32 = a31 / (a28 + 1) + input;
    a33 = a32 ^ a31 + input;
    a34 = a33 | a32 + input;
    a35 = a34 & a33 + input;
    a36 = a35 << 5 + input;
    a37 = a36 >> 4 + input;
    a38 = a37 + a36 + input;
    a39 = a38 - a37 + input;
    a40 = a39 * a38 + input;
    
    /* Complex loop with all variables to prevent dead store elimination */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        /* Use all variables in computation */
        a1 = a2 + a3 + i;
        a2 = a3 + a4 + i;
        a3 = a4 + a5 + i;
        a4 = a5 + a6 + i;
        a5 = a6 + a7 + i;
        a6 = a7 + a8 + i;
        a7 = a8 + a9 + i;
        a8 = a9 + a10 + i;
        a9 = a10 + a11 + i;
        a10 = a11 + a12 + i;
        
        a11 = a12 + a13 + i;
        a12 = a13 + a14 + i;
        a13 = a14 + a15 + i;
        a14 = a15 + a16 + i;
        a15 = a16 + a17 + i;
        a16 = a17 + a18 + i;
        a17 = a18 + a19 + i;
        a18 = a19 + a20 + i;
        a19 = a20 + a21 + i;
        a20 = a21 + a22 + i;
        
        a21 = a22 + a23 + i;
        a22 = a23 + a24 + i;
        a23 = a24 + a25 + i;
        a24 = a25 + a26 + i;
        a25 = a26 + a27 + i;
        a26 = a27 + a28 + i;
        a27 = a28 + a29 + i;
        a28 = a29 + a30 + i;
        a29 = a30 + a31 + i;
        a30 = a31 + a32 + i;
        
        a31 = a32 + a33 + i;
        a32 = a33 + a34 + i;
        a33 = a34 + a35 + i;
        a34 = a35 + a36 + i;
        a35 = a36 + a37 + i;
        a36 = a37 + a38 + i;
        a37 = a38 + a39 + i;
        a38 = a39 + a40 + i;
        a39 = a40 + a1 + i;
        a40 = a1 + a2 + i;
        
        sum += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
               a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19 + a20 +
               a21 + a22 + a23 + a24 + a25 + a26 + a27 + a28 + a29 + a30 +
               a31 + a32 + a33 + a34 + a35 + a36 + a37 + a38 + a39 + a40;
    }
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5),
                       "r"(a6), "r"(a7), "r"(a8), "r"(a9), "r"(a10),
                       "r"(a11), "r"(a12), "r"(a13), "r"(a14), "r"(a15),
                       "r"(a16), "r"(a17), "r"(a18), "r"(a19), "r"(a20),
                       "r"(a21), "r"(a22), "r"(a23), "r"(a24), "r"(a25),
                       "r"(a26), "r"(a27), "r"(a28), "r"(a29), "r"(a30),
                       "r"(a31), "r"(a32), "r"(a33), "r"(a34), "r"(a35),
                       "r"(a36), "r"(a37), "r"(a38), "r"(a39), "r"(a40));
    
    return sum + input;
}

/* Pattern B: Floating-point intensive computation
 * Pressures floating-point/SIMD registers */
NOINLINE static double pattern_b_float_pressure(double input) {
    /* 20 double variables to pressure FP registers */
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    double d11, d12, d13, d14, d15, d16, d17, d18, d19, d20;
    
    d1 = input + 1.0;
    d2 = d1 * 2.0 + input;
    d3 = d2 - d1 + input;
    d4 = d3 * d2 + input;
    d5 = d4 / (d1 + 1.0) + input;
    d6 = d5 * 0.5 + input;
    d7 = d6 + d5 + input;
    d8 = d7 - d6 + input;
    d9 = d8 * d7 + input;
    d10 = d9 / (d6 + 1.0) + input;
    
    d11 = d10 + d9 + input;
    d12 = d11 - d10 + input;
    d13 = d12 * d11 + input;
    d14 = d13 / (d10 + 1.0) + input;
    d15 = d14 * 0.25 + input;
    d16 = d15 + d14 + input;
    d17 = d16 - d15 + input;
    d18 = d17 * d16 + input;
    d19 = d18 / (d15 + 1.0) + input;
    d20 = d19 + d18 + input;
    
    /* Complex floating-point loop */
    double sum = 0.0;
    for (int i = 0; i < 50; i++) {
        d1 = d2 + d3 + i;
        d2 = d3 + d4 + i;
        d3 = d4 + d5 + i;
        d4 = d5 + d6 + i;
        d5 = d6 + d7 + i;
        d6 = d7 + d8 + i;
        d7 = d8 + d9 + i;
        d8 = d9 + d10 + i;
        d9 = d10 + d11 + i;
        d10 = d11 + d12 + i;
        
        d11 = d12 + d13 + i;
        d12 = d13 + d14 + i;
        d13 = d14 + d15 + i;
        d14 = d15 + d16 + i;
        d15 = d16 + d17 + i;
        d16 = d17 + d18 + i;
        d17 = d18 + d19 + i;
        d18 = d19 + d20 + i;
        d19 = d20 + d1 + i;
        d20 = d1 + d2 + i;
        
        sum += d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10 +
               d11 + d12 + d13 + d14 + d15 + d16 + d17 + d18 + d19 + d20;
    }
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(d1), "r"(d2), "r"(d3), "r"(d4), "r"(d5),
                       "r"(d6), "r"(d7), "r"(d8), "r"(d9), "r"(d10),
                       "r"(d11), "r"(d12), "r"(d13), "r"(d14), "r"(d15),
                       "r"(d16), "r"(d17), "r"(d18), "r"(d19), "r"(d20));
    
    return sum + input;
}

/* Pattern C: Complex control flow with switch statement
 * Creates many basic blocks for complex CFG */
NOINLINE static int pattern_c_complex_cfg(int input) {
    int result = input;
    
    /* Nested loops with breaks/continues */
    for (int i = 0; i < 10; i++) {
        if (i % 2 == 0) continue;
        
        for (int j = 0; j < 10; j++) {
            if (j == i) break;
            
            /* Switch with 20+ cases */
            switch ((i * j + input) % 23) {
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
                case 10: result += i; break;
                case 11: result -= j; break;
                case 12: result *= i + j; break;
                case 13: result /= (i + 1); break;
                case 14: result ^= j; break;
                case 15: result |= i; break;
                case 16: result &= j; break;
                case 17: result <<= (i % 4); break;
                case 18: result >>= (j % 4); break;
                case 19: result = result + i - j; break;
                case 20: result = result * 2 - 1; break;
                case 21: result = result / 2 + 1; break;
                case 22: result = result ^ ~0; break;
                default: result += 1000; break;
            }
            
            /* Computed goto (GCC extension) to create irreducible CFG */
            static void *labels[] = {
                &&label0, &&label1, &&label2, &&label3, &&label4,
                &&label5, &&label6, &&label7, &&label8, &&label9
            };
            
            int idx = (i + j) % 10;
            goto *labels[idx];
            
            label0: result += 1; goto end_label;
            label1: result += 2; goto end_label;
            label2: result += 3; goto end_label;
            label3: result += 4; goto end_label;
            label4: result += 5; goto end_label;
            label5: result += 6; goto end_label;
            label6: result += 7; goto end_label;
            label7: result += 8; goto end_label;
            label8: result += 9; goto end_label;
            label9: result += 10; goto end_label;
            end_label:;
        }
    }
    
    return result;
}

/* Pattern D: SIMD vector operations
 * Pressures vector registers */
NOINLINE static v4si pattern_d_simd_pressure(v4si input) {
    v4si v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    
    /* Initialize vectors */
    v1 = input + (v4si){1, 2, 3, 4};
    v2 = v1 * (v4si){2, 2, 2, 2};
    v3 = v2 - v1;
    v4 = v3 * v2;
    v5 = v4 + v1;
    v6 = v5 - v2;
    v7 = v6 * v3;
    v8 = v7 + v4;
    v9 = v8 - v5;
    v10 = v9 * v6;
    
    /* Vector loop */
    for (int i = 0; i < 20; i++) {
        v1 = v2 + v3 + i;
        v2 = v3 + v4 + i;
        v3 = v4 + v5 + i;
        v4 = v5 + v6 + i;
        v5 = v6 + v7 + i;
        v6 = v7 + v8 + i;
        v7 = v8 + v9 + i;
        v8 = v9 + v10 + i;
        v9 = v10 + v1 + i;
        v10 = v1 + v2 + i;
    }
    
    /* Prevent elimination */
    asm volatile ("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4), "r"(v5),
                       "r"(v6), "r"(v7), "r"(v8), "r"(v9), "r"(v10));
    
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
}

/* Pattern E: Explicit register variables
 * Conflicts with allocator's choices */
NOINLINE static int pattern_e_explicit_registers(int input) {
    /* Try to pin variables to specific registers */
    register int r1 asm ("r12") = input + 1;
    register int r2 asm ("r13") = r1 * 2;
    register int r3 asm ("r14") = r2 - r1;
    register int r4 asm ("r15") = r3 * r2;
    
    /* Force spills by using many explicit registers */
    int temp;
    asm volatile ("mov %1, %0" : "=r"(temp) : "r"(r1));
    r1 = temp + 1;
    
    asm volatile ("mov %1, %0" : "=r"(temp) : "r"(r2));
    r2 = temp * 2;
    
    asm volatile ("mov %1, %0" : "=r"(temp) : "r"(r3));
    r3 = temp - 1;
    
    asm volatile ("mov %1, %0" : "=r"(temp) : "r"(r4));
    r4 = temp / 2;
    
    /* Complex computation */
    for (int i = 0; i < 10; i++) {
        r1 = r2 + r3 + i;
        r2 = r3 + r4 + i;
        r3 = r4 + r1 + i;
        r4 = r1 + r2 + i;
        
        /* Inline asm to prevent optimization */
        asm volatile ("" : "+r"(r1), "+r"(r2), "+r"(r3), "+r"(r4));
    }
    
    return r1 + r2 + r3 + r4 + input;
}

/* Main function marked cold to potentially affect block ordering */
COLD int main(int argc, char **argv) {
    int result = 0;
    
    /* Use CPU feature detection to engage target-specific optimizations */
    int use_avx2 = 0;
#ifdef __GNUC__
    if (__builtin_cpu_supports("avx2")) {
        use_avx2 = 1;
    }
#endif
    
    /* Array of inputs to prevent constant folding */
    int int_inputs[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    double float_inputs[] = {1.1, 2.2, 3.3, 4.4, 5.5};
    v4si vector_input = {1, 2, 3, 4};
    
    /* Call all patterns multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        result += pattern_a_int_pressure(int_inputs[i % 10]);
        
        if (i < 5) {
            result += (int)pattern_b_float_pressure(float_inputs[i]);
        }
        
        result += pattern_c_complex_cfg(int_inputs[i % 10]);
        
        if (use_avx2 && (i % 3 == 0)) {
            v4si vec_result = pattern_d_simd_pressure(vector_input);
            result += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
        }
        
        result += pattern_e_explicit_registers(int_inputs[i % 10]);
    }
    
    /* Print result to prevent entire program elimination */
    printf("Final result: %d\n", result);
    
    return result % 256;
}
