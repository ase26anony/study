/* test_mcf_coverage.c
 * 
 * This test program is designed to trigger GCC's Min-Cost Flow pass
 * during register allocation, specifically to exercise the print_node
 * function with special node indices (ENTRY_BLOCK, EXIT_BLOCK,
 * new_exit_index, new_entry_index).
 *
 * Compile with: gcc -O3 -fprofile-arcs -ftest-coverage -fdump-rtl-all -c test_mcf_coverage.c
 * Then run: ./test_mcf_coverage (to generate profile data)
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units for each pressure function */
#define NOINLINE __attribute__((noinline))
#define COLD __attribute__((cold))

/* Vector type for SIMD pressure */
typedef int v4si __attribute__((vector_size(16)));

/* ============================================================
 * PATTERN A: Integer arithmetic chain with 30+ variables
 * Creates massive register pressure in a tight loop
 * ============================================================ */
NOINLINE int pattern_a_int_pressure(int input) {
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
    a11 = a10 ^ a8;
    a12 = a11 | a9;
    a13 = a12 & a10;
    a14 = a13 + a11;
    a15 = a14 - a12;
    a16 = a15 * a13;
    a17 = a16 / (a14 ? a14 : 1);
    a18 = a17 ^ a15;
    a19 = a18 | a16;
    a20 = a19 & a17;
    a21 = a20 + a18;
    a22 = a21 - a19;
    a23 = a22 * a20;
    a24 = a23 / (a21 ? a21 : 1);
    a25 = a24 ^ a22;
    a26 = a25 | a23;
    a27 = a26 & a24;
    a28 = a27 + a25;
    a29 = a28 - a26;
    a30 = a29 * a27;
    
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
        
        /* Force spill decisions with many live variables */
        sum += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
        
        /* Rotate values to keep all variables live */
        int tmp = a30;
        a30 = a29; a29 = a28; a28 = a27; a27 = a26;
        a26 = a25; a25 = a24; a24 = a23; a23 = a22;
        a22 = a21; a21 = a20; a20 = a19; a19 = a18;
        a18 = a17; a17 = a16; a16 = a15; a15 = a14;
        a14 = a13; a13 = a12; a12 = a11; a11 = a10;
        a10 = a9; a9 = a8; a8 = a7; a7 = a6;
        a6 = a5; a5 = a4; a4 = a3; a3 = a2;
        a2 = a1; a1 = tmp + i;
    }
    
    /* Use all variables in final computation */
    return sum + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
           a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19 + a20 +
           a21 + a22 + a23 + a24 + a25 + a26 + a27 + a28 + a29 + a30;
}

/* ============================================================
 * PATTERN B: Floating-point intensive computation
 * Pressures floating-point and SIMD registers
 * ============================================================ */
NOINLINE double pattern_b_fp_pressure(double input) {
    /* 20+ double variables */
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    double d11, d12, d13, d14, d15, d16, d17, d18, d19, d20;
    double result = 0.0;
    
    d1 = input + 1.0;
    d2 = d1 * 1.1;
    d3 = d2 - 0.5;
    d4 = d3 / 2.0;
    d5 = d4 * d1;
    d6 = d5 + d2;
    d7 = d6 - d3;
    d8 = d7 * d4;
    d9 = d8 / d5;
    d10 = d9 + d6;
    d11 = d10 - d7;
    d12 = d11 * d8;
    d13 = d12 / d9;
    d14 = d13 + d10;
    d15 = d14 - d11;
    d16 = d15 * d12;
    d17 = d16 / d13;
    d18 = d17 + d14;
    d19 = d18 - d15;
    d20 = d19 * d16;
    
    /* Loop with FP operations */
    for (int i = 0; i < 50; i++) {
        double t = (double)i * 0.01;
        d1 = d20 * t;
        d2 = d1 + d19;
        d3 = d2 - d18;
        d4 = d3 * d17;
        d5 = d4 / (d16 + 0.001);
        d6 = d5 + d15;
        d7 = d6 - d14;
        d8 = d7 * d13;
        d9 = d8 / (d12 + 0.001);
        d10 = d9 + d11;
        
        /* Complex FP expression */
        result += d1 * d2 - d3 / d4 + d5 * d6 - d7 / d8 + d9 * d10;
        
        /* Rotate to keep pressure */
        double tmp = d20;
        d20 = d19; d19 = d18; d18 = d17; d17 = d16;
        d16 = d15; d15 = d14; d14 = d13; d13 = d12;
        d12 = d11; d11 = d10; d10 = d9; d9 = d8;
        d8 = d7; d7 = d6; d6 = d5; d5 = d4;
        d4 = d3; d3 = d2; d2 = d1; d1 = tmp * t;
    }
    
    return result + d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10 +
           d11 + d12 + d13 + d14 + d15 + d16 + d17 + d18 + d19 + d20;
}

