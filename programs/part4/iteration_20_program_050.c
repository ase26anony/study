/* test_mcf_coverage.c
 * 
 * This test program is designed to trigger GCC's Min-Cost Flow solver
 * during register allocation, specifically to exercise the print_node
 * function with special node indices (ENTRY_BLOCK, EXIT_BLOCK,
 * new_exit_index, new_entry_index).
 *
 * Compile with: gcc -O3 -fprofile-arcs -ftest-coverage -fdump-rtl-all -c test_mcf_coverage.c
 * Then run: ./test_mcf_coverage (if linking with -lgcov)
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
 * Creates massive register pressure in a tight loop
 * ============================================ */
NOINLINE
int pattern_a_int_pressure(int input) {
    /* Declare 30+ integer variables to pressure integer registers */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int a11, a12, a13, a14, a15, a16, a17, a18, a19, a20;
    int a21, a22, a23, a24, a25, a26, a27, a28, a29, a30;
    int a31, a32, a33, a34, a35;
    
    /* Complex interdependent computation to prevent optimization */
    a1 = input + 1;
    a2 = a1 * 3 - input;
    a3 = a2 + a1;
    a4 = a3 ^ a2;
    a5 = a4 * 7 + a3;
    a6 = a5 - a4;
    a7 = a6 / 2 + a5;
    a8 = a7 | a6;
    a9 = a8 * 11 - a7;
    a10 = a9 + a8;
    a11 = a10 & a9;
    a12 = a11 * 13 + a10;
    a13 = a12 - a11;
    a14 = a13 ^ a12;
    a15 = a14 * 17 - a13;
    a16 = a15 + a14;
    a17 = a16 | a15;
    a18 = a17 * 19 + a16;
    a19 = a18 - a17;
    a20 = a19 ^ a18;
    a21 = a20 * 23 - a19;
    a22 = a21 + a20;
    a23 = a22 & a21;
    a24 = a23 * 29 + a22;
    a25 = a24 - a23;
    a26 = a25 ^ a24;
    a27 = a26 * 31 - a25;
    a28 = a27 + a26;
    a29 = a28 | a27;
    a30 = a29 * 37 + a28;
    a31 = a30 - a29;
    a32 = a31 ^ a30;
    a33 = a32 * 41 - a31;
    a34 = a33 + a32;
    a35 = a34 | a33;
    
    /* Loop with all variables to create spill pressure */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        /* Use asm to prevent dead code elimination */
        asm volatile ("" : "+r" (a1), "+r" (a2), "+r" (a3), "+r" (a4), "+r" (a5));
        asm volatile ("" : "+r" (a6), "+r" (a7), "+r" (a8), "+r" (a9), "+r" (a10));
        
        /* Rotate values to create dependencies */
        int t = a1;
        a1 = a2 + i;
        a2 = a3 ^ t;
        a3 = a4 * a1;
        a4 = a5 - a2;
        a5 = a6 | a3;
        a6 = a7 + a4;
        a7 = a8 ^ a5;
        a8 = a9 * a6;
        a9 = a10 - a7;
        a10 = a11 | a8;
        
        sum += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    }
    
    return sum + a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19 + a20 +
           a21 + a22 + a23 + a24 + a25 + a26 + a27 + a28 + a29 + a30 +
           a31 + a32 + a33 + a34 + a35;
}

/* ============================================
 * PATTERN B: Floating-point intensive computation
 * Pressures floating-point registers
 * ============================================ */
