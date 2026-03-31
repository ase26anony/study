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

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))
#define NORETURN __attribute__((noreturn))
#define COLD __attribute__((cold))

/* Vector type for SIMD pressure */
typedef int v4si __attribute__((vector_size(16)));

/* Stub for non-GCC compilers */
#ifndef __GNUC__
#define __builtin_cpu_supports(x) 0
#endif

/* ============================================================
 * PATTERN A: Integer arithmetic chain with 30+ variables
 * Creates massive register pressure in a loop
 * ============================================================ */
NOINLINE static int pattern_a_int_pressure(int input) {
    /* 30+ integer variables to pressure general purpose registers */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int a11, a12, a13, a14, a15, a16, a17, a18, a19, a20;
    int a21, a22, a23, a24, a25, a26, a27, a28, a29, a30;
    int sum = 0;
    
    /* Initialize with input to prevent constant folding */
    a1 = input + 1;
    a2 = a1 * 2;
    a3 = a2 - input;
    a4 = a3 ^ a1;
    a5 = a4 | a2;
    a6 = a5 & a3;
    a7 = a6 + a4;
    a8 = a7 - a5;
    a9 = a8 * a6;
    a10 = a9 / (a7 ? a7 : 1);
    
    a11 = a10 << 2;
    a12 = a11 >> 1;
    a13 = a12 + a10;
    a14 = a13 - a11;
    a15 = a14 * a12;
    a16 = a15 ^ a13;
    a17 = a16 | a14;
    a18 = a17 & a15;
    a19 = a18 + a16;
    a20 = a19 - a17;
    
    a21 = a20 * a18;
    a22 = a21 / (a19 ? a19 : 1);
    a23 = a22 << 3;
    a24 = a23 >> 2;
    a25 = a24 + a22;
    a26 = a25 - a23;
    a27 = a26 * a24;
    a28 = a27 ^ a25;
    a29 = a28 | a26;
    a30 = a29 & a27;
    
    /* Complex loop with interdependencies */
    for (int i = 0; i < 100; i++) {
        a1 = a30 + i;
        a2 = a1 * a29;
        a3 = a2 - a28;
        a4 = a3 ^ a27;
        a5 = a4 | a26;
        a6 = a5 & a25;
        a7 = a6 + a24;
        a8 = a7 - a23;
        a9 = a8 * a22;
        a10 = a9 / (a21 ? a21 : 1);
        
        /* Force all variables to be live */
        asm volatile("" : "+r"(a1), "+r"(a2), "+r"(a3), "+r"(a4), "+r"(a5));
        asm volatile("" : "+r"(a6), "+r"(a7), "+r"(a8), "+r"(a9), "+r"(a10));
        
        sum += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    }
    
    /* Use all variables to prevent elimination */
    return sum + a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19 + a20
           + a21 + a22 + a23 + a24 + a25 + a26 + a27 + a28 + a29 + a30;
}

/* ============================================================
 * PATTERN B: Floating-point intensive computation
 * Pressures FP registers with double variables
 * ============================================================ */
