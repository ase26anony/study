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

/* Stub for non-GCC compilers */
#ifndef __GNUC__
#define __builtin_cpu_supports(x) 0
#endif

/* ============================================================
 * PATTERN A: Integer arithmetic chain with 30+ variables
 * Creates massive register pressure in a tight loop
 * ============================================================ */
NOINLINE int pattern_a_intensive(int input) {
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
    
    /* Complex loop with interdependencies */
    for (int i = 0; i < 100; i++) {
        a1 = a2 + i;
        a2 = a3 - i;
        a3 = a4 * a1;
        a4 = a5 ^ a2;
        a5 = a6 | a3;
        a6 = a7 & a4;
        a7 = a8 << (i & 3);
        a8 = a9 >> 1;
        a9 = a10 + a5;
        a10 = a11 - a6;
        
        /* Force spill decisions with many live variables */
        a11 = a12 * a7;
        a12 = a13 ^ a8;
        a13 = a14 | a9;
        a14 = a15 & a10;
        a15 = a16 << 2;
        a16 = a17 >> 1;
        a17 = a18 + a11;
        a18 = a19 - a12;
        a19 = a20 * a13;
        a20 = a21 ^ a14;
        
        /* Prevent dead code elimination */
        asm volatile("" : "+r"(a1), "+r"(a2), "+r"(a3), "+r"(a4), "+r"(a5));
        asm volatile("" : "+r"(a6), "+r"(a7), "+r"(a8), "+r"(a9), "+r"(a10));
    }
    
    /* Return a complex expression to keep all variables live */
    return a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
           a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19 + a20 +
           a21 + a22 + a23 + a24 + a25 + a26 + a27 + a28 + a29 + a30 +
           a31 + a32 + a33 + a34 + a35 + a36 + a37 + a38 + a39 + a40;
}

/* ============================================================
 * PATTERN B: Floating-point intensive computation
 * Pressures floating-point and vector registers
 * ============================================================ */
NOINLINE double pattern_b_fp_intensive(double input) {
    /* 20+ double variables to pressure FP registers */
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    double d11, d12, d13, d14, d15, d16, d17, d18, d19, d20;
    double d21, d22, d23, d24, d25;
    
    d1 = input;
    d2 = d1 * 1.1;
    d3 = d2 + 2.2;
    d4 = d3 - d2;
    d5 = d4 * d3;
    d6 = d5 / d4;
    d7 = d6 + 3.3;
    d8 = d7 - 4.4;
    d9 = d8 * 5.5;
    d10 = d9 / 6.6;
    
    d11 = d10 + d9;
    d12 = d11 - d10;
    d13 = d12 * d11;
    d14 = d13 / d12;
    d15 = d14 + 7.7;
    d16 = d15 - 8.8;
    d17 = d16 * 9.9;
    d18 = d17 / 10.1;
    d19 = d18 + d17;
    d20 = d19 - d18;
    
    d21 = d20 * d19;
    d22 = d21 / d20;
    d23 = d22 + 11.11;
    d24 = d23 - 12.12;
    d25 = d24 * 13.13;
    
    /* Nested loops with FP operations */
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 10; j++) {
            d1 = d2 * (i + 1);
            d2 = d3 / (j + 1);
            d3 = d4 + d1;
            d4 = d5 - d2;
            d5 = d6 * d3;
            d6 = d7 / d4;
            
            /* Mix integer and FP to create conversion pressure */
            int temp = i * j;
            d7 = d8 + temp;
            d8 = d9 - temp;
            d9 = d10 * (temp + 1);
            
            /* Complex control flow within loops */
            if (i > j) {
                d10 = d11 * 2.0;
                d11 = d12 / 2.0;
            } else {
                d10 = d11 / 3.0;
                d11 = d12 * 3.0;
            }
            
            /* Prevent elimination */
            asm volatile("" : "+f"(d1), "+f"(d2), "+f"(d3));
        }
        
        /* Break/continue to create CFG complexity */
        if (i % 7 == 0) continue;
        if (i % 13 == 0) break;
    }
    
    return d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10 +
           d11 + d12 + d13 + d14 + d15 + d16 + d17 + d18 + d19 + d20 +
           d21 + d22 + d23 + d24 + d25;
}

