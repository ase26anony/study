/* test_mcf_coverage.c
 * 
 * This test program is designed to trigger GCC's Min-Cost Flow pass
 * during register allocation, specifically to exercise the print_node
 * function with special node indices (ENTRY_BLOCK, EXIT_BLOCK,
 * new_exit_index, new_entry_index).
 *
 * Compile with: gcc -O3 -fprofile-arcs -ftest-coverage -fdump-rtl-all test_mcf_coverage.c -o test_mcf
 * Run with: ./test_mcf
 * Generate coverage with: gcov test_mcf_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure each function gets its own compilation unit */
#define NOINLINE __attribute__((noinline))
#define COLD __attribute__((cold))

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
    
    /* Initialize with input to prevent constant folding */
    a1 = input;
    
    /* Long dependency chain to prevent reordering */
    for (int i = 0; i < 100; i++) {
        a2 = a1 + i;
        a3 = a2 * a1;
        a4 = a3 - a2;
        a5 = a4 ^ a3;
        a6 = a5 | a4;
        a7 = a6 & a5;
        a8 = a7 << 2;
        a9 = a8 >> 1;
        a10 = a9 + a8;
        a11 = a10 - a9;
        a12 = a11 * a10;
        a13 = a12 / (a11 + 1);
        a14 = a13 % (a12 + 1);
        a15 = a14 ^ a13;
        a16 = a15 | a14;
        a17 = a16 & a15;
        a18 = a17 << 3;
        a19 = a18 >> 2;
        a20 = a19 + a18;
        a21 = a20 - a19;
        a22 = a21 * a20;
        a23 = a22 / (a21 + 1);
        a24 = a23 % (a22 + 1);
        a25 = a24 ^ a23;
        a26 = a25 | a24;
        a27 = a26 & a25;
        a28 = a27 << 1;
        a29 = a28 >> 1;
        a30 = a29 + a28;
        a31 = a30 - a29;
        a32 = a31 * a30;
        a33 = a32 / (a31 + 1);
        a34 = a33 % (a32 + 1);
        a35 = a34 ^ a33;
        a36 = a35 | a34;
        a37 = a36 & a35;
        a38 = a37 << 4;
        a39 = a38 >> 2;
        a40 = a39 + a38;
        
        /* Feed back to create loop-carried dependency */
        a1 = a40 % 997;
        
        /* Prevent dead code elimination */
        asm volatile ("" : : "r"(a1), "r"(a20), "r"(a40));
    }
    
    /* Complex return to use all variables */
    return a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
           a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19 + a20 +
           a21 + a22 + a23 + a24 + a25 + a26 + a27 + a28 + a29 + a30 +
           a31 + a32 + a33 + a34 + a35 + a36 + a37 + a38 + a39 + a40;
}

/* ============================================
 * PATTERN B: Floating-point intensive computation
 * Pressures floating-point registers
 * ============================================ */
NOINLINE static double pattern_b_fp_pressure(double input) {
    /* 20+ double variables */
    double b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    double b11, b12, b13, b14, b15, b16, b17, b18, b19, b20;
    
    b1 = input;
    
    for (int i = 0; i < 50; i++) {
        /* Floating-point operations with dependencies */
        b2 = b1 * 1.1;
        b3 = b2 + 2.3;
        b4 = b3 - b2;
        b5 = b4 * b3;
        b6 = b5 / (b4 + 0.1);
        b7 = b6 + b5;
        b8 = b7 - b6;
        b9 = b8 * b7;
        b10 = b9 / (b8 + 0.1);
        b11 = b10 + b9;
        b12 = b11 - b10;
        b13 = b12 * b11;
        b14 = b13 / (b12 + 0.1);
        b15 = b14 + b13;
        b16 = b15 - b14;
        b17 = b16 * b15;
        b18 = b17 / (b16 + 0.1);
        b19 = b18 + b17;
        b20 = b19 - b18;
        
        /* Trigonometric operations for more register pressure */
        b1 = b20 * 0.99;
        
        /* Mix with integer operations */
        int temp = (int)(b1 * 1000);
        b1 = (double)temp / 1000.0;
        
        asm volatile ("" : : "r"(b1), "r"(b10), "r"(b20));
    }
    
    return b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9 + b10 +
           b11 + b12 + b13 + b14 + b15 + b16 + b17 + b18 + b19 + b20;
}

/* ============================================
 * PATTERN C: Complex control flow with switch statement
 * Creates many basic blocks for CFG complexity
 * ============================================ */
