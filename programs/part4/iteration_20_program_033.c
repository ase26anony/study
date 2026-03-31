/* test_mcf_coverage.c
 * 
 * This test program is designed to trigger GCC's Min-Cost Flow pass
 * during register allocation, specifically to cover the special node
 * printing logic in mcf.cc's print_node function.
 * 
 * Compile with: gcc -O3 -fprofile-arcs -ftest-coverage -fdump-rtl-all test_mcf_coverage.c -o test_mcf
 * Run with: ./test_mcf
 * Generate coverage with: gcov -b test_mcf_coverage.c
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
    
    /* More complex interdependencies */
    a11 = a10 + a9 * a8 - a7;
    a12 = a11 ^ a10 | a9 & a8;
    a13 = a12 * 3 + a11 / 2;
    a14 = a13 - a12 + a11 - a10;
    a15 = a14 * a13 % (a12 + 1);
    a16 = a15 << (a14 & 3);
    a17 = a16 >> (a15 % 4);
    a18 = a17 + a16 - a15 + a14;
    a19 = a18 * 17 - a17 * 13;
    a20 = a19 ^ a18 | a17 & a16;
    
    /* Even more variables to ensure spill decisions */
    a21 = a20 + a19 - a18 + a17;
    a22 = a21 * a20 % 19;
    a23 = a22 + input * 2;
    a24 = a23 - input / 3;
    a25 = a24 * a23 + a22;
    a26 = a25 ^ a24 | a23;
    a27 = a26 & a25 + a24;
    a28 = a27 << 3;
    a29 = a28 >> 2;
    a30 = a29 + a28 - a27;
    
    /* Final set to exceed typical register file */
    a31 = a30 * 31;
    a32 = a31 + a30 - a29;
    a33 = a32 ^ a31 | a30;
    a34 = a33 & 0xFFFF;
    a35 = a34 * 3 + 1;
    a36 = a35 / 2 + a34;
    a37 = a36 ^ 0xAAAA;
    a38 = a37 | 0x5555;
    a39 = a38 << 1;
    a40 = a39 >> 2;
    
    /* Complex loop with all variables used */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        /* Rotate values through variables */
        int tmp = a1;
        a1 = a2 + i; a2 = a3 - i; a3 = a4 * (i + 1); a4 = a5 ^ i;
        a5 = a6 | i; a6 = a7 & i; a7 = a8 << (i & 3); a8 = a9 >> 1;
        a9 = a10 + tmp; a10 = a11 - tmp;
        
        /* Use all variables in computation */
        sum += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
        sum += a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19 + a20;
        sum += a21 + a22 + a23 + a24 + a25 + a26 + a27 + a28 + a29 + a30;
        sum += a31 + a32 + a33 + a34 + a35 + a36 + a37 + a38 + a39 + a40;
        
        /* Prevent loop unrolling from reducing pressure */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    
    return sum;
}

/* ============================================
 * PATTERN B: Floating-point intensive computation
 * Pressures floating-point and vector registers
 * ============================================ */
NOINLINE static double pattern_b_fp_pressure(double input) {
    /* 20+ double variables */
    double b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    double b11, b12, b13, b14, b15, b16, b17, b18, b19, b20;
    double b21, b22, b23, b24, b25;
    
    /* Initialize with transcendental functions to prevent optimization */
    b1 = input + 1.0;
    b2 = b1 * 2.0 + input;
    b3 = b2 - b1 + input;
    b4 = b3 * b2 + input;
    b5 = b4 / (b1 + 1.0) + input;
    b6 = b5 * 0.5 + input;
    b7 = b6 * 0.3333333333 + input;
    b8 = b7 * 0.25 + input;
    b9 = b8 * 0.2 + input;
    b10 = b9 * 0.1666666667 + input;
    
    /* More FP operations */
    b11 = b10 + b9 - b8 + b7;
    b12 = b11 * 1.1 - b10 * 0.9;
    b13 = b12 / 1.3 + b11 * 1.7;
    b14 = b13 * b12 - b11 * b10;
    b15 = b14 / (b13 + 0.001);
    b16 = b15 * 3.1415926535;
    b17 = b16 / 2.7182818284;
    b18 = b17 * b16 - b15 * b14;
    b19 = b18 + 1.6180339887;
    b20 = b19 * 0.7071067812;
    
    /* Final set */
    b21 = b20 * b19;
    b22 = b21 - b20 + b19;
    b23 = b22 / (b21 + 0.0001);
    b24 = b23 * 42.0;
    b25 = b24 - 17.0;
    
    /* Nested loops with FP computations */
    double result = 0.0;
    for (int i = 0; i < 50; i++) {
        double inner = 0.0;
        for (int j = 0; j < 10; j++) {
            /* Use all FP variables */
            inner += b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9 + b10;
            inner += b11 + b12 + b13 + b14 + b15 + b16 + b17 + b18 + b19 + b20;
            inner += b21 + b22 + b23 + b24 + b25;
            
            /* Modify variables to create live range splits */
            b1 += 0.01; b2 -= 0.01; b3 *= 1.01; b4 /= 1.01;
            b5 = b4 * 0.99; b6 = b5 + 0.5; b7 = b6 - 0.5;
            b8 = b7 * b6; b9 = b8 / b7; b10 = b9 + b8;
            
            /* Memory barrier for FP */
            asm volatile("" : "+f"(inner) : : "memory");
        }
        result += inner;
        
        /* Complex control flow within loop */
        if (i % 3 == 0) {
            b11 = b10 * 2.0;
            continue;
        } else if (i % 3 == 1) {
            b12 = b11 / 2.0;
            break;  /* Early exit creates complex CFG */
        } else {
            b13 = b12 + b11;
        }
        
        /* Restart loop after break */
        if (i == 25) {
            i = 0;
            continue;
        }
    }
    
    return result;
}