NOINLINE static double pattern_b_fp_pressure(double input) {
    /* 20+ double variables to pressure FP registers */
    double b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    double b11, b12, b13, b14, b15, b16, b17, b18, b19, b20;
    double result = 0.0;
    
    b1 = input + 1.0;
    b2 = b1 * 2.0;
    b3 = b2 - input;
    b4 = b3 / b1;
    b5 = b4 * b2;
    b6 = b5 - b3;
    b7 = b6 + b4;
    b8 = b7 * b5;
    b9 = b8 / b6;
    b10 = b9 - b7;
    
    b11 = b10 * 3.14159;
    b12 = b11 / 2.71828;
    b13 = b12 + b10;
    b14 = b13 - b11;
    b15 = b14 * b12;
    b16 = b15 / b13;
    b17 = b16 - b14;
    b18 = b17 + b15;
    b19 = b18 * b16;
    b20 = b19 / b17;
    
    /* Nested loops with FP operations */
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 10; j++) {
            b1 = b20 + (double)i;
            b2 = b1 * b19;
            b3 = b2 - b18;
            b4 = b3 / b17;
            b5 = b4 * b16;
            
            /* Mix control flow to create complex CFG */
            if (j % 3 == 0) {
                b6 = b5 + b15;
            } else if (j % 3 == 1) {
                b6 = b5 - b15;
            } else {
                b6 = b5 * b15;
            }
            
            /* Prevent dead code elimination */
            asm volatile("" : "+f"(b1), "+f"(b2), "+f"(b3));
            asm volatile("" : "+f"(b4), "+f"(b5), "+f"(b6));
            
            result += b1 + b2 + b3 + b4 + b5 + b6;
        }
    }
    
    return result + b7 + b8 + b9 + b10 + b11 + b12 + b13 + b14 + b15 
           + b16 + b17 + b18 + b19 + b20;
}

/* ============================================================
 * PATTERN C: Complex control flow with switch statement
 * Creates many basic blocks for CFG complexity
 * ============================================================ */
NOINLINE static int pattern_c_cfg_complexity(int input) {
    int result = input;
    
    /* Switch with 20+ cases to create many basic blocks */
    for (int i = 0; i < 100; i++) {
        switch (i % 23) {
            case 0: result += i * 2; break;
            case 1: result ^= i; break;
            case 2: result |= i << 1; break;
            case 3: result &= ~i; break;
            case 4: result = result * 3 + i; break;
            case 5: result = result / 2 - i; break;
            case 6: result = (result << 3) | i; break;
            case 7: result = (result >> 2) ^ i; break;
            case 8: result += result % (i + 1); break;
            case 9: result -= i * result; break;
            case 10: result = ~result + i; break;
            case 11: result = result * result - i; break;
            case 12: result = result | (i << 4); break;
            case 13: result = result & (0xFFF0 + i); break;
            case 14: result = result ^ (0xABCD ^ i); break;
            case 15: result = result + (i << 8); break;
            case 16: result = result - (i * 16); break;
            case 17: result = result * (i % 7 + 1); break;
            case 18: result = result / ((i % 5) + 1); break;
            case 19: result = result << (i % 4); break;
            case 20: result = result >> ((i % 3) + 1); break;
            case 21: result = result % ((i % 9) + 1); break;
            case 22: result = -result + i; break;
            default: result += 1; break;
        }
        
        /* Additional control flow with break/continue */
        if (i % 7 == 0) {
            continue;
        }
        if (i % 13 == 0) {
            result *= 2;
            break;
        }
    }
    
    return result;
}

/* ============================================================
 * PATTERN D: SIMD vector operations
 * Pressures vector/SIMD registers
 * ============================================================ */
NOINLINE static v4si pattern_d_simd_pressure(v4si input) {
    v4si v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    v4si result = {0, 0, 0, 0};
    
    v1 = input + (v4si){1, 2, 3, 4};
    v2 = v1 * (v4si){2, 3, 4, 5};
    v3 = v2 - input;
    v4 = v3 / (v4si){1, 1, 2, 2};
    v5 = v4 * v2;
    v6 = v5 - v3;
    v7 = v6 + v4;
    v8 = v7 * v5;
    v9 = v8 / (v4si){2, 2, 3, 3};
    v10 = v9 - v7;
    
    /* Loop with vector operations */
    for (int i = 0; i < 50; i++) {
        v1 = v10 + (v4si){i, i+1, i+2, i+3};
        v2 = v1 * v9;
        v3 = v2 - v8;
        v4 = v3 + v7;
        v5 = v4 * v6;
        
        /* Mix with scalar to force moves between register classes */
        int scalar = i * 3;
        v4si temp = v5 + (v4si){scalar, scalar+1, scalar+2, scalar+3};
        
        /* Prevent elimination */
        asm volatile("" : "+x"(v1), "+x"(v2), "+x"(v3));
        asm volatile("" : "+x"(v4), "+x"(v5), "+x"(temp));
        
        result = result + v1 + v2 + v3 + v4 + v5 + temp;
    }
    
    return result;
}