/* ============================================================
 * PATTERN C: Complex control flow with switch statement
 * Creates many basic blocks for CFG complexity
 * ============================================================ */
NOINLINE int pattern_c_cfg_complexity(int input) {
    int result = input;
    
    /* Outer loop with switch inside */
    for (int i = 0; i < 100; i++) {
        switch (i % 23) {  /* 23 cases to create many basic blocks */
            case 0: result += i * 2; break;
            case 1: result ^= i; result *= 3; break;
            case 2: result = (result << 1) | (result >> 31); break;
            case 3: result += result % 17; break;
            case 4: result ^= 0xAAAAAAAA; break;
            case 5: result = result * 7 + 1; break;
            case 6: result = ~result; break;
            case 7: result += i * i; break;
            case 8: result = result / (i % 5 + 1); break;
            case 9: result |= 0x55555555; break;
            case 10: result = result - i + 42; break;
            case 11: result = (result & 0xFF) << 24; break;
            case 12: result += result >> 2; break;
            case 13: result = result * 11 % 256; break;
            case 14: result ^= result << 3; break;
            case 15: result = result + (i << 8); break;
            case 16: result = result & 0x0F0F0F0F; break;
            case 17: result = result * 13 / 7; break;
            case 18: result = result | i; break;
            case 19: result = result - (result % 11); break;
            case 20: result ^= 0x12345678; break;
            case 21: result = result << 4 | result >> 28; break;
            case 22: result = result + (i % 3) * 100; break;
            default: result += 1; /* Should never hit */
        }
        
        /* Nested loop with break/continue */
        for (int j = 0; j < 10; j++) {
            if (j == i % 7) {
                result += j * 100;
                continue;
            }
            if (j == 5 && (i % 3 == 0)) {
                result -= 50;
                break;
            }
            result += j;
        }
    }
    
    return result;
}

/* ============================================================
 * PATTERN D: Vector/SIMD operations
 * Pressures vector registers
 * ============================================================ */
NOINLINE v4si pattern_d_vector_pressure(v4si input) {
    v4si v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    v4si result = {0, 0, 0, 0};
    
    /* Initialize vectors */
    v1 = input + (v4si){1, 2, 3, 4};
    v2 = v1 * (v4si){2, 2, 2, 2};
    v3 = v2 - (v4si){1, 1, 1, 1};
    v4 = v3 & (v4si){0xFF, 0xFF, 0xFF, 0xFF};
    v5 = v4 | (v4si){0x80, 0x80, 0x80, 0x80};
    v6 = v5 ^ (v4si){0x55, 0x55, 0x55, 0x55};
    v7 = v6 + v1;
    v8 = v7 - v2;
    v9 = v8 * v3;
    v10 = v9 / (v4si){2, 2, 2, 2};
    
    /* Vector loop */
    for (int i = 0; i < 40; i++) {
        v4si idx = {i, i+1, i+2, i+3};
        v1 = v10 + idx;
        v2 = v1 * v9;
        v3 = v2 - v8;
        v4 = v3 & v7;
        v5 = v4 | v6;
        v6 = v5 ^ v1;
        v7 = v6 + v2;
        v8 = v7 - v3;
        v9 = v8 * v4;
        v10 = v9 / (v4si){i+1, i+2, i+3, i+4};
        
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        
        /* Rotate vectors */
        v4si tmp = v10;
        v10 = v9; v9 = v8; v8 = v7; v7 = v6;
        v6 = v5; v5 = v4; v4 = v3; v3 = v2;
        v2 = v1; v1 = tmp + idx;
    }
    
    return result;
}

