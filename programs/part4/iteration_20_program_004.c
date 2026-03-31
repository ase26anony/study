/* test_mcf_coverage.c
 * 
 * This test program is designed to stress GCC's Min-Cost Flow register allocator
 * to trigger coverage of special node indices in the print_node function.
 * 
 * Compile with: gcc -O3 -fprofile-arcs -ftest-coverage -fdump-rtl-bbro test_mcf_coverage.c -o test_mcf
 * Run with: ./test_mcf
 * Generate coverage with: gcov test_mcf_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure each function is compiled separately */
#define NOINLINE __attribute__((noinline))
#define COLD __attribute__((cold))

/* Pattern A: Integer arithmetic chain with 30+ variables in a loop
 * Forces spill decisions through high register pressure */
NOINLINE static int pattern_a_int_pressure(int input) {
    /* 30+ integer variables to create register pressure */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int a11, a12, a13, a14, a15, a16, a17, a18, a19, a20;
    int a21, a22, a23, a24, a25, a26, a27, a28, a29, a30;
    int a31, a32, a33, a34, a35;
    
    /* Initialize with input to prevent constant folding */
    a1 = input + 1;
    a2 = a1 * 2 + input;
    a3 = a2 + a1 - input;
    a4 = a3 * a2 / (input + 1);
    a5 = a4 ^ a3;
    a6 = a5 | a4;
    a7 = a6 & a5;
    a8 = a7 << 2;
    a9 = a8 >> 1;
    a10 = a9 + a8 - a7;
    
    a11 = a10 * 3 + input;
    a12 = a11 - a10;
    a13 = a12 | a11;
    a14 = a13 & a12;
    a15 = a14 ^ a13;
    a16 = a15 + a14;
    a17 = a16 * a15;
    a18 = a17 / (input + 2);
    a19 = a18 % (input + 3);
    a20 = a19 << 3;
    
    a21 = a20 >> 2;
    a22 = a21 + a20;
    a23 = a22 * a21;
    a24 = a23 - a22;
    a25 = a24 ^ a23;
    a26 = a25 | a24;
    a27 = a26 & a25;
    a28 = a27 + input;
    a29 = a28 * 5;
    a30 = a29 / (input + 4);
    
    a31 = a30 + a29;
    a32 = a31 * a30;
    a33 = a32 - a31;
    a34 = a33 ^ a32;
    a35 = a34 | a33;
    
    /* Complex loop with interdependencies to force MCF analysis */
    for (int i = 0; i < 100; i++) {
        a1 = a35 + i;
        a2 = a1 * a35;
        a3 = a2 - a1;
        a4 = a3 ^ a2;
        a5 = a4 | a3;
        a6 = a5 & a4;
        a7 = a6 << (i & 3);
        a8 = a7 >> 1;
        a9 = a8 + a7;
        a10 = a9 * a8;
        
        /* Create cross-dependencies */
        a35 = a10 % (input + 5);
        a20 = a35 + a10;
        a15 = a20 * a35;
    }
    
    /* Force all variables to be live at return */
    asm volatile ("" : : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5),
                     "r"(a6), "r"(a7), "r"(a8), "r"(a9), "r"(a10),
                     "r"(a11), "r"(a12), "r"(a13), "r"(a14), "r"(a15),
                     "r"(a16), "r"(a17), "r"(a18), "r"(a19), "r"(a20),
                     "r"(a21), "r"(a22), "r"(a23), "r"(a24), "r"(a25),
                     "r"(a26), "r"(a27), "r"(a28), "r"(a29), "r"(a30),
                     "r"(a31), "r"(a32), "r"(a33), "r"(a34), "r"(a35));
    
    return a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
           a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19 + a20 +
           a21 + a22 + a23 + a24 + a25 + a26 + a27 + a28 + a29 + a30 +
           a31 + a32 + a33 + a34 + a35;
}

/* Pattern B: Floating-point intensive computation
 * Pressures floating-point register class */
NOINLINE static double pattern_b_float_pressure(double input) {
    /* 20+ double variables */
    double b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    double b11, b12, b13, b14, b15, b16, b17, b18, b19, b20;
    double b21, b22, b23, b24, b25;
    
    b1 = input + 1.0;
    b2 = b1 * 2.0;
    b3 = b2 / 1.5;
    b4 = b3 - b2;
    b5 = b4 * b3;
    b6 = b5 + b4;
    b7 = b6 / b5;
    b8 = b7 * 3.14159;
    b9 = b8 + b7;
    b10 = b9 - b8;
    
    b11 = b10 * 2.71828;
    b12 = b11 / b10;
    b13 = b12 + b11;
    b14 = b13 - b12;
    b15 = b14 * b13;
    b16 = b15 / 2.0;
    b17 = b16 + 1.0;
    b18 = b17 * 0.5;
    b19 = b18 - 0.25;
    b20 = b19 * b18;
    
    b21 = b20 / 3.0;
    b22 = b21 + input;
    b23 = b22 * b21;
    b24 = b23 - b22;
    b25 = b24 / (input + 0.001);
    
    /* Loop with floating-point operations */
    for (int i = 0; i < 50; i++) {
        double t = (double)i * 0.1;
        b1 = b25 + t;
        b2 = b1 * t;
        b3 = b2 / (t + 1.0);
        b4 = b3 - b2;
        b5 = b4 * 1.1;
        
        /* Mix with integer index */
        b25 = b5 + (double)(i % 10);
    }
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "f"(b1), "f"(b2), "f"(b3), "f"(b4), "f"(b5),
                     "f"(b6), "f"(b7), "f"(b8), "f"(b9), "f"(b10),
                     "f"(b11), "f"(b12), "f"(b13), "f"(b14), "f"(b15),
                     "f"(b16), "f"(b17), "f"(b18), "f"(b19), "f"(b20),
                     "f"(b21), "f"(b22), "f"(b23), "f"(b24), "f"(b25));
    
    return b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9 + b10 +
           b11 + b12 + b13 + b14 + b15 + b16 + b17 + b18 + b19 + b20 +
           b21 + b22 + b23 + b24 + b25;
}

