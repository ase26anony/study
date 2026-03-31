/* test_mcf.c - Comprehensive test for GCC Min-Cost Flow register allocator
 * Designed to trigger print_node with special indices: ENTRY_BLOCK, EXIT_BLOCK,
 * new_exit_index, and new_entry_index from the fixup graph.
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))
#define COLD __attribute__((cold))

/* Vector type for SIMD pressure */
typedef int v4si __attribute__((vector_size(16)));

/* ============================================
 * Pattern A: Integer arithmetic chain with 30+ variables
 * Creates massive register pressure in a tight loop
 * ============================================ */
NOINLINE int pattern_a_int_pressure(int input) {
    /* 30+ integer variables to pressure general purpose registers */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int a11, a12, a13, a14, a15, a16, a17, a18, a19, a20;
    int a21, a22, a23, a24, a25, a26, a27, a28, a29, a30;
    int a31, a32, a33, a34, a35, a36, a37, a38, a39, a40;
    
    /* Complex initialization chain */
    a1 = input + 1;
    a2 = a1 * 2;
    a3 = a2 - a1;
    a4 = a3 + input;
    a5 = a4 * a3;
    a6 = a5 / (a1 ? a1 : 1);
    a7 = a6 ^ a5;
    a8 = a7 | a6;
    a9 = a8 & a7;
    a10 = a9 << 2;
    
    a11 = a10 >> 1;
    a12 = a11 + a10;
    a13 = a12 - a11;
    a14 = a13 * a12;
    a15 = a14 % (a13 ? a13 : 1);
    a16 = a15 ^ a14;
    a17 = a16 | a15;
    a18 = a17 & a16;
    a19 = a18 << 3;
    a20 = a19 >> 2;
    
    a21 = a20 + a19;
    a22 = a21 - a20;
    a23 = a22 * a21;
    a24 = a23 / (a22 ? a22 : 1);
    a25 = a24 ^ a23;
    a26 = a25 | a24;
    a27 = a26 & a25;
    a28 = a27 << 4;
    a29 = a28 >> 3;
    a30 = a29 + a28;
    
    a31 = a30 - a29;
    a32 = a31 * a30;
    a33 = a32 % (a31 ? a31 : 1);
    a34 = a33 ^ a32;
    a35 = a34 | a33;
    a36 = a35 & a34;
    a37 = a36 << 5;
    a38 = a37 >> 4;
    a39 = a38 + a37;
    a40 = a39 - a38;
    
    /* Tight interdependent loop to force spill decisions */
    int sum = 0;
    for (int i = 0; i < 1000; i++) {
        a1 = a40 + i;
        a2 = a1 * a39;
        a3 = a2 - a38;
        a4 = a3 + a37;
        a5 = a4 * a36;
        a6 = a5 / (a35 ? a35 : 1);
        a7 = a6 ^ a34;
        a8 = a7 | a33;
        a9 = a8 & a32;
        a10 = a9 << (i & 3);
        
        a11 = a10 >> 1;
        a12 = a11 + a31;
        a13 = a12 - a30;
        a14 = a13 * a29;
        a15 = a14 % (a28 ? a28 : 1);
        a16 = a15 ^ a27;
        a17 = a16 | a26;
        a18 = a17 & a25;
        a19 = a18 << 2;
        a20 = a19 >> 1;
        
        sum += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
               a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19 + a20;
        
        /* Prevent loop invariant code motion */
        asm volatile ("" : : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5),
                         "r"(a6), "r"(a7), "r"(a8), "r"(a9), "r"(a10));
    }
    
    return sum + a40;
}

/* ============================================
 * Pattern B: Floating-point intensive computation
 * Pressures floating-point and vector registers
 * ============================================ */