/* ============================================================
 * PATTERN E: Explicit register variables
 * Conflicts with allocator's choices
 * ============================================================ */
NOINLINE int pattern_e_register_conflict(int input) {
    /* Try to use specific registers */
    register int r12_var asm ("r12") = input;
    register int r13_var asm ("r13") = input * 2;
    register int r14_var asm ("r14") = input + 1;
    register int r15_var asm ("r15") = input - 1;
    
    int local1, local2, local3, local4, local5;
    int local6, local7, local8, local9, local10;
    
    /* Force interactions between register vars and locals */
    local1 = r12_var * 3;
    local2 = r13_var + local1;
    local3 = r14_var - local2;
    local4 = r15_var ^ local3;
    local5 = local1 | local4;
    
    /* Loop that uses all variables */
    for (int i = 0; i < 30; i++) {
        r12_var += local1 + i;
        r13_var ^= local2 * i;
        r14_var -= local3 / (i + 1);
        r15_var |= local4 << (i % 4);
        
        local6 = r12_var + r13_var;
        local7 = r14_var - r15_var;
        local8 = local6 * local7;
        local9 = local8 / (local5 + 1);
        local10 = local9 ^ local6;
        
        /* Force all to be live */
        asm volatile ("" : : "r"(r12_var), "r"(r13_var), "r"(r14_var), "r"(r15_var));
        asm volatile ("" : : "r"(local1), "r"(local2), "r"(local3), "r"(local4), "r"(local5));
        asm volatile ("" : : "r"(local6), "r"(local7), "r"(local8), "r"(local9), "r"(local10));
        
        /* Rotate */
        int tmp = local10;
        local10 = local9; local9 = local8; local8 = local7; local7 = local6;
        local6 = local5; local5 = local4; local4 = local3; local3 = local2;
        local2 = local1; local1 = tmp + i;
    }
    
    return r12_var + r13_var + r14_var + r15_var +
           local1 + local2 + local3 + local4 + local5 +
           local6 + local7 + local8 + local9 + local10;
}

/* ============================================================
 * Helper to prevent dead code elimination
 * ============================================================ */
static void use_result(int result) {
    /* Use inline asm to prevent optimization */
    asm volatile ("" : : "r"(result));
}

/* ============================================================
 * Main function - drives all patterns
 * ============================================================ */
COLD int main(int argc, char **argv) {
    int total = 0;
    
    /* Use CPU feature check to engage target-specific optimizations */
    #ifdef __GNUC__
    if (__builtin_cpu_supports("avx2")) {
        /* This may affect register allocation decisions */
        total += 1;
    }
    #endif
    
    /* Array of inputs to prevent constant folding */
    int inputs[] = {1, 2, 3, 5, 7, 11, 13, 17, 19, 23};
    
    /* Execute each pattern multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        int input = inputs[i % 10];
        
        /* Pattern A: Integer pressure */
        int result_a = pattern_a_int_pressure(input);
        use_result(result_a);
        total += result_a % 1000;
        
        /* Pattern B: Floating-point pressure */
        double result_b = pattern_b_fp_pressure((double)input);
        use_result((int)result_b);
        total += (int)result_b % 1000;
        
        /* Pattern C: CFG complexity */
        int result_c = pattern_c_cfg_complexity(input);
        use_result(result_c);
        total += result_c % 1000;
        
        /* Pattern D: Vector pressure */
        v4si vec_input = {input, input+1, input+2, input+3};
        v4si result_d = pattern_d_vector_pressure(vec_input);
        int sum_d = result_d[0] + result_d[1] + result_d[2] + result_d[3];
        use_result(sum_d);
        total += sum_d % 1000;
        
        /* Pattern E: Register conflict */
        int result_e = pattern_e_register_conflict(input);
        use_result(result_e);
        total += result_e % 1000;
    }
    
    /* Simple output to verify execution */
    #ifdef DEBUG_OUTPUT
    printf("Total checksum: %d\n", total);
    #endif
    
    return total == 0 ? 0 : 1;
}