NOINLINE
double pattern_b_float_pressure(double input) {
    /* 20 double variables to pressure FP registers */
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    double d11, d12, d13, d14, d15, d16, d17, d18, d19, d20;
    
    /* Complex FP computation chain */
    d1 = input + 1.0;
    d2 = d1 * 1.6180339887;  /* golden ratio */
    d3 = d2 / d1;
    d4 = d3 * d2 - d1;
    d5 = d4 + d3;
    d6 = d5 * 2.7182818284;  /* e */
    d7 = d6 / d5;
    d8 = d7 * d6 - d4;
    d9 = d8 + d7;
    d10 = d9 * 3.1415926535; /* pi */
    d11 = d10 / d9;
    d12 = d11 * d10 - d8;
    d13 = d12 + d11;
    d14 = d13 * 1.4142135623; /* sqrt(2) */
    d15 = d14 / d13;
    d16 = d15 * d14 - d12;
    d17 = d16 + d15;
    d18 = d17 * 1.7320508075; /* sqrt(3) */
    d19 = d18 / d17;
    d20 = d19 * d18 - d16;
    
    /* Loop with FP operations */
    double result = 0.0;
    for (int i = 0; i < 50; i++) {
        /* Mix all FP variables */
        d1 = d2 * d20 + i;
        d2 = d3 / d1;
        d3 = d4 + d2;
        d4 = d5 - d3;
        d5 = d6 * d4;
        d6 = d7 / d5;
        d7 = d8 + d6;
        d8 = d9 - d7;
        d9 = d10 * d8;
        d10 = d11 / d9;
        d11 = d12 + d10;
        d12 = d13 - d11;
        d13 = d14 * d12;
        d14 = d15 / d13;
        d15 = d16 + d14;
        d16 = d17 - d15;
        d17 = d18 * d16;
        d18 = d19 / d17;
        d19 = d20 + d18;
        d20 = d1 - d19;
        
        result += d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10;
    }
    
    return result;
}

/* ============================================
 * PATTERN C: Complex control flow with switch statement
 * Creates many basic blocks for CFG complexity
 * ============================================ */
NOINLINE
int pattern_c_complex_cfg(int input) {
    int result = input;
    
    /* Nested loops with breaks/continues */
    for (int i = 0; i < 10; i++) {
        if (i % 2 == 0) continue;
        
        for (int j = 0; j < 15; j++) {
            if (j == i) break;
            
            /* Switch with 20+ cases */
            switch ((i * j + input) % 23) {
                case 0:  result += 1; break;
                case 1:  result *= 2; break;
                case 2:  result ^= 3; break;
                case 3:  result |= 4; break;
                case 4:  result &= 5; break;
                case 5:  result -= 6; break;
                case 6:  result += 7; break;
                case 7:  result *= 8; break;
                case 8:  result ^= 9; break;
                case 9:  result |= 10; break;
                case 10: result &= 11; break;
                case 11: result -= 12; break;
                case 12: result += 13; break;
                case 13: result *= 14; break;
                case 14: result ^= 15; break;
                case 15: result |= 16; break;
                case 16: result &= 17; break;
                case 17: result -= 18; break;
                case 18: result += 19; break;
                case 19: result *= 20; break;
                case 20: result ^= 21; break;
                case 21: result |= 22; break;
                case 22: result &= 23; break;
                default: result = ~result; break;
            }
            
            /* Early return in some cases */
            if (result > 1000) {
                return result;
            }
        }
    }
    
    /* Computed goto (GCC extension) for additional CFG complexity */
    void* labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4, &&L5 };
    int idx = result % 6;
    goto *labels[idx];
    
    L0: result += 100; goto END;
    L1: result += 200; goto END;
    L2: result += 300; goto END;
    L3: result += 400; goto END;
    L4: result += 500; goto END;
    L5: result += 600; goto END;
    
    END:
    return result;
}

/* ============================================
 * PATTERN D: Vector/SIMD operations
 * Pressures vector registers
 * ============================================ */
NOINLINE
v4si pattern_d_vector_pressure(v4si input) {
    v4si v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    
    /* Initialize vectors */
    v1 = input + (v4si){1, 2, 3, 4};
    v2 = v1 * (v4si){2, 3, 4, 5};
    v3 = v2 + v1;
    v4 = v3 - v2;
    v5 = v4 * v3;
    v6 = v5 + v4;
    v7 = v6 - v5;
    v8 = v7 * v6;
    v9 = v8 + v7;
    v10 = v9 - v8;
    
    /* Vector loop */
    for (int i = 0; i < 25; i++) {
        v1 = v2 + (v4si){i, i+1, i+2, i+3};
        v2 = v3 * v1;
        v3 = v4 - v2;
        v4 = v5 + v3;
        v5 = v6 * v4;
        v6 = v7 - v5;
        v7 = v8 + v6;
        v8 = v9 * v7;
        v9 = v10 - v8;
        v10 = v1 + v9;
        
        /* Prevent optimization */
        asm volatile ("" : "+x" (v1), "+x" (v2), "+x" (v3));
        asm volatile ("" : "+x" (v4), "+x" (v5), "+x" (v6));
    }
    
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
}

