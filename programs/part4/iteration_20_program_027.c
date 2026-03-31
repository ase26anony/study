/* test_mcf_coverage.c
 * 
 * This test program is designed to trigger GCC's Min-Cost Flow pass
 * during register allocation, specifically to exercise the print_node
 * function with special node indices (ENTRY_BLOCK, EXIT_BLOCK,
 * new_exit_index, new_entry_index).
 *
 * Compile with: gcc -O3 -fprofile-arcs -ftest-coverage -fdump-rtl-bbro -c test_mcf_coverage.c
 * Then run: ./test_mcf_coverage (if linking with -lgcov)
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure each function is compiled separately */
#define NOINLINE __attribute__((noinline))
#define NORETURN __attribute__((noreturn))
#define COLD __attribute__((cold))

/* Vector type for SIMD pressure */
typedef int v4si __attribute__((vector_size(16)));

/* ============================================
 * PATTERN A: Integer arithmetic chain with 30+ variables
 * Creates massive register pressure in a loop
 * ============================================ */
NOINLINE int pattern_a_int_pressure(int input) {
    /* 30+ integer variables to pressure general purpose registers */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int a11, a12, a13, a14, a15, a16, a17, a18, a19, a20;
    int a21, a22, a23, a24, a25, a26, a27, a28, a29, a30;
    int result = 0;
    
    /* Initialize with input to prevent constant folding */
    a1 = input;
    
    /* Long dependency chain to force sequential evaluation */
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
        a11 = a10 * a9;
        a12 = a11 - a10;
        a13 = a12 ^ a11;
        a14 = a13 | a12;
        a15 = a14 & a13;
        a16 = a15 << 3;
        a17 = a16 >> 2;
        a18 = a17 + a16;
        a19 = a18 * a17;
        a20 = a19 - a18;
        a21 = a20 ^ a19;
        a22 = a21 | a20;
        a23 = a22 & a21;
        a24 = a23 << 1;
        a25 = a24 >> 1;
        a26 = a25 + a24;
        a27 = a26 * a25;
        a28 = a27 - a26;
        a29 = a28 ^ a27;
        a30 = a29 | a28;
        
        /* Use all variables to prevent dead code elimination */
        asm volatile("" : : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5),
                         "r"(a6), "r"(a7), "r"(a8), "r"(a9), "r"(a10),
                         "r"(a11), "r"(a12), "r"(a13), "r"(a14), "r"(a15),
                         "r"(a16), "r"(a17), "r"(a18), "r"(a19), "r"(a20),
                         "r"(a21), "r"(a22), "r"(a23), "r"(a24), "r"(a25),
                         "r"(a26), "r"(a27), "r"(a28), "r"(a29), "r"(a30));
        
        result += a30;
        a1 = a30 % 127; /* Prevent pattern recognition */
    }
    
    return result;
}

/* ============================================
 * PATTERN B: Floating-point intensive computation
 * Pressures floating-point and MMX/SSE registers
 * ============================================ */
NOINLINE double pattern_b_fp_pressure(double input) {
    /* 20+ double variables */
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    double d11, d12, d13, d14, d15, d16, d17, d18, d19, d20;
    double result = 0.0;
    
    d1 = input;
    
    for (int i = 0; i < 50; i++) {
        /* Complex FP operations */
        d2 = d1 * 1.1 + i;
        d3 = d2 / 1.3 - d1;
        d4 = d3 * d2;
        d5 = d4 + d3;
        d6 = d5 - d4;
        d7 = d6 * 0.9;
        d8 = d7 / 1.7;
        d9 = d8 + d7;
        d10 = d9 - d8;
        d11 = d10 * d9;
        d12 = d11 / d10;
        d13 = d12 + 3.14159;
        d14 = d13 * 2.71828;
        d15 = d14 - d13;
        d16 = d15 / 1.414;
        d17 = d16 * d15;
        d18 = d17 + d16;
        d19 = d18 - d17;
        d20 = d19 * 0.99;
        
        /* Prevent optimization */
        asm volatile("" : : "x"(d1), "x"(d2), "x"(d3), "x"(d4), "x"(d5),
                         "x"(d6), "x"(d7), "x"(d8), "x"(d9), "x"(d10),
                         "x"(d11), "x"(d12), "x"(d13), "x"(d14), "x"(d15),
                         "x"(d16), "x"(d17), "x"(d18), "x"(d19), "x"(d20));
        
        result += d20;
        d1 = d20 * 0.5;
    }
    
    return result;
}

/* ============================================
 * PATTERN C: Complex control flow with switch statement
 * Creates many basic blocks for CFG complexity
 * ============================================ */