/* ============================================
 * PATTERN C: Complex control flow with switch statement
 * Creates many basic blocks for CFG complexity
 * ============================================ */
NOINLINE static int pattern_c_cfg_complexity(int input) {
    int result = input;
    
    /* Outer loop with switch inside */
    for (int i = 0; i < 100; i++) {
        switch (i % 23) {  /* 23 cases to create many basic blocks */
            case 0: result += i * 2; break;
            case 1: result ^= i; result *= 3; break;
            case 2: result |= 0xFF; result -= i; break;
            case 3: result &= 0xF0; result += i * i; break;
            case 4: result <<= (i & 3); break;
            case 5: result >>= 1; result ^= 0xAA; break;
            case 6: result = ~result; result += i; break;
            case 7: result = result % 17 + i; break;
            case 8: result = result / 2 + i * 3; break;
            case 9: result = (result << 4) | (result >> 4); break;
            case 10: result ^= result << 1; break;
            case 11: result |= result >> 2; break;
            case 12: result &= result << 3; break;
            case 13: result += result * 2; break;
            case 14: result -= result / 3; break;
            case 15: result = result ^ 0x1234; break;
            case 16: result = result | 0xABCD; break;
            case 17: result = result & 0xDCBA; break;
            case 18: result = -result + i; break;
            case 19: result = abs(result) + i; break;
            case 20: result = (result + i) * (result - i); break;
            case 21: result = result ^ ~i; break;
            case 22: result = (result << i) | (result >> (32 - i)); break;
            default: result = 0; break;
        }
        
        /* Nested switch for extra complexity */
        switch (result % 7) {
            case 0: result += 1; continue;  /* Continue creates back edge */
            case 1: result -= 2; break;
            case 2: result *= 3; 
                    if (result > 1000) goto early_exit;  /* goto creates irregular CFG */
                    break;
            case 3: result /= 4; break;
            case 4: result ^= 5; break;
            case 5: result |= 6; break;
            case 6: result &= 7; break;
        }
        
        /* Inner loop with break/continue */
        for (int j = 0; j < 10; j++) {
            if (j == result % 3) {
                continue;
            }
            if (j == result % 5) {
                break;
            }
            result += j;
        }
    }
    
early_exit:
    return result;
}

/* ============================================
 * PATTERN D: Vector extensions with SIMD pressure
 * Uses GCC vector extensions to pressure SIMD registers
 * ============================================ */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

NOINLINE static v4si pattern_d_vector_pressure(v4si input) {
    /* Multiple vector variables */
    v4si v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    v4si v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    
    /* Initialize vectors */
    v1 = input + (v4si){1, 2, 3, 4};
    v2 = v1 * (v4si){2, 3, 4, 5};
    v3 = v2 - v1;
    v4 = v3 & (v4si){0xFF, 0xFF, 0xFF, 0xFF};
    v5 = v4 | (v4si){0xF0, 0xF0, 0xF0, 0xF0};
    v6 = v5 ^ (v4si){0xAA, 0xAA, 0xAA, 0xAA};
    v7 = v6 << (v4si){1, 2, 3, 4};
    v8 = v7 >> (v4si){4, 3, 2, 1};
    v9 = v8 + v7 - v6;
    v10 = v9 * v8 / (v4si){2, 2, 2, 2};
    
    /* More vector operations */
    v11 = v10 + v9;
    v12 = v11 - v10;
    v13 = v12 * v11;
    v14 = v13 & v12;
    v15 = v14 | v13;
    v16 = v15 ^ v14;
    v17 = v16 << 1;
    v18 = v17 >> 2;
    v19 = v18 + v17;
    v20 = v19 - v18;
    
    /* Vector loop with horizontal reductions */
    v4si sum = {0, 0, 0, 0};
    for (int i = 0; i < 100; i++) {
        /* Cycle through vector variables */
        v4si tmp = v1;
        v1 = v2 + (v4si){i, i+1, i+2, i+3};
        v2 = v3 - (v4si){i, i, i, i};
        v3 = v4 * (v4si){i%2+1, i%3+1, i%4+1, i%5+1};
        v4 = v5 & (v4si){i, ~i, i, ~i};
        v5 = v6 | tmp;
        
        /* Use all vectors */
        sum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        sum += v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
        
        /* Prevent optimization */
        asm volatile("" : "+x"(sum) : : "memory");
    }
    
    return sum;
}
#endif