/* ============================================================
 * PATTERN C: Complex control flow with switch statement
 * Creates many basic blocks for CFG complexity
 * ============================================================ */
NOINLINE int pattern_c_switch_complex(int input, int mode) {
    int result = input;
    
    /* Switch with 20+ cases - each creates a basic block */
    switch (mode % 23) {
        case 0:
            result = result * 2;
            /* Fall through */
        case 1:
            result = result + 1;
            break;
        case 2:
            result = result ^ 0xFF;
            break;
        case 3:
            result = result << 3;
            break;
        case 4:
            result = result >> 2;
            break;
        case 5:
            result = result | 0xAA;
            break;
        case 6:
            result = result & 0x55;
            break;
        case 7:
            result = result % 17;
            break;
        case 8:
            result = result * 3;
            /* Fall through */
        case 9:
            result = result - 5;
            break;
        case 10:
            result = result + 10;
            break;
        case 11:
            result = result ^ 0xCC;
            break;
        case 12:
            result = result << 1;
            break;
        case 13:
            result = result >> 4;
            break;
        case 14:
            result = result | 0x33;
            break;
        case 15:
            result = result & 0x66;
            break;
        case 16:
            result = result % 19;
            break;
        case 17:
            result = result * 5;
            break;
        case 18:
            result = result - 7;
            break;
        case 19:
            result = result + 15;
            break;
        case 20:
            result = result ^ 0x99;
            break;
        case 21:
            result = result << 2;
            break;
        case 22:
            result = result >> 3;
            break;
        default:
            result = 0;
    }
    
    /* Loop with switch inside - creates complex CFG */
    for (int i = 0; i < 100; i++) {
        int temp = result + i;
        
        /* Another switch inside loop */
        switch (temp % 5) {
            case 0:
                result += i * 2;
                continue;  /* CFG edge */
            case 1:
                result -= i;
                break;
            case 2:
                result *= i;
                if (result > 1000) break;
                /* else fall through */
            case 3:
                result /= (i + 1);
                break;
            case 4:
                result ^= i;
                if (i % 2 == 0) continue;
                break;
        }
        
        /* Additional control flow */
        if (i % 3 == 0) {
            result += 100;
        } else if (i % 3 == 1) {
            result -= 50;
        } else {
            result *= 2;
        }
    }
    
    return result;
}

/* ============================================================
 * PATTERN D: Vector/SIMD operations
 * Pressures vector registers and creates vector spill decisions
 * ============================================================ */
NOINLINE v4si pattern_d_vector_ops(v4si input) {
    v4si v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    v4si v11, v12, v13, v14, v15;
    
    /* Initialize vectors */
    v1 = input;
    v2 = v1 + (v4si){1, 2, 3, 4};
    v3 = v2 * (v4si){2, 2, 2, 2};
    v4 = v3 - v2;
    v5 = v4 & (v4si){0xFF, 0xFF, 0xFF, 0xFF};
    v6 = v5 | (v4si){0xAA, 0xAA, 0xAA, 0xAA};
    v7 = v6 ^ (v4si){0x55, 0x55, 0x55, 0x55};
    v8 = v7 << (v4si){1, 2, 3, 4};
    v9 = v8 >> (v4si){4, 3, 2, 1};
    v10 = v9 + v8;
    
    v11 = v10 * v9;
    v12 = v11 - v10;
    v13 = v12 & v11;
    v14 = v13 | v12;
    v15 = v14 ^ v13;
    
    /* Vector loop */
    for (int i = 0; i < 50; i++) {
        v4si idx = (v4si){i, i+1, i+2, i+3};
        
        v1 = v2 + idx;
        v2 = v3 * (v4si){i, i, i, i};
        v3 = v4 - idx;
        v4 = v5 & (v4si){i, i, i, i};
        v5 = v6 | idx;
        
        /* Conditional vector operations */
        if (i % 2 == 0) {
            v6 = v7 << (v4si){1, 1, 1, 1};
            v7 = v8 >> (v4si){2, 2, 2, 2};
        } else {
            v6 = v7 + v8;
            v7 = v8 - v9;
        }
        
        v8 = v9 * v10;
        v9 = v10 + v11;
        v10 = v11 - v12;
        
        /* Prevent elimination */
        asm volatile("" : "+x"(v1), "+x"(v2), "+x"(v3));
    }
    
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15;
}