/* ============================================================
 * PATTERN E: Explicit register variables
 * Conflicts with allocator's choices
 * ============================================================ */
NOINLINE static int pattern_e_explicit_registers(int input) {
    /* Explicit register variables that conflict with allocator */
    register int r1 asm ("r12") = input + 1;
    register int r2 asm ("r13") = r1 * 2;
    register int r3 asm ("r14") = r2 - input;
    register int r4 asm ("r15") = r3 ^ r1;
    
    int local1, local2, local3, local4, local5;
    int local6, local7, local8, local9, local10;
    
    /* Force interaction between explicit and implicit registers */
    local1 = r4 + 1;
    local2 = local1 * r3;
    local3 = local2 - r2;
    local4 = local3 ^ r1;
    local5 = local4 | r4;
    
    /* Complex loop with register pressure */
    for (int i = 0; i < 100; i++) {
        r1 = local5 + i;
        r2 = r1 * local4;
        r3 = r2 - local3;
        r4 = r3 ^ local2;
        
        local6 = r4 + local1;
        local7 = local6 * r3;
        local8 = local7 - r2;
        local9 = local8 ^ r1;
        local10 = local9 | r4;
        
        /* Force all to be live */
        asm volatile("" : "+r"(r1), "+r"(r2), "+r"(r3), "+r"(r4));
        asm volatile("" : "+r"(local6), "+r"(local7), "+r"(local8));
        asm volatile("" : "+r"(local9), "+r"(local10));
        
        /* Mix with memory operations to force spills */
        local1 = local10;
        local2 = local9;
        local3 = local8;
        local4 = local7;
        local5 = local6;
    }
    
    return r1 + r2 + r3 + r4 + local1 + local2 + local3 + local4 + local5
           + local6 + local7 + local8 + local9 + local10;
}

/* ============================================================
 * Main function - cold attribute may affect block ordering
 * ============================================================ */
COLD int main(int argc, char **argv) {
    int int_result = 0;
    double fp_result = 0.0;
    v4si vec_result;
    int switch_result = 0;
    int explicit_result = 0;
    
    /* Use CPU feature detection to engage target-specific optimizations */
    int use_avx2 = __builtin_cpu_supports("avx2");
    int use_sse4 = __builtin_cpu_supports("sse4.2");
    
    /* Varying inputs to prevent constant folding */
    int inputs[] = {1, 2, 3, 5, 7, 11, 13, 17, 19, 23};
    
    for (int i = 0; i < 10; i++) {
        /* Call all patterns with different inputs */
        int_result += pattern_a_int_pressure(inputs[i] + i);
        fp_result += pattern_b_fp_pressure((double)inputs[i] * 0.5);
        switch_result += pattern_c_cfg_complexity(inputs[i] * 3);
        
        v4si vec_input = {inputs[i], inputs[i]+1, inputs[i]+2, inputs[i]+3};
        vec_result = pattern_d_simd_pressure(vec_input);
        int_result += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
        
        explicit_result += pattern_e_explicit_registers(inputs[i] * 2);
        
        /* Mix with conditional based on CPU features */
        if (use_avx2) {
            int_result *= 2;
        }
        if (use_sse4) {
            fp_result += 1.0;
        }
    }
    
    /* Prevent dead code elimination of results */
    asm volatile("" : "+r"(int_result), "+r"(switch_result), "+r"(explicit_result));
    asm volatile("" : "+f"(fp_result));
    
    /* Optional debug output - doesn't affect coverage collection */
    if (argc > 1) {
        printf("Results: int=%d, fp=%f, switch=%d, explicit=%d\n",
               int_result, fp_result, switch_result, explicit_result);
    }
    
    return 0;
}