/* ============================================
 * PATTERN E: Explicit register variables
 * Conflicts with allocator's choices
 * ============================================ */
NOINLINE static int pattern_e_register_conflict(int input) {
    /* Explicit register variables that conflict with allocator */
    register int r0 asm ("r12") = input + 1;
    register int r1 asm ("r13") = r0 * 2;
    register int r2 asm ("r14") = r1 - r0;
    register int r3 asm ("r15") = r2 * r1;
    register int r4 asm ("bx") = r3 / (r0 + 1);
    
    /* More variables without explicit registers to create conflicts */
    int v1 = r4 ^ r3;
    int v2 = v1 | r2;
    int v3 = v2 & r1;
    int v4 = v3 << 2;
    int v5 = v4 >> 1;
    int v6 = v5 + r0;
    int v7 = v6 - r1;
    int v8 = v7 * r2;
    int v9 = v8 / r3;
    int v10 = v9 % r4;
    
    /* Complex loop that uses both register and stack variables */
    int result = 0;
    for (int i = 0; i < 100; i++) {
        /* Force spilling of register variables */
        asm volatile("" : "+r"(r0), "+r"(r1), "+r"(r2), "+r"(r3), "+r"(r4) : : "memory");
        
        /* Use all variables in computation */
        result += r0 + r1 + r2 + r3 + r4;
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        
        /* Modify variables to extend live ranges */
        r0 = r1 + i;
        r1 = r2 - i;
        r2 = r3 * (i + 1);
        r3 = r4 / (i + 2);
        r4 = r0 ^ i;
        
        v1 = v2 | i;
        v2 = v3 & i;
        v3 = v4 << (i & 3);
        v4 = v5 >> 1;
        v5 = v6 + v1;
        v6 = v7 - v2;
        v7 = v8 * v3;
        v8 = v9 / (v4 + 1);
        v9 = v10 % (v5 + 1);
        v10 = result & 0xFF;
        
        /* Memory barrier */
        asm volatile("" : "+r"(result) : : "memory");
    }
    
    return result;
}

/* ============================================
 * Main function that calls all patterns
 * ============================================ */
COLD int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use CPU feature check to engage target-specific optimizations */
    if (__builtin_cpu_supports("avx2") || 
        __builtin_cpu_supports("sse4.2") ||
        __builtin_cpu_supports("popcnt")) {
        /* This branch ensures architecture-specific register allocation */
        total += 1;
    }
    
    /* Array of inputs to prevent constant folding */
    int inputs[] = {1, 2, 3, 5, 7, 11, 13, 17, 19, 23};
    double fp_inputs[] = {1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1};
    
    /* Call each pattern multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        /* Pattern A: Integer pressure */
        total += pattern_a_int_pressure(inputs[i]);
        
        /* Pattern B: Floating-point pressure */
        total += (int)pattern_b_fp_pressure(fp_inputs[i]);
        
        /* Pattern C: CFG complexity */
        total += pattern_c_cfg_complexity(inputs[i] * 3);
        
        /* Pattern E: Register conflict */
        total += pattern_e_register_conflict(inputs[i] * 2);
    }
    
#ifdef __GNUC__
    /* Pattern D: Vector pressure (GCC only) */
    v4si vec_input = {1, 2, 3, 4};
    v4si vec_result = pattern_d_vector_pressure(vec_input);
    total += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
#endif
    
    /* Prevent dead code elimination of total */
    asm volatile("" : "+r"(total) : : "memory");
    
    /* Optional debug output (commented for minimal I/O) */
    /* printf("Result: %d\n", total); */
    
    return total % 256;  /* Return non-zero to indicate execution */
}
