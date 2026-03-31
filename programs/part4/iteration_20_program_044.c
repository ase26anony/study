/* test_mcf_coverage.c
 * 
 * This test program is designed to stress GCC's Min-Cost Flow register allocator
 * to trigger coverage of special node indices in the print_node function:
 * ENTRY_BLOCK, ENTRY_BLOCK+1, 2*EXIT_BLOCK, 2*EXIT_BLOCK+1,
 * new_exit_index, and new_entry_index.
 *
 * Compile with: gcc -O3 -fprofile-arcs -ftest-coverage -fdump-rtl-bbro -c test_mcf_coverage.c
 * Then run: ./test_mcf_coverage
 * Process coverage: gcov test_mcf_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure each function is compiled independently */
#define NOINLINE __attribute__((noinline))
#define COLD __attribute__((cold))

/* Vector type for SIMD pressure */
typedef int v4si __attribute__((vector_size(16)));

/* Pattern A: Integer arithmetic chain with 30+ variables in a loop
 * Forces spill decisions through high register pressure */
NOINLINE static int pattern_a_int_chain(int input) {
    /* Declare 30+ integer variables to create register pressure */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int a11, a12, a13, a14, a15, a16, a17, a18, a19, a20;
    int a21, a22, a23, a24, a25, a26, a27, a28, a29, a30;
    int a31, a32, a33, a34, a35;
    
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
    a14 = a13 / (a12 + 1) + input;
    a15 = a14 ^ a13 + input;
    a16 = a15 | a14 + input;
    a17 = a16 & a15 + input;
    a18 = a17 << 3 + input;
    a19 = a18 >> 2 + input;
    a20 = a19 + a18 + input;
    
    a21 = a20 * a19 + input;
    a22 = a21 - a20 + input;
    a23 = a22 ^ a21 + input;
    a24 = a23 | a22 + input;
    a25 = a24 & a23 + input;
    a26 = a25 << 1 + input;
    a27 = a26 >> 1 + input;
    a28 = a27 + a26 + input;
    a29 = a28 * a27 + input;
    a30 = a29 - a28 + input;
    
    a31 = a30 ^ a29 + input;
    a32 = a31 | a30 + input;
    a33 = a32 & a31 + input;
    a34 = a33 << 2 + input;
    a35 = a34 >> 1 + input;
    
    /* Complex loop with interdependencies */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        /* Create data dependencies between all variables */
        a1 = (a35 + i) % 256;
        a2 = a1 + a35;
        a3 = a2 - a1;
        a4 = a3 * a2;
        a5 = a4 + a3;
        a6 = a5 ^ a4;
        a7 = a6 | a5;
        a8 = a7 & a6;
        a9 = a8 << (i % 4);
        a10 = a9 >> 1;
        
        a11 = a10 + a9 + i;
        a12 = a11 - a10;
        a13 = a12 * a11;
        a14 = a13 / (a12 + 1);
        a15 = a14 ^ a13;
        a16 = a15 | a14;
        a17 = a16 & a15;
        a18 = a17 << (i % 3);
        a19 = a18 >> 2;
        a20 = a19 + a18;
        
        /* Force compiler to keep all variables alive */
        asm volatile("" : "+r"(a1), "+r"(a2), "+r"(a3), "+r"(a4), "+r"(a5),
                         "+r"(a6), "+r"(a7), "+r"(a8), "+r"(a9), "+r"(a10),
                         "+r"(a11), "+r"(a12), "+r"(a13), "+r"(a14), "+r"(a15),
                         "+r"(a16), "+r"(a17), "+r"(a18), "+r"(a19), "+r"(a20));
        
        sum += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
               a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19 + a20;
    }
    
    return sum + a21 + a22 + a23 + a24 + a25 + a26 + a27 + a28 + a29 + a30 +
           a31 + a32 + a33 + a34 + a35;
}

/* Pattern B: Floating-point intensive computation
 * Pressures floating-point register class */