/* ============================================
 * PATTERN E: Explicit register variables
 * Conflicts with allocator's choices
 * ============================================ */
NOINLINE
int pattern_e_explicit_registers(int input) {
    /* Try to bind specific registers */
    register int r12_var asm ("r12") = input;
    register int r13_var asm ("r13") = input * 2;
    register int r14_var asm ("r14") = input * 3;
    register int r15_var asm ("r15") = input * 4;
    
    int local1, local2, local3, local4, local5;
    int local6, local7, local8, local9, local10;
    
    /* Force conflicts */
    local1 = r12_var + 1;
    local2 = r13_var * local1;
    local3 = r14_var ^ local2;
    local4 = r15_var | local3;
    local5 = r12_var - local4;
    local6 = r13_var + local5;
    local7 = r14_var * local6;
    local8 = r15_var ^ local7;
    local9 = r12_var | local8;
    local10 = r13_var - local9;
    
    /* Loop to increase pressure */
    for (int i = 0; i < 20; i++) {
        r12_var = local1 + i;
        r13_var = local2 * r12_var;
        r14_var = local3 ^ r13_var;
        r15_var = local4 | r14_var;
        local1 = local5 - r15_var;
        local2 = local6 + local1;
        local3 = local7 * local2;
        local4 = local8 ^ local3;
        local5 = local9 | local4;
        local6 = local10 - local5;
        
        /* Use explicit registers in asm */
        asm volatile ("" : "+r" (r12_var), "+r" (r13_var));
        asm volatile ("" : "+r" (r14_var), "+r" (r15_var));
    }
    
    return r12_var + r13_var + r14_var + r15_var +
           local1 + local2 + local3 + local4 + local5 +
           local6 + local7 + local8 + local9 + local10;
}

/* ============================================
 * Helper function to use CPU features
 * Ensures target-specific optimizations
 * ============================================ */
NOINLINE
int use_cpu_features(void) {
    int has_avx2 = 0;
    int has_sse4 = 0;
    
    /* Use GCC builtins to check CPU features */
#ifdef __GNUC__
    #ifdef __x86_64__
    has_avx2 = __builtin_cpu_supports("avx2");
    has_sse4 = __builtin_cpu_supports("sse4.2");
    #endif
#endif
    
    return has_avx2 * 2 + has_sse4;
}

/* ============================================
 * Main function - drives all patterns
 * ============================================ */
COLD
int main(int argc, char **argv) {
    int result = 0;
    
    /* Array of inputs to prevent constant folding */
    int inputs[] = {1, 2, 3, 5, 7, 11, 13, 17, 19, 23};
    double dinputs[] = {1.1, 2.2, 3.3, 4.4, 5.5};
    
    /* Call all patterns multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        result ^= pattern_a_int_pressure(inputs[i % 10]);
        
        if (i % 3 == 0) {
            double dres = pattern_b_float_pressure(dinputs[i % 5]);
            result += (int)dres;
        }
        
        if (i % 2 == 0) {
            result += pattern_c_complex_cfg(inputs[i % 10]);
        }
        
        if (i % 4 == 0) {
            v4si vec = {inputs[0], inputs[1], inputs[2], inputs[3]};
            v4si vres = pattern_d_vector_pressure(vec);
            result += vres[0] + vres[1] + vres[2] + vres[3];
        }
        
        if (i % 5 == 0) {
            result |= pattern_e_explicit_registers(inputs[i % 10]);
        }
    }
    
    /* Use CPU features to engage target-specific passes */
    result += use_cpu_features();
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result != 0;
}