NOINLINE static int pattern_c_cfg_complexity(int input) {
    int result = input;
    
    /* Outer loop with switch inside */
    for (int i = 0; i < 100; i++) {
        switch (result % 23) {
            case 0:  result += i * 2; break;
            case 1:  result -= i * 3; break;
            case 2:  result ^= i; break;
            case 3:  result |= 0xABCD; break;
            case 4:  result &= 0x1234; break;
            case 5:  result <<= (i % 4); break;
            case 6:  result >>= (i % 4); break;
            case 7:  result = result * 3 + 1; break;
            case 8:  result = result / 2; break;
            case 9:  result = result % 1000; break;
            case 10: result = ~result; break;
            case 11: result = result + (i << 2); break;
            case 12: result = result - (i << 1); break;
            case 13: result = result * result; break;
            case 14: result = result | (1 << (i % 16)); break;
            case 15: result = result & ~(1 << (i % 16)); break;
            case 16: result = result ^ (i << 8); break;
            case 17: result = (result << 16) | (result >> 16); break;
            case 18: result = result + 0xDEADBEEF; break;
            case 19: result = result - 0xCAFEBABE; break;
            case 20: result = result * 0x12345678; break;
            case 21: result = result / (i + 1); break;
            case 22: result = result % (i + 2); break;
        }
        
        /* Nested loop with break/continue */
        for (int j = 0; j < 10; j++) {
            if (j == result % 5) {
                continue;
            }
            if (j == 8) {
                break;
            }
            result += j;
        }
    }
    
    return result;
}

/* ============================================
 * PATTERN D: SIMD vector operations
 * Pressures vector/SIMD registers
 * ============================================ */
NOINLINE static v4si pattern_d_simd_pressure(v4si input) {
    v4si v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    v4si v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    
    v1 = input;
    v4si inc = {1, 2, 3, 4};
    
    for (int i = 0; i < 50; i++) {
        /* Chain of vector operations */
        v2 = v1 + inc;
        v3 = v2 * v1;
        v4 = v3 - v2;
        v5 = v4 & v3;
        v6 = v5 | v4;
        v7 = v6 << 1;
        v8 = v7 >> 1;
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
        
        /* Mix with scalar operations */
        int temp[4];
        temp[0] = v20[0];
        temp[1] = v20[1];
        temp[2] = v20[2];
        temp[3] = v20[3];
        
        v1 = (v4si){temp[3], temp[2], temp[1], temp[0]};
        
        asm volatile ("" : : "r"(v1), "r"(v10), "r"(v20));
    }
    
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
}

/* ============================================
 * PATTERN E: Explicit register variables
 * Conflicts with allocator's choices
 * ============================================ */
NOINLINE static int pattern_e_explicit_registers(int input) {
    /* Try to use specific registers */
    register int r12_var asm ("r12") = input;
    register int r13_var asm ("r13") = input + 1;
    register int r14_var asm ("r14") = input + 2;
    register int r15_var asm ("r15") = input + 3;
    
    int local1, local2, local3, local4, local5;
    
    /* Force conflicts */
    for (int i = 0; i < 100; i++) {
        local1 = r12_var * i;
        local2 = r13_var + local1;
        local3 = r14_var - local2;
        local4 = r15_var ^ local3;
        local5 = local4 % (i + 1);
        
        /* Rotate register values */
        int temp = r12_var;
        r12_var = r13_var;
        r13_var = r14_var;
        r14_var = r15_var;
        r15_var = temp + local5;
        
        /* Use all variables */
        asm volatile ("" : : "r"(r12_var), "r"(r13_var), "r"(r14_var), 
                      "r"(r15_var), "r"(local1), "r"(local5));
    }
    
    return r12_var + r13_var + r14_var + r15_var + local1 + local5;
}

/* ============================================
 * Helper to prevent optimization
 * ============================================ */
static void use_result(int result) {
    /* Use result to prevent dead code elimination */
    static volatile int sink;
    sink = result;
}

/* ============================================
 * Main function - COLD attribute affects block ordering
 * ============================================ */
COLD int main(int argc, char **argv) {
    int total = 0;
    
    /* Varying inputs to prevent constant folding */
    int inputs[] = {1, 2, 3, 5, 7, 11, 13, 17, 19, 23};
    
    /* Use CPU feature detection to engage target-specific optimizations */
#ifdef __GNUC__
    if (__builtin_cpu_supports("avx2")) {
        /* This may affect register allocation decisions */
        asm volatile ("# AVX2 detected, may affect MCF decisions");
    }
#endif
    
    for (int i = 0; i < 10; i++) {
        /* Call each pattern with different inputs */
        int a_result = pattern_a_int_pressure(inputs[i]);
        double b_result = pattern_b_fp_pressure((double)inputs[i]);
        int c_result = pattern_c_cfg_complexity(inputs[i]);
        v4si d_input = {inputs[i], inputs[i] + 1, inputs[i] + 2, inputs[i] + 3};
        v4si d_result = pattern_d_simd_pressure(d_input);
        int e_result = pattern_e_explicit_registers(inputs[i]);
        
        /* Combine results in non-trivial way */
        total += a_result + (int)b_result + c_result + 
                 d_result[0] + d_result[1] + d_result[2] + d_result[3] + 
                 e_result;
        
        /* Prevent optimization */
        use_result(total);
    }
    
    /* Optional debug output */
    if (argc > 1) {
        printf("Total: %d\n", total);
    }
    
    return total % 256;
}