NOINLINE static double pattern_b_float_pressure(double input) {
    /* Many double variables to pressure FP registers */
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    double d11, d12, d13, d14, d15, d16, d17, d18, d19, d20;
    double d21, d22, d23, d24, d25;
    
    d1 = input + 1.0;
    d2 = d1 * 2.0 + input;
    d3 = d2 - d1 + input;
    d4 = d3 * d2 + input;
    d5 = d4 / (d1 + 1.0) + input;
    d6 = d5 * d4 + input;
    d7 = d6 - d5 + input;
    d8 = d7 * d6 + input;
    d9 = d8 / (d7 + 1.0) + input;
    d10 = d9 * d8 + input;
    
    d11 = d10 + d9 + input;
    d12 = d11 - d10 + input;
    d13 = d12 * d11 + input;
    d14 = d13 / (d12 + 1.0) + input;
    d15 = d14 * d13 + input;
    d16 = d15 - d14 + input;
    d17 = d16 * d15 + input;
    d18 = d17 / (d16 + 1.0) + input;
    d19 = d18 * d17 + input;
    d20 = d19 - d18 + input;
    
    d21 = d20 * d19 + input;
    d22 = d21 - d20 + input;
    d23 = d22 * d21 + input;
    d24 = d23 / (d22 + 1.0) + input;
    d25 = d24 * d23 + input;
    
    /* Nested loops with floating-point operations */
    double result = 0.0;
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 10; j++) {
            /* Complex FP calculations with dependencies */
            d1 = d25 * 0.99 + i * 0.01;
            d2 = d1 * d25 + j * 0.1;
            d3 = d2 - d1;
            d4 = d3 * d2;
            d5 = d4 / (d3 + 0.001);
            d6 = d5 * d4;
            d7 = d6 - d5;
            d8 = d7 * d6;
            d9 = d8 / (d7 + 0.001);
            d10 = d9 * d8;
            
            /* Mix integer and floating point */
            int temp = i * j;
            d11 = d10 + temp * 0.5;
            d12 = d11 - d10;
            d13 = d12 * d11;
            d14 = d13 / (d12 + 0.001);
            d15 = d14 * d13;
            
            /* Prevent dead code elimination */
            asm volatile("" : "+x"(d1), "+x"(d2), "+x"(d3), "+x"(d4), "+x"(d5),
                             "+x"(d6), "+x"(d7), "+x"(d8), "+x"(d9), "+x"(d10),
                             "+x"(d11), "+x"(d12), "+x"(d13), "+x"(d14), "+x"(d15));
            
            result += d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10 +
                      d11 + d12 + d13 + d14 + d15;
        }
        
        /* Break/continue to create complex CFG */
        if (i % 7 == 0) continue;
        if (i % 13 == 0) break;
    }
    
    return result + d16 + d17 + d18 + d19 + d20 + d21 + d22 + d23 + d24 + d25;
}

/* Pattern C: Complex control flow with switch statement
 * Creates many basic blocks for MCF to handle */
NOINLINE static int pattern_c_complex_cfg(int input) {
    int result = input;
    
    /* Outer loop with switch inside */
    for (int i = 0; i < 100; i++) {
        /* Switch with many cases creates multiple basic blocks */
        switch (i % 23) {
            case 0:  result += i * 2; break;
            case 1:  result ^= i; result *= 3; break;
            case 2:  result |= 0xFF; result -= i; break;
            case 3:  result &= 0xF0; result += i * i; break;
            case 4:  result <<= 2; result ^= 0xAA; break;
            case 5:  result >>= 1; result |= 0x55; break;
            case 6:  result = ~result; result += i; break;
            case 7:  result = result * result % 256; break;
            case 8:  result = (result + i) | (result - i); break;
            case 9:  result = result ^ (result >> 4); break;
            case 10: result = (result << 3) & 0xFF; break;
            case 11: result = result / (i % 7 + 1); break;
            case 12: result = result % 17 + i; break;
            case 13: result = -result + i; break;
            case 14: result = abs(result) + i; break;
            case 15: result = result * 7 - i; break;
            case 16: result = result & ~i; result += 42; break;
            case 17: result = result | i; result ^= 0xCC; break;
            case 18: result = (result << 1) | (result >> 7); break;
            case 19: result = result + (i << 2); break;
            case 20: result = result - (i * 3); break;
            case 21: result = result * 5 % 128; break;
            case 22: result = ~result & 0x7F; break;
            default: result += 1; break;
        }
        
        /* Nested loop with break/continue */
        for (int j = 0; j < 10; j++) {
            if (j % 3 == 0) continue;
            if (j == 7) break;
            result += j;
        }
        
        /* Conditional with goto (GCC extension) for even more complex CFG */
        if (i % 11 == 0) {
            void *label = &&skip_block;
            goto *label;
        skip_block:
            result += 111;
        }
    }
    
    return result;
}

/* Pattern D: SIMD vector operations
 * Pressures vector/SIMD registers */
NOINLINE static v4si pattern_d_simd_pressure(v4si input) {
    v4si v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    v4si v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    
    /* Initialize vectors */
    v1 = input + (v4si){1, 1, 1, 1};
    v2 = v1 * (v4si){2, 2, 2, 2} + input;
    v3 = v2 - v1 + input;
    v4 = v3 * v2 + input;
    v5 = v4 + v3;
    v6 = v5 * v4;
    v7 = v6 - v5;
    v8 = v7 * v6;
    v9 = v8 + v7;
    v10 = v9 * v8;
    
    v11 = v10 + v9;
    v12 = v11 - v10;
    v13 = v12 * v11;
    v14 = v13 + v12;
    v15 = v14 * v13;
    v16 = v15 - v14;
    v17 = v16 * v15;
    v18 = v17 + v16;
    v19 = v18 * v17;
    v20 = v19 - v18;
    
    /* Loop with vector operations */
    v4si result = {0, 0, 0, 0};
    for (int i = 0; i < 50; i++) {
        v4si idx = {i, i+1, i+2, i+3};
        
        /* Complex vector calculations */
        v1 = v20 + idx;
        v2 = v1 * v20;
        v3 = v2 - v1;
        v4 = v3 * v2;
        v5 = v4 + v3;
        v6 = v5 * v4;
        v7 = v6 - v5;
        v8 = v7 * v6;
        v9 = v8 + v7;
        v10 = v9 * v8;
        
        /* Mix with scalar operations */
        int scalar = i * 3;
        v11 = v10 + (v4si){scalar, scalar, scalar, scalar};
        v12 = v11 - v10;
        v13 = v12 * v11;
        
        /* Prevent optimization */
        asm volatile("" : "+x"(v1), "+x"(v2), "+x"(v3), "+x"(v4), "+x"(v5),
                         "+x"(v6), "+x"(v7), "+x"(v8), "+x"(v9), "+x"(v10),
                         "+x"(v11), "+x"(v12), "+x"(v13));
        
        result = result + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                 v11 + v12 + v13;
        
        /* Control flow in vector loop */
        if (i % 7 == 0) {
            v14 = v13 * (v4si){2, 2, 2, 2};
            result = result + v14;
        }
    }
    
    return result + v15 + v16 + v17 + v18 + v19 + v20;
}

