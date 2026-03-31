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

/* ============================================
   PATTERN A: Integer arithmetic chain with 30+ variables
   Creates massive register pressure in a loop
   ============================================ */
NOINLINE static int pattern_a_int_pressure(int input) {
    /* Declare 30+ integer variables to pressure integer registers */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int a11, a12, a13, a14, a15, a16, a17, a18, a19, a20;
    int a21, a22, a23, a24, a25, a26, a27, a28, a29, a30;
    int a31, a32, a33, a34, a35;
    
    /* Initialize with input to prevent constant folding */
    a1 = input + 1;
    a2 = a1 * 2 + input;
    a3 = a2 - a1 + input;
    a4 = a3 * a2 + input;
    a5 = a4 / (a1 > 0 ? a1 : 1) + input;
    a6 = a5 ^ a4 + input;
    a7 = a6 | a5 + input;
    a8 = a7 & a6 + input;
    a9 = a8 << 2 + input;
    a10 = a9 >> 1 + input;
    
    /* Complex interdependent chain */
    a11 = a10 + a9 * a8 - a7;
    a12 = a11 * a10 / (a9 > 0 ? a9 : 1);
    a13 = a12 ^ a11 | a10;
    a14 = a13 & a12 << 3;
    a15 = a14 - a13 + a12;
    a16 = a15 * a14 - a13;
    a17 = a16 + a15 * a14;
    a18 = a17 ^ a16 & a15;
    a19 = a18 | a17 << 1;
    a20 = a19 >> 2 + a18;
    
    /* More variables to increase pressure */
    a21 = a20 * a19 - a18;
    a22 = a21 + a20 * a19;
    a23 = a22 ^ a21 | a20;
    a24 = a23 & a22 << 2;
    a25 = a24 - a23 + a22;
    a26 = a25 * a24 - a23;
    a27 = a26 + a25 * a24;
    a28 = a27 ^ a26 & a25;
    a29 = a28 | a27 << 1;
    a30 = a29 >> 1 + a28;
    
    /* Final set */
    a31 = a30 * a29 - a28;
    a32 = a31 + a30 * a29;
    a33 = a32 ^ a31 | a30;
    a34 = a33 & a32 << 1;
    a35 = a34 - a33 + a32;
    
    /* Loop to create back-edge in CFG and increase register pressure */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        /* Use all variables in loop to keep them live */
        sum += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
               a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19 + a20 +
               a21 + a22 + a23 + a24 + a25 + a26 + a27 + a28 + a29 + a30 +
               a31 + a32 + a33 + a34 + a35;
        
        /* Modify some variables to prevent dead code elimination */
        if (i % 2 == 0) {
            a1 += a2;
            a3 ^= a4;
            a5 |= a6;
        } else {
            a7 &= a8;
            a9 <<= 1;
            a10 >>= 1;
        }
    }
    
    /* Use asm to prevent optimization */
    asm volatile ("" : : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5),
                       "r"(a6), "r"(a7), "r"(a8), "r"(a9), "r"(a10));
    
    return sum + a35;
}

/* ============================================
   PATTERN B: Floating-point intensive computation
   Pressures floating-point registers
   ============================================ */