/* ============================================================
 * PATTERN E: Explicit register variables
 * Conflicts with allocator's choices, prompting fixup
 * ============================================================ */
NOINLINE int pattern_e_explicit_registers(int input) {
    /* Explicit register variables - may conflict with allocator */
    register int r1 asm ("r12") = input;
    register int r2 asm ("r13") = input + 1;
    register int r3 asm ("r14") = input * 2;
    register int r4 asm ("r15") = input - 1;
    
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    
    /* Mix explicit and regular variables */
    a1 = r1 + r2;
    a2 = r3 - r4;
    a3 = a1 * a2;
    a4 = a3 / (r1 + 1);
    a5 = a4 ^ r2;
    a6 = a5 | r3;
    a7 = a6 & r4;
    a8 = a7 << r1;
    a9 = a8 >> r2;
    a10 = a9 + a8;
    
    /* Computed goto - GCC extension that creates complex CFG */
    void* labels[] = {&&L0, &&L1, &&L2, &&L3, &&L4, &&L5, &&L6, &&L7};
    
    int i = input % 8;
    goto *labels[i];
    
L0:
    r1 = a1 + 1;
    goto end;
L1:
    r2 = a2 - 1;
    goto end;
L2:
    r3 = a3 * 2;
    goto end;
L3:
    r4 = a4 / 2;
    goto end;
L4:
    r1 = a5 ^ 0xFF;
    goto end;
L5:
    r2 = a6 | 0xAA;
    goto end;
L6:
    r3 = a7 & 0x55;
    goto end;
L7:
    r4 = a8 << 2;
    /* fall through */
    
end:
    /* Force all registers to be used */
    asm volatile("" : : "r"(r1), "r"(r2), "r"(r3), "r"(r4));
    
    return r1 + r2 + r3 + r4 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
}

/* ============================================================
 * Main function - calls all patterns with varying inputs
 * ============================================================ */
COLD int main(int argc, char** argv) {
    int result = 0;
    
    /* Use CPU feature check to engage target-specific optimizations */
    if (__builtin_cpu_supports("avx2")) {
        /* This may affect register allocation decisions */
        result += 1;
    }
    
    /* Array of inputs to prevent constant folding */
    int inputs[] = {1, 2, 3, 5, 7, 11, 13, 17, 19, 23};
    double fp_inputs[] = {1.1, 2.2, 3.3, 5.5, 7.7, 11.11};
    
    /* Call each pattern multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        result ^= pattern_a_intensive(inputs[i % 10]);
        
        if (i < 6) {
            double fp_res = pattern_b_fp_intensive(fp_inputs[i]);
            result += (int)fp_res;
        }
        
        result += pattern_c_switch_complex(inputs[i % 10], i);
        
        if (i % 3 == 0) {
            v4si vec_input = {i, i+1, i+2, i+3};
            v4si vec_res = pattern_d_vector_ops(vec_input);
            result += vec_res[0] + vec_res[1] + vec_res[2] + vec_res[3];
        }
        
        result += pattern_e_explicit_registers(inputs[i % 10]);
    }
    
    /* Optional: Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result % 256;
}