/* Pattern E: Explicit register variables causing conflicts */
NOINLINE static int pattern_e_register_conflict(int input) {
    /* Explicit register variables that conflict with allocator's choices */
    register int r1 asm ("r12") = input + 1;
    register int r2 asm ("r13") = r1 * 2;
    register int r3 asm ("r14") = r2 - r1;
    register int r4 asm ("r15") = r3 * r2;
    
    /* Many other variables to create pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    
    v1 = r4 + 1;
    v2 = v1 * r1;
    v3 = v2 - v1;
    v4 = v3 * v2;
    v5 = v4 + r2;
    v6 = v5 * v4;
    v7 = v6 - v5;
    v8 = v7 * v6;
    v9 = v8 + r3;
    v10 = v9 * v8;
    
    v11 = v10 + v9;
    v12 = v11 - v10;
    v13 = v12 * v11;
    v14 = v13 + v12;
    v15 = v14 * v13;
    v16 = v15 - v14;
    v17 = v16 * v15;
    v18 = v17 + v16;
    v19 = v18 * v17;
    v20 = v19 - v18;
    
    /* Complex loop mixing register and stack variables */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        /* Force use of all variables */
        r1 = (r1 + i) % 256;
        r2 = r2 * 3 + i;
        r3 = r3 ^ r2;
        r4 = r4 | r3;
        
        v1 = v20 + i;
        v2 = v1 * r1;
        v3 = v2 - v1;
        v4 = v3 * v2;
        v5 = v4 + r2;
        v6 = v5 * v4;
        v7 = v6 - v5;
        v8 = v7 * v6;
        v9 = v8 + r3;
        v10 = v9 * v8;
        
        /* Prevent optimization */
        asm volatile("" : "+r"(r1), "+r"(r2), "+r"(r3), "+r"(r4),
                         "+r"(v1), "+r"(v2), "+r"(v3), "+r"(v4), "+r"(v5),
                         "+r"(v6), "+r"(v7), "+r"(v8), "+r"(v9), "+r"(v10));
        
        sum += r1 + r2 + r3 + r4 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        
        /* Nested control flow */
        if (i % 13 == 0) {
            for (int j = 0; j < 5; j++) {
                if (j % 2 == 0) continue;
                sum += j * v11;
                v11 = v11 + 1;
            }
        }
    }
    
    return sum + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
}

/* Main function that calls all patterns */
COLD int main(int argc, char *argv[]) {
    int total = 0;
    double total_double = 0.0;
    
    /* Use CPU features to engage target-specific optimizations */
#ifdef __GNUC__
    if (__builtin_cpu_supports("avx2")) {
        /* This may affect register allocation decisions */
        asm volatile("" ::: "memory");
    }
#endif
    
    /* Array of inputs to prevent constant folding */
    int inputs[] = {1, 2, 3, 5, 7, 11, 13, 17, 19, 23};
    int num_inputs = sizeof(inputs) / sizeof(inputs[0]);
    
    /* Call each pattern multiple times with different inputs */
    for (int i = 0; i < num_inputs; i++) {
        /* Pattern A: Integer chain */
        total += pattern_a_int_chain(inputs[i]);
        
        /* Pattern B: Floating point */
        total_double += pattern_b_float_pressure(inputs[i] * 0.5);
        
        /* Pattern C: Complex CFG */
        total += pattern_c_complex_cfg(inputs[i]);
        
        /* Pattern D: SIMD vectors */
        v4si vec_input = {inputs[i], inputs[i]+1, inputs[i]+2, inputs[i]+3};
        v4si vec_result = pattern_d_simd_pressure(vec_input);
        total += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
        
        /* Pattern E: Register conflicts */
        total += pattern_e_register_conflict(inputs[i]);
    }
    
    /* Mix integer and double results to prevent optimization */
    int final_result = total + (int)total_double;
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", final_result);
    
    return final_result % 256;
}