NOINLINE static double pattern_b_float_pressure(double input) {
    /* 20+ double variables to pressure FP registers */
    double b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    double b11, b12, b13, b14, b15, b16, b17, b18, b19, b20;
    double b21, b22, b23, b24, b25;
    
    /* Initialize with transcendental functions to prevent optimization */
    b1 = input + 1.0;
    b2 = b1 * 3.141592653589793;
    b3 = b2 / 2.718281828459045;
    b4 = b3 * b2 - b1;
    b5 = b4 + b3 / b2;
    b6 = b5 * 1.414213562373095;
    b7 = b6 - b5 + b4;
    b8 = b7 * 0.577215664901532;  /* Euler-Mascheroni constant */
    b9 = b8 + b7 - b6;
    b10 = b9 * 1.618033988749894; /* Golden ratio */
    
    /* Complex FP chain */
    b11 = b10 * b9 / b8;
    b12 = b11 + b10 - b9;
    b13 = b12 * 2.302585092994046; /* ln(10) */
    b14 = b13 / 0.693147180559945; /* ln(2) */
    b15 = b14 - b13 + b12;
    b16 = b15 * b14 / b13;
    b17 = b16 + b15 - b14;
    b18 = b17 * 0.434294481903252; /* log10(e) */
    b19 = b18 / 1.442695040888963; /* log2(e) */
    b20 = b19 - b18 + b17;
    
    /* More FP variables */
    b21 = b20 * b19 / b18;
    b22 = b21 + b20 - b19;
    b23 = b22 * 0.785398163397448; /* π/4 */
    b24 = b23 / 0.523598775598299; /* π/6 */
    b25 = b24 - b23 + b22;
    
    /* Loop with mixed operations */
    double result = 0.0;
    for (int i = 0; i < 50; i++) {
        /* Use all FP variables */
        result += b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9 + b10 +
                  b11 + b12 + b13 + b14 + b15 + b16 + b17 + b18 + b19 + b20 +
                  b21 + b22 + b23 + b24 + b25;
        
        /* Modify variables to keep them live */
        if (i % 3 == 0) {
            b1 *= 1.01;
            b2 /= 1.01;
            b3 += 0.01;
        } else if (i % 3 == 1) {
            b4 -= 0.01;
            b5 *= 0.99;
            b6 /= 0.99;
        }
    }
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "f"(b1), "f"(b2), "f"(b3), "f"(b4), "f"(b5));
    
    return result + b25;
}

/* ============================================
   PATTERN C: Complex control flow with switch statement
   Creates many basic blocks for CFG complexity
   ============================================ */