NOINLINE int pattern_c_cfg_complexity(int input) {
    int result = input;
    
    /* Nested loops with breaks/continues */
    for (int i = 0; i < 20; i++) {
        if (i % 3 == 0) continue;
        
        for (int j = 0; j < 15; j++) {
            if (j == 10) break;
            
            /* Switch with 20+ cases */
            switch ((i * j + input) % 23) {
                case 0:  result += 1; break;
                case 1:  result -= 2; break;
                case 2:  result *= 3; break;
                case 3:  result ^= 4; break;
                case 4:  result |= 5; break;
                case 5:  result &= 6; break;
                case 6:  result <<= 1; break;
                case 7:  result >>= 2; break;
                case 8:  result += i; break;
                case 9:  result -= j; break;
                case 10: result *= i + j; break;
                case 11: result ^= i ^ j; break;
                case 12: result |= i | j; break;
                case 13: result &= i & j; break;
                case 14: result <<= (i % 4); break;
                case 15: result >>= (j % 4); break;
                case 16: result += i * j; break;
                case 17: result -= i - j; break;
                case 18: result *= (i % 5) + 1; break;
                case 19: result ^= (j % 7) + 1; break;
                case 20: result |= 0xFF; break;
                case 21: result &= 0x0F; break;
                case 22: result = ~result; break;
                default: result = 0; break;
            }
            
            /* Early return to create exit block complexity */
            if (result > 1000000) return result;
        }
    }
    
    return result;
}

/* ============================================
 * PATTERN D: SIMD vector operations
 * Pressures vector/SIMD registers
 * ============================================ */
NOINLINE v4si pattern_d_simd_pressure(v4si input) {
    v4si v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    v4si result;
    
    v1 = input;
    
    for (int i = 0; i < 25; i++) {
        v2 = v1 + (v4si){i, i+1, i+2, i+3};
        v3 = v2 * v1;
        v4 = v3 - v2;
        v5 = v4 & v3;
        v6 = v5 | v4;
        v7 = v6 << 1;
        v8 = v7 >> 2;
        v9 = v8 + v7;
        v10 = v9 * v8;
        
        /* Mix operations to prevent optimization */
        v1 = v10 ^ (v4si){i*2, i*3, i*4, i*5};
        
        /* Force register usage */
        asm volatile("" : : "x"(v1), "x"(v2), "x"(v3), "x"(v4), "x"(v5),
                         "x"(v6), "x"(v7), "x"(v8), "x"(v9), "x"(v10));
    }
    
    result = v1;
    return result;
}

/* ============================================
 * PATTERN E: Explicit register variables
 * Conflicts with allocator's choices
 * ============================================ */
NOINLINE int pattern_e_explicit_registers(int input) {
    /* Try to use specific registers */
    register int r1 asm ("r12") = input;
    register int r2 asm ("r13") = input + 1;
    register int r3 asm ("r14") = input + 2;
    register int r4 asm ("r15") = input + 3;
    
    int temp;
    
    /* Force these registers to be live simultaneously */
    for (int i = 0; i < 100; i++) {
        r1 = r1 * 3 + i;
        r2 = r2 ^ r1;
        r3 = r3 | r2;
        r4 = r4 & r3;
        
        /* Mix with stack variables to force spills */
        temp = r1 + r2 + r3 + r4;
        r1 = temp - r4;
        r2 = temp ^ r3;
        r3 = temp | r2;
        r4 = temp & r1;
        
        /* Prevent dead code elimination */
        asm volatile("" : : "r"(r1), "r"(r2), "r"(r3), "r"(r4));
    }
    
    return r1 + r2 + r3 + r4;
}

/* ============================================
 * Helper function with computed goto
 * Creates irreducible control flow
 * ============================================ */
NOINLINE int computed_goto_test(int input) {
    static void* labels[] = {
        &&label0, &&label1, &&label2, &&label3, &&label4,
        &&label5, &&label6, &&label7, &&label8, &&label9
    };
    
    int result = input;
    int index = input % 10;
    
    goto *labels[index];
    
label0: result += 1; goto end;
label1: result += 2; goto end;
label2: result += 3; goto end;
label3: result += 4; goto end;
label4: result += 5; goto end;
label5: result += 6; goto end;
label6: result += 7; goto end;
label7: result += 8; goto end;
label8: result += 9; goto end;
label9: result += 10; goto end;

end:
    return result;
}

/* ============================================
 * Main function - COLD attribute may affect block ordering
 * ============================================ */
COLD int main(int argc, char** argv) {
    int int_result = 0;
    double fp_result = 0.0;
    v4si vec_result = {0};
    int test_inputs[] = {1, 2, 3, 5, 7, 11, 13, 17, 19, 23};
    
    /* Use CPU feature detection to engage target-specific optimizations */
#ifdef __GNUC__
    if (__builtin_cpu_supports("avx2")) {
        /* This may affect register allocation decisions */
        int_result += 1000;
    }
#endif
    
    /* Call all pressure functions with varying inputs */
    for (int i = 0; i < 10; i++) {
        int_result += pattern_a_int_pressure(test_inputs[i]);
        fp_result += pattern_b_fp_pressure(test_inputs[i] * 0.5);
        int_result += pattern_c_cfg_complexity(test_inputs[i]);
        
        v4si vec_input = {test_inputs[i], test_inputs[i]+1, 
                          test_inputs[i]+2, test_inputs[i]+3};
        vec_result = vec_result + pattern_d_simd_pressure(vec_input);
        
        int_result += pattern_e_explicit_registers(test_inputs[i]);
        int_result += computed_goto_test(test_inputs[i]);
    }
    
    /* Print results to prevent entire program elimination */
    printf("Results: int=%d, fp=%f, vec=[%d,%d,%d,%d]\n",
           int_result, fp_result,
           vec_result[0], vec_result[1], vec_result[2], vec_result[3]);
    
    return 0;
}