/* Pattern C: Complex control flow with switch statement
 * Creates many basic blocks for complex CFG */
NOINLINE static int pattern_c_complex_cfg(int input) {
    int result = input;
    
    /* Nested loops with breaks and continues */
    for (int i = 0; i < 20; i++) {
        if (i % 3 == 0) {
            result += i;
            continue;
        }
        
        for (int j = 0; j < 15; j++) {
            if (j == i) {
                result *= 2;
                break;
            }
            result += j;
            
            /* Switch with many cases creates multiple basic blocks */
            switch ((i + j) % 12) {
                case 0: result += 1; break;
                case 1: result -= 2; break;
                case 2: result *= 3; break;
                case 3: result /= 4; break;
                case 4: result |= 0xFF; break;
                case 5: result &= 0xF0; break;
                case 6: result ^= 0xAA; break;
                case 7: result <<= 1; break;
                case 8: result >>= 2; break;
                case 9: result = ~result; break;
                case 10: result = result % 17; break;
                case 11: result = result * result; break;
                default: result += 100; break;
            }
            
            /* Additional conditional */
            if (result > 1000) {
                result %= 1000;
                continue;
            }
        }
        
        /* Another switch */
        switch (i % 8) {
            case 0: result += 10; break;
            case 1: result -= 20; break;
            case 2: result *= 2; break;
            case 3: result /= 3; break;
            case 4: result = result ^ 0x55; break;
            case 5: result = result | 0x33; break;
            case 6: result = result & 0xCC; break;
            case 7: result = -result; break;
        }
    }
    
    /* Use computed goto (GCC extension) to create even more complex CFG */
    void* labels[] = { &&label0, &&label1, &&label2, &&label3, &&label4,
                      &&label5, &&label6, &&label7, &&label8, &&label9 };
    
    for (int i = 0; i < 10; i++) {
        goto *labels[i % 10];
        
        label0: result += 1; continue;
        label1: result += 2; continue;
        label2: result += 3; continue;
        label3: result += 4; continue;
        label4: result += 5; continue;
        label5: result += 6; continue;
        label6: result += 7; continue;
        label7: result += 8; continue;
        label8: result += 9; continue;
        label9: result += 10; continue;
    }
    
    return result;
}

/* Pattern D: Vector extensions for SIMD register pressure */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
#endif

NOINLINE static int pattern_d_vector_pressure(int input) {
#ifdef __GNUC__
    /* Multiple vector variables */
    v4si v1 = {input, input+1, input+2, input+3};
    v4si v2 = {input+4, input+5, input+6, input+7};
    v4si v3 = {input+8, input+9, input+10, input+11};
    v4si v4 = {input+12, input+13, input+14, input+15};
    v4si v5 = {input+16, input+17, input+18, input+19};
    v4si v6 = {input+20, input+21, input+22, input+23};
    v4si v7 = {input+24, input+25, input+26, input+27};
    v4si v8 = {input+28, input+29, input+30, input+31};
    
    v4sf f1 = {input*0.1f, input*0.2f, input*0.3f, input*0.4f};
    v4sf f2 = {input*0.5f, input*0.6f, input*0.7f, input*0.8f};
    v4sf f3 = {input*0.9f, input*1.0f, input*1.1f, input*1.2f};
    v4sf f4 = {input*1.3f, input*1.4f, input*1.5f, input*1.6f};
    
    v2df d1 = {input*0.01, input*0.02};
    v2df d2 = {input*0.03, input*0.04};
    v2df d3 = {input*0.05, input*0.06};
    v2df d4 = {input*0.07, input*0.08};
    
    /* Vector operations in loop */
    for (int i = 0; i < 25; i++) {
        v1 = v1 + v2;
        v2 = v2 * v3;
        v3 = v3 - v4;
        v4 = v4 ^ v5;
        v5 = v5 | v6;
        v6 = v6 & v7;
        v7 = v7 << 1;
        v8 = v8 >> 2;
        
        f1 = f1 + f2;
        f2 = f2 * f3;
        f3 = f3 - f4;
        f4 = f4 / (v4sf){2.0f, 2.0f, 2.0f, 2.0f};
        
        d1 = d1 + d2;
        d2 = d2 * d3;
        d3 = d3 - d4;
        d4 = d4 / (v2df){2.0, 2.0};
        
        /* Mix vector types */
        v1 = v1 + (v4si){i, i, i, i};
    }
    
    /* Extract results */
    int sum = v1[0] + v1[1] + v1[2] + v1[3] +
              v2[0] + v2[1] + v2[2] + v2[3] +
              v3[0] + v3[1] + v3[2] + v3[3] +
              v4[0] + v4[1] + v4[2] + v4[3];
    
    /* Prevent optimization */
    asm volatile ("" : : "x"(v1), "x"(v2), "x"(v3), "x"(v4),
                     "x"(v5), "x"(v6), "x"(v7), "x"(v8),
                     "x"(f1), "x"(f2), "x"(f3), "x"(f4),
                     "x"(d1), "x"(d2), "x"(d3), "x"(d4));
    
    return sum;
#else
    return input;
#endif
}