NOINLINE static int pattern_c_complex_cfg(int input, int mode) {
    int result = input;
    
    /* Outer loop with switch inside */
    for (int i = 0; i < 100; i++) {
        switch (mode % 20) {
            case 0: result += i * 2; break;
            case 1: result ^= i; break;
            case 2: result |= i << 1; break;
            case 3: result &= ~i; break;
            case 4: result *= (i + 1); break;
            case 5: result /= (i > 0 ? i : 1); break;
            case 6: result -= i * 3; break;
            case 7: result = result << (i % 4); break;
            case 8: result = result >> (i % 4); break;
            case 9: result = ~result; break;
            case 10: result = result + (i * i); break;
            case 11: result = result - (i / 2); break;
            case 12: result = result | 0xFF; break;
            case 13: result = result & 0xFFFF; break;
            case 14: result = result ^ 0xAAAA; break;
            case 15: result = (result << 1) | 1; break;
            case 16: result = (result >> 1) & 0x7FFF; break;
            case 17: result = result + mode; break;
            case 18: result = result * mode; break;
            case 19: result = result ^ mode; break;
            default: result = 0; break;
        }
        
        /* Nested loop with break/continue */
        for (int j = 0; j < 10; j++) {
            if (j % 2 == 0) {
                result += j;
                continue;
            }
            
            if (j == 5) {
                result -= 10;
                break;
            }
            
            result *= 2;
        }
        
        /* Change mode for next iteration */
        mode = (mode * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    return result;
}

/* ============================================
   PATTERN D: Vector/SIMD operations
   Pressures vector registers
   ============================================ */
NOINLINE static v4si pattern_d_vector_pressure(v4si input) {
    /* Multiple vector variables */
    v4si v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    v4si v11, v12, v13, v14, v15;
    
    /* Initialize vectors */
    v1 = input + (v4si){1, 2, 3, 4};
    v2 = v1 * (v4si){2, 3, 4, 5};
    v3 = v2 - v1;
    v4 = v3 * v2;
    v5 = v4 + v3;
    v6 = v5 ^ v4;
    v7 = v6 | v5;
    v8 = v7 & v6;
    v9 = v8 << (v4si){1, 2, 1, 2};
    v10 = v9 >> (v4si){1, 0, 1, 0};
    
    /* More vector operations */
    v11 = v10 + v9 * v8;
    v12 = v11 - v10 + v9;
    v13 = v12 * v11 / (v4si){2, 2, 2, 2};
    v14 = v13 ^ v12 | v11;
    v15 = v14 & v13 << (v4si){1, 1, 1, 1};
    
    /* Loop with vector operations */
    v4si sum = {0, 0, 0, 0};
    for (int i = 0; i < 50; i++) {
        sum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
               v11 + v12 + v13 + v14 + v15;
        
        /* Modify vectors */
        if (i % 4 == 0) {
            v1 = v1 + (v4si){i, i, i, i};
            v2 = v2 * (v4si){2, 1, 2, 1};
        }
    }
    
    /* Prevent optimization */
    asm volatile ("" : : "x"(v1), "x"(v2), "x"(v3), "x"(v4), "x"(v5));
    
    return sum + v15;
}

/* ============================================
   PATTERN E: Explicit register variables
   Conflicts with register allocator
   ============================================ */
NOINLINE static int pattern_e_explicit_registers(int input) {
    /* Try to use specific registers */
    register int r1 asm ("r12") = input + 1;
    register int r2 asm ("r13") = r1 * 2;
    register int r3 asm ("r14") = r2 + input;
    register int r4 asm ("r15") = r3 ^ r2;
    
    /* More variables that will conflict */
    int x1, x2, x3, x4, x5, x6, x7, x8, x9, x10;
    
    x1 = r1 + r2;
    x2 = r3 * r4;
    x3 = x1 ^ x2;
    x4 = x3 + r1;
    x5 = x4 * r2;
    x6 = x5 - r3;
    x7 = x6 | r4;
    x8 = x7 & x1;
    x9 = x8 << 2;
    x10 = x9 >> 1;
    
    /* Complex loop with register pressure */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        /* Force use of all variables */
        sum += r1 + r2 + r3 + r4 + x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9 + x10;
        
        /* Modify register variables */
        r1 += i;
        r2 ^= i;
        r3 |= i;
        r4 &= ~i;
        
        /* Modify other variables */
        x1 = x2 + i;
        x2 = x3 * i;
        x3 = x4 ^ i;
    }
    
    /* Explicit asm to use the register variables */
    asm volatile ("" : : "r"(r1), "r"(r2), "r"(r3), "r"(r4));
    
    return sum + x10;
}

/* ============================================
   Helper function with computed goto (GCC extension)
   Creates unusual control flow
   ============================================ */
NOINLINE static int pattern_f_computed_goto(int input) {
    void *labels[] = {&&label0, &&label1, &&label2, &&label3, &&label4,
                     &&label5, &&label6, &&label7, &&label8, &&label9};
    
    int result = input;
    int i = 0;
    
    /* Loop with computed goto */
    while (i < 100) {
        goto *labels[i % 10];
        
    label0:
        result += i * 3;
        i++;
        continue;
    label1:
        result ^= i;
        i++;
        continue;
    label2:
        result |= 0xAA;
        i++;
        continue;
    label3:
        result &= 0x55;
        i++;
        continue;
    label4:
        result *= 2;
        i++;
        continue;
    label5:
        result /= 2;
        i++;
        continue;
    label6:
        result = ~result;
        i++;
        continue;
    label7:
        result <<= 1;
        i++;
        continue;
    label8:
        result >>= 1;
        i++;
        continue;
    label9:
        result = result + i - 5;
        i++;
        continue;
    }
    
    return result;
}

/* ============================================
   Main function - calls all patterns
   ============================================ */
COLD int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use CPU feature check to engage target-specific optimizations */
    #ifdef __GNUC__
    if (__builtin_cpu_supports("sse2") ||
        __builtin_cpu_supports("avx") ||
        __builtin_cpu_supports("avx2")) {
        /* This ensures the compilation pipeline includes
           target-specific register allocation */
    }
    #endif
    
    /* Call each pattern multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        total += pattern_a_int_pressure(i);
        total += (int)pattern_b_float_pressure(i * 0.1);
        total += pattern_c_complex_cfg(i, i * 3);
        
        v4si vec_input = {i, i+1, i+2, i+3};
        v4si vec_result = pattern_d_vector_pressure(vec_input);
        total += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
        
        total += pattern_e_explicit_registers(i);
        total += pattern_f_computed_goto(i);
    }
    
    /* Print result to prevent optimization of entire program */
    printf("Total result: %d\n", total);
    
    /* Additional loop to increase execution count for coverage */
    for (int iter = 0; iter < 1000; iter++) {
        for (int i = 1; i < 5; i++) {
            pattern_a_int_pressure(iter + i);
        }
    }
    
    return total > 0 ? 0 : 1;
}