NOINLINE double pattern_b_fp_pressure(double input) {
    /* 20+ double variables */
    double b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    double b11, b12, b13, b14, b15, b16, b17, b18, b19, b20;
    double b21, b22, b23, b24, b25;
    
    b1 = input + 1.0;
    b2 = b1 * 2.0;
    b3 = b2 - b1;
    b4 = b3 + input;
    b5 = b4 * b3;
    b6 = b5 / (b1 != 0.0 ? b1 : 1.0);
    b7 = b6 * b5;
    b8 = b7 - b6;
    b9 = b8 + b7;
    b10 = b9 * b8;
    
    b11 = b10 / (b9 != 0.0 ? b9 : 1.0);
    b12 = b11 * b10;
    b13 = b12 - b11;
    b14 = b13 + b12;
    b15 = b14 * b13;
    b16 = b15 / (b14 != 0.0 ? b14 : 1.0);
    b17 = b16 * b15;
    b18 = b17 - b16;
    b19 = b18 + b17;
    b20 = b19 * b18;
    
    b21 = b20 / (b19 != 0.0 ? b19 : 1.0);
    b22 = b21 * b20;
    b23 = b22 - b21;
    b24 = b23 + b22;
    b25 = b24 * b23;
    
    /* Complex floating-point loop */
    double result = 0.0;
    for (int i = 0; i < 500; i++) {
        double t = (double)i;
        b1 = b25 + t;
        b2 = b1 * b24;
        b3 = b2 - b23;
        b4 = b3 + b22;
        b5 = b4 * b21;
        b6 = b5 / (b20 != 0.0 ? b20 : 1.0);
        b7 = b6 * b19;
        b8 = b7 - b18;
        b9 = b8 + b17;
        b10 = b9 * b16;
        
        /* Trigonometric-like computation */
        b11 = b10 * 0.5;
        b12 = b11 + 0.25;
        b13 = b12 * 0.75;
        b14 = b13 - 0.125;
        b15 = b14 * 1.5;
        
        result += b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9 + b10 +
                  b11 + b12 + b13 + b14 + b15;
        
        /* Prevent optimization */
        asm volatile ("" : : "r"(b1), "r"(b2), "r"(b3), "r"(b4), "r"(b5));
    }
    
    return result + b25;
}

/* ============================================
 * Pattern C: Complex control flow with switch statement
 * Creates many basic blocks for CFG complexity
 * ============================================ */
NOINLINE int pattern_c_cfg_complexity(int input) {
    int result = input;
    
    /* Nested loops with breaks/continues */
    for (int i = 0; i < 100; i++) {
        if (i % 3 == 0) {
            result += i;
            continue;
        }
        
        for (int j = 0; j < 50; j++) {
            if (j % 7 == 0) {
                result -= j;
                break;
            }
            
            int temp = 0;
            /* Switch with 20+ cases */
            switch ((i + j) % 23) {
                case 0: temp = result * 2; break;
                case 1: temp = result + 1; break;
                case 2: temp = result - 1; break;
                case 3: temp = result * 3; break;
                case 4: temp = result / 2; break;
                case 5: temp = result ^ 0xFF; break;
                case 6: temp = result | 0xAA; break;
                case 7: temp = result & 0x55; break;
                case 8: temp = result << 1; break;
                case 9: temp = result >> 2; break;
                case 10: temp = result % 17; break;
                case 11: temp = result * result; break;
                case 12: temp = -result; break;
                case 13: temp = ~result; break;
                case 14: temp = result + j; break;
                case 15: temp = result - i; break;
                case 16: temp = result * i; break;
                case 17: temp = result / (j ? j : 1); break;
                case 18: temp = result ^ j; break;
                case 19: temp = result | i; break;
                case 20: temp = result & j; break;
                case 21: temp = result << (i & 3); break;
                case 22: temp = result >> (j & 3); break;
                default: temp = 0; break;
            }
            result = temp;
            
            /* Early exit to create more CFG edges */
            if (result > 1000000) {
                goto early_exit;
            }
        }
        
        /* Another control flow twist */
        if (i % 11 == 0) {
            result += 1000;
        } else if (i % 13 == 0) {
            result -= 500;
        } else {
            result *= 2;
        }
    }
    
early_exit:
    return result;
}

/* ============================================
 * Pattern D: SIMD vector operations
 * Pressures vector/SIMD registers
 * ============================================ */