/* Pattern E: Explicit register variables to conflict with allocator */
NOINLINE static int pattern_e_register_conflict(int input) {
    /* Try to use specific registers that might conflict with allocator */
    register int r1 asm ("r12") = input + 1;
    register int r2 asm ("r13") = input + 2;
    register int r3 asm ("r14") = input + 3;
    register int r4 asm ("r15") = input + 4;
    
    /* Additional regular variables to create pressure */
    int v1 = r1 * 2;
    int v2 = r2 + r1;
    int v3 = r3 ^ r2;
    int v4 = r4 | r3;
    int v5 = v1 << 2;
    int v6 = v2 >> 1;
    int v7 = v3 & v4;
    int v8 = v5 + v6;
    int v9 = v7 * v8;
    int v10 = v9 / (input + 1);
    
    /* Force use of the register variables in complex expressions */
    for (int i = 0; i < 30; i++) {
        r1 = r2 + v1;
        r2 = r3 ^ v2;
        r3 = r4 | v3;
        r4 = r1 & v4;
        
        v1 = v2 + r1;
        v2 = v3 * r2;
        v3 = v4 - r3;
        v4 = v5 ^ r4;
        
        v5 = v6 << (i & 3);
        v6 = v7 >> 1;
        v7 = v8 + i;
        v8 = v9 * 2;
        v9 = v10 / (i + 1);
        v10 = r1 + r2 + r3 + r4;
    }
    
    /* Mix with inline asm to force specific register usage */
    asm volatile (
        "addl %%r12d, %%eax\n\t"
        "subl %%r13d, %%eax\n\t"
        "xorl %%r14d, %%eax\n\t"
        "orl  %%r15d, %%eax"
        : "=a"(v1)
        : "a"(v1), "r"(r1), "r"(r2), "r"(r3), "r"(r4)
        : "cc"
    );
    
    return r1 + r2 + r3 + r4 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
}

/* Helper to use __builtin_cpu_supports to engage target-specific optimizations */
static int use_cpu_features(void) {
    int has_avx2 = 0;
    int has_sse4 = 0;
    
#ifdef __GNUC__
    if (__builtin_cpu_supports("avx2")) {
        has_avx2 = 1;
    }
    if (__builtin_cpu_supports("sse4.2")) {
        has_sse4 = 1;
    }
#endif
    
    return has_avx2 + has_sse4;
}

/* Cold main to potentially affect block ordering */
COLD int main(int argc, char **argv) {
    int results[5] = {0};
    double float_result = 0.0;
    
    /* Use varying inputs to prevent constant folding */
    int base_input = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Engage CPU feature detection */
    int cpu_features = use_cpu_features();
    base_input += cpu_features;
    
    /* Call all pattern functions multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        results[0] += pattern_a_int_pressure(base_input + i);
        float_result += pattern_b_float_pressure((double)(base_input + i) * 0.5);
        results[1] += pattern_c_complex_cfg(base_input + i * 3);
        results[2] += pattern_d_vector_pressure(base_input + i * 2);
        results[3] += pattern_e_register_conflict(base_input + i * 5);
        
        /* Mix in some conditional execution */
        if (i % 3 == 0) {
            results[4] += pattern_a_int_pressure(results[0] % 100);
        } else if (i % 3 == 1) {
            results[4] += pattern_c_complex_cfg(results[1] % 200);
        } else {
            results[4] += pattern_e_register_conflict(results[2] % 300);
        }
    }
    
    /* Final computation using all results */
    int final_result = results[0] + results[1] + results[2] + results[3] + results[4];
    final_result += (int)float_result;
    
    /* Prevent dead code elimination of entire program */
    asm volatile ("" : : "r"(final_result));
    
    /* Optional: Print result to prevent optimization */
    if (argc > 2) {
        printf("Final result: %d (float: %f)\n", final_result, float_result);
    }
    
    return final_result % 256;
}