NOINLINE v4si pattern_d_simd_pressure(v4si input) {
    v4si v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    v4si v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    
    /* Initialize vectors */
    v1 = input + (v4si){1, 1, 1, 1};
    v2 = v1 * (v4si){2, 2, 2, 2};
    v3 = v2 - v1;
    v4 = v3 + input;
    v5 = v4 * v3;
    v6 = v5 & (v4si){0x0F, 0x0F, 0x0F, 0x0F};
    v7 = v6 | (v4si){0xF0, 0xF0, 0xF0, 0xF0};
    v8 = v7 ^ (v4si){0xFF, 0xFF, 0xFF, 0xFF};
    v9 = v8 << (v4si){1, 2, 3, 4};
    v10 = v9 >> (v4si){4, 3, 2, 1};
    
    v11 = v10 + v9;
    v12 = v11 - v8;
    v13 = v12 * v7;
    v14 = v13 & v6;
    v15 = v14 | v5;
    v16 = v15 ^ v4;
    v17 = v16 << (v4si){2, 3, 4, 1};
    v18 = v17 >> (v4si){1, 4, 3, 2};
    v19 = v18 + v17;
    v20 = v19 - v16;
    
    /* Vector loop */
    v4si result = {0, 0, 0, 0};
    for (int i = 0; i < 200; i++) {
        v4si idx = {i, i+1, i+2, i+3};
        v1 = v20 + idx;
        v2 = v1 * v19;
        v3 = v2 - v18;
        v4 = v3 + v17;
        v5 = v4 * v16;
        v6 = v5 & v15;
        v7 = v6 | v14;
        v8 = v7 ^ v13;
        v9 = v8 << (idx & (v4si){3, 3, 3, 3});
        v10 = v9 >> (v4si){1, 1, 1, 1};
        
        result = result + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        
        /* Prevent dead code elimination */
        asm volatile ("" : : "x"(v1), "x"(v2), "x"(v3), "x"(v4), "x"(v5));
    }
    
    return result;
}

/* ============================================
 * Pattern E: Explicit register variables
 * Conflicts with allocator's choices
 * ============================================ */
NOINLINE int pattern_e_register_conflict(int input) {
    /* Explicit register variables that may conflict */
    register int r1 asm ("r12") = input + 1;
    register int r2 asm ("r13") = r1 * 2;
    register int r3 asm ("r14") = r2 - r1;
    register int r4 asm ("r15") = r3 + input;
    
    /* Many other variables to create pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    
    v1 = r4 * r3;
    v2 = v1 / (r2 ? r2 : 1);
    v3 = v2 ^ r1;
    v4 = v3 | r4;
    v5 = v4 & r3;
    v6 = v5 << 2;
    v7 = v6 >> 1;
    v8 = v7 + r2;
    v9 = v8 - r1;
    v10 = v9 * r4;
    
    v11 = v10 % (v9 ? v9 : 1);
    v12 = v11 ^ v10;
    v13 = v12 | v9;
    v14 = v13 & v8;
    v15 = v14 << 3;
    v16 = v15 >> 2;
    v17 = v16 + v15;
    v18 = v17 - v14;
    v19 = v18 * v13;
    v20 = v19 / (v12 ? v12 : 1);
    
    /* Mix explicit and automatic variables in computation */
    int sum = 0;
    for (int i = 0; i < 300; i++) {
        r1 = v20 + i;
        r2 = r1 * v19;
        r3 = r2 - v18;
        r4 = r3 + v17;
        
        v1 = r4 * r3;
        v2 = v1 / (r2 ? r2 : 1);
        v3 = v2 ^ r1;
        
        sum += r1 + r2 + r3 + r4 + v1 + v2 + v3;
        
        /* Force register usage */
        asm volatile ("" : : "r"(r1), "r"(r2), "r"(r3), "r"(r4));
    }
    
    return sum + v20;
}

/* ============================================
 * Main function - calls all patterns
 * ============================================ */
COLD int main(int argc, char **argv) {
    int result = 0;
    
    /* Use CPU features to engage target-specific optimizations */
#ifdef __GNUC__
    if (__builtin_cpu_supports("avx2")) {
        /* This may affect register allocation decisions */
        result += 1;
    }
    if (__builtin_cpu_supports("sse4.2")) {
        result += 2;
    }
#endif
    
    /* Array of inputs to prevent constant folding */
    int inputs[] = {1, 2, 3, 5, 7, 11, 13, 17, 19, 23};
    
    for (int i = 0; i < 10; i++) {
        /* Call each pattern with different inputs */
        result += pattern_a_int_pressure(inputs[i]);
        result += (int)pattern_b_fp_pressure((double)inputs[i]);
        result += pattern_c_cfg_complexity(inputs[i]);
        
        v4si vec_input = {inputs[i], inputs[i]+1, inputs[i]+2, inputs[i]+3};
        v4si vec_result = pattern_d_simd_pressure(vec_input);
        result += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
        
        result += pattern_e_register_conflict(inputs[i]);
    }
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
