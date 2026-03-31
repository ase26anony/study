/* test_mcf_coverage.c
 * 
 * This test program is designed to stress GCC's Min-Cost Flow register allocator
 * to trigger coverage of special node indices in the print_node function.
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

/* Pattern A: Integer arithmetic chain with 30+ variables in a loop
 * Forces spill decisions through high register pressure */
NOINLINE static int pattern_a_intensive(int input) {
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
    a5 = a4 / (input != 0 ? input : 1) + a1;
    a6 = a5 ^ a4 + input;
    a7 = a6 | a5 + input;
    a8 = a7 & a6 + input;
    a9 = a8 << 2 + input;
    a10 = a9 >> 1 + input;
    
    a11 = a10 + a9 + input;
    a12 = a11 - a10 + input;
    a13 = a12 * a11 + input;
    a14 = a13 % (input != 0 ? input : 13) + a12;
    a15 = a14 ^ a13 + input;
    a16 = a15 | a14 + input;
    a17 = a16 & a15 + input;
    a18 = a17 << 3 + input;
    a19 = a18 >> 2 + input;
    a20 = a19 + a18 + input;
    
    a21 = a20 * a19 + input;
    a22 = a21 - a20 + input;
    a23 = a22 * a21 + input;
    a24 = a23 / (input != 0 ? input : 17) + a22;
    a25 = a24 ^ a23 + input;
    a26 = a25 | a24 + input;
    a27 = a26 & a25 + input;
    a28 = a27 << 1 + input;
    a29 = a28 >> 1 + input;
    a30 = a29 + a28 + input;
    
    a31 = a30 * a29 + input;
    a32 = a31 - a30 + input;
    a33 = a32 * a31 + input;
    a34 = a33 % (input != 0 ? input : 19) + a32;
    a35 = a34 ^ a33 + input;
    
    /* Complex loop with interdependencies to force MCF analysis */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        /* Create complex data flow with all variables */
        a1 = a35 + i;
        a2 = a1 + a34;
        a3 = a2 + a33;
        a4 = a3 + a32;
        a5 = a4 + a31;
        a6 = a5 + a30;
        a7 = a6 + a29;
        a8 = a7 + a28;
        a9 = a8 + a27;
        a10 = a9 + a26;
        
        a11 = a10 + a25;
        a12 = a11 + a24;
        a13 = a12 + a23;
        a14 = a13 + a22;
        a15 = a14 + a21;
        a16 = a15 + a20;
        a17 = a16 + a19;
        a18 = a17 + a18; /* Self-reference to create complexity */
        a19 = a18 + a17;
        a20 = a19 + a16;
        
        a21 = a20 + a15;
        a22 = a21 + a14;
        a23 = a22 + a13;
        a24 = a23 + a12;
        a25 = a24 + a11;
        a26 = a25 + a10;
        a27 = a26 + a9;
        a28 = a27 + a8;
        a29 = a28 + a7;
        a30 = a29 + a6;
        
        a31 = a30 + a5;
        a32 = a31 + a4;
        a33 = a32 + a3;
        a34 = a33 + a2;
        a35 = a34 + a1;
        
        sum += a35;
        
        /* Conditional break to create control flow complexity */
        if (i == input % 50) {
            a1 += 1000; /* Modify variable to affect data flow */
        }
        
        /* Continue with different condition */
        if (i % 7 == 0) {
            a2 *= 2;
            continue;
        }
    }
    
    /* Final computation using all variables */
    return a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
           a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19 + a20 +
           a21 + a22 + a23 + a24 + a25 + a26 + a27 + a28 + a29 + a30 +
           a31 + a32 + a33 + a34 + a35 + sum + input;
}

/* Pattern B: Floating-point intensive computation
 * Pressures floating-point register class */
NOINLINE static double pattern_b_float_pressure(int seed) {
    /* Declare many double variables */
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    double d11, d12, d13, d14, d15, d16, d17, d18, d19, d20;
    double d21, d22, d23, d24, d25;
    
    /* Initialize with trigonometric functions to prevent optimization */
    d1 = seed * 0.01;
    d2 = d1 * d1;
    d3 = d2 * d1;
    d4 = d3 / (d1 != 0.0 ? d1 : 1.0);
    d5 = d4 + d3;
    d6 = d5 - d4;
    d7 = d6 * d5;
    d8 = d7 / (d2 != 0.0 ? d2 : 1.0);
    d9 = d8 + d7;
    d10 = d9 - d8;
    
    d11 = d10 * d9;
    d12 = d11 / (d3 != 0.0 ? d3 : 1.0);
    d13 = d12 + d11;
    d14 = d13 - d12;
    d15 = d14 * d13;
    d16 = d15 / (d4 != 0.0 ? d4 : 1.0);
    d17 = d16 + d15;
    d18 = d17 - d16;
    d19 = d18 * d17;
    d20 = d19 / (d5 != 0.0 ? d5 : 1.0);
    
    d21 = d20 + d19;
    d22 = d21 - d20;
    d23 = d22 * d21;
    d24 = d23 / (d6 != 0.0 ? d6 : 1.0);
    d25 = d24 + d23;
    
    /* Nested loops with floating-point operations */
    double result = 0.0;
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 10; j++) {
            /* Complex floating-point data flow */
            d1 = d25 * 0.99 + i * 0.1 + j * 0.01;
            d2 = d1 * d24;
            d3 = d2 + d23;
            d4 = d3 - d22;
            d5 = d4 * d21;
            d6 = d5 / (d20 != 0.0 ? d20 : 1.0);
            d7 = d6 + d19;
            d8 = d7 - d18;
            d9 = d8 * d17;
            d10 = d9 / (d16 != 0.0 ? d16 : 1.0);
            
            d11 = d10 + d15;
            d12 = d11 - d14;
            d13 = d12 * d13; /* Self-modification */
            d14 = d13 / (d12 != 0.0 ? d12 : 1.0);
            d15 = d14 + d11;
            d16 = d15 - d10;
            d17 = d16 * d9;
            d18 = d17 / (d8 != 0.0 ? d8 : 1.0);
            d19 = d18 + d7;
            d20 = d19 - d6;
            
            d21 = d20 * d5;
            d22 = d21 / (d4 != 0.0 ? d4 : 1.0);
            d23 = d22 + d3;
            d24 = d23 - d2;
            d25 = d24 * d1;
            
            result += d25;
            
            /* Conditional with floating-point comparison */
            if (d25 > 1000.0) {
                d1 /= 2.0;
                continue;
            }
            
            if (j % 3 == 0) {
                d2 *= 1.5;
                break; /* Creates additional control flow edges */
            }
        }
    }
    
    return result + d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10 +
           d11 + d12 + d13 + d14 + d15 + d16 + d17 + d18 + d19 + d20 +
           d21 + d22 + d23 + d24 + d25;
}

/* Pattern C: Complex control flow with switch statement
 * Creates many basic blocks for MCF to analyze */
NOINLINE static int pattern_c_complex_cfg(int input) {
    int result = input;
    
    /* Loop with switch inside to create complex CFG */
    for (int i = 0; i < 100; i++) {
        switch (i % 23) { /* Prime number to reduce pattern recognition */
            case 0: result += i * 2; break;
            case 1: result -= i * 3; break;
            case 2: result *= (i + 1); break;
            case 3: result /= (i != 0 ? i : 1); break;
            case 4: result ^= i; break;
            case 5: result |= i << 2; break;
            case 6: result &= ~i; break;
            case 7: result = result << (i % 4); break;
            case 8: result = result >> (i % 4); break;
            case 9: result += result * i; break;
            case 10: result -= result / (i != 0 ? i : 1); break;
            case 11: result *= result % (i + 2); break;
            case 12: result ^= result | i; break;
            case 13: result |= result & i; break;
            case 14: result &= result ^ i; break;
            case 15: result = ~result + i; break;
            case 16: result = result + (i << 3); break;
            case 17: result = result - (i >> 1); break;
            case 18: result = result * (i % 7 + 1); break;
            case 19: result = result / (i % 5 + 1); break;
            case 20: result = result ^ (i << 4); break;
            case 21: result = result | (i << 5); break;
            case 22: result = result & (i << 6); break;
        }
        
        /* Additional control flow with breaks and continues */
        if (i % 11 == 0) {
            result += 1000;
            continue;
        }
        
        if (i % 13 == 0) {
            result -= 500;
            break; /* Early exit from loop */
        }
        
        if (i % 17 == 0) {
            result *= 2;
            /* Nested loop for additional complexity */
            for (int j = 0; j < 5; j++) {
                result += j;
                if (j % 2 == 0) continue;
                result -= j * 2;
            }
        }
    }
    
    return result;
}

/* Pattern D: Vector extensions for SIMD register pressure */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

NOINLINE static int pattern_d_vector_ops(int input) {
#ifdef __GNUC__
    /* Declare many vector variables */
    v4si v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    v4sf f1, f2, f3, f4, f5;
    
    /* Initialize integer vectors */
    v1 = (v4si){input, input + 1, input + 2, input + 3};
    v2 = v1 + (v4si){1, 2, 3, 4};
    v3 = v2 - v1;
    v4 = v3 * v2;
    v5 = v4 + v3;
    v6 = v5 - v4;
    v7 = v6 * v5;
    v8 = v7 + v6;
    v9 = v8 - v7;
    v10 = v9 * v8;
    
    /* Initialize float vectors */
    f1 = (v4sf){input * 0.1f, input * 0.2f, input * 0.3f, input * 0.4f};
    f2 = f1 * (v4sf){1.1f, 1.2f, 1.3f, 1.4f};
    f3 = f2 - f1;
    f4 = f3 * f2;
    f5 = f4 + f3;
    
    /* Mixed vector operations in a loop */
    int sum = 0;
    for (int i = 0; i < 50; i++) {
        /* Integer vector operations */
        v1 = v10 + (v4si){i, i+1, i+2, i+3};
        v2 = v1 * v9;
        v3 = v2 - v8;
        v4 = v3 * v7;
        v5 = v4 + v6;
        v6 = v5 - v5; /* Creates zero vector */
        v7 = v6 + v4;
        v8 = v7 * v3;
        v9 = v8 - v2;
        v10 = v9 * v1;
        
        /* Float vector operations */
        f1 = f5 * (v4sf){0.9f, 0.8f, 0.7f, 0.6f};
        f2 = f1 + f4;
        f3 = f2 - f3;
        f4 = f3 * f2;
        f5 = f4 + f1;
        
        /* Extract and sum elements to prevent dead code elimination */
        int* vi = (int*)&v10;
        sum += vi[0] + vi[1] + vi[2] + vi[3];
        
        float* vf = (float*)&f5;
        sum += (int)(vf[0] + vf[1] + vf[2] + vf[3]);
        
        /* Conditional with vector comparison */
        if (i % 7 == 0) {
            v1 = (v4si){0, 0, 0, 0};
            continue;
        }
    }
    
    return sum + input;
#else
    /* Fallback for non-GCC compilers */
    return input * 2;
#endif
}

/* Pattern E: Explicit register variables to conflict with allocator */
NOINLINE static int pattern_e_register_conflict(int input) {
    /* Declare explicit register variables */
    register int r1 asm ("r12") = input + 1;
    register int r2 asm ("r13") = input + 2;
    register int r3 asm ("r14") = input + 3;
    register int r4 asm ("r15") = input + 4;
    
    /* Force these to be used extensively */
    int a = 0, b = 0, c = 0, d = 0, e = 0, f = 0, g = 0, h = 0;
    int i = 0, j = 0, k = 0, l = 0, m = 0, n = 0, o = 0, p = 0;
    
    /* Complex computation mixing register and stack variables */
    for (int iter = 0; iter < 100; iter++) {
        a = r1 + iter;
        b = r2 + a;
        c = r3 + b;
        d = r4 + c;
        
        e = a * b;
        f = b * c;
        g = c * d;
        h = d * a;
        
        i = e + f;
        j = f + g;
        k = g + h;
        l = h + e;
        
        m = i * j;
        n = j * k;
        o = k * l;
        p = l * i;
        
        /* Update register variables */
        r1 = m % 256;
        r2 = n % 256;
        r3 = o % 256;
        r4 = p % 256;
        
        /* Use asm to prevent optimization */
        asm volatile ("" : : "r"(r1), "r"(r2), "r"(r3), "r"(r4));
        
        /* Complex control flow with computed goto (GCC extension) */
        if (iter % 13 == 0) {
            static void* labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4 };
            goto *labels[iter % 5];
            
            L0: a += 100; goto END;
            L1: b += 200; goto END;
            L2: c += 300; goto END;
            L3: d += 400; goto END;
            L4: e += 500; goto END;
            END:;
        }
    }
    
    /* Final result using all variables */
    return r1 + r2 + r3 + r4 + a + b + c + d + e + f + g + h + 
           i + j + k + l + m + n + o + p + input;
}

/* Helper to prevent dead code elimination */
static void use_result(int result) {
    /* Use volatile asm to ensure result is used but not optimized away */
    asm volatile ("" : : "r"(result));
}

/* Main function marked as cold to potentially affect block ordering */
COLD int main(int argc, char** argv) {
    int total = 0;
    
    /* Use CPU feature detection to engage target-specific optimizations */
#ifdef __GNUC__
    if (__builtin_cpu_supports("avx2")) {
        total += 1000000; /* Bonus for AVX2 support */
    }
    if (__builtin_cpu_supports("sse4.2")) {
        total += 500000;
    }
#endif
    
    /* Array of inputs to prevent constant folding */
    int inputs[] = {1, 2, 3, 5, 7, 11, 13, 17, 19, 23};
    int num_inputs = sizeof(inputs) / sizeof(inputs[0]);
    
    /* Call each pattern function with different inputs */
    for (int i = 0; i < num_inputs; i++) {
        int input = inputs[i];
        
        /* Pattern A: Integer pressure */
        int result_a = pattern_a_intensive(input);
        use_result(result_a);
        total += result_a;
        
        /* Pattern B: Floating-point pressure */
        double result_b = pattern_b_float_pressure(input);
        use_result((int)result_b);
        total += (int)result_b;
        
        /* Pattern C: Complex CFG */
        int result_c = pattern_c_complex_cfg(input);
        use_result(result_c);
        total += result_c;
        
        /* Pattern D: Vector operations */
        int result_d = pattern_d_vector_ops(input);
        use_result(result_d);
        total += result_d;
        
        /* Pattern E: Register conflict */
        int result_e = pattern_e_register_conflict(input);
        use_result(result_e);
        total += result_e;
    }
    
    /* Print result to prevent entire program from being optimized away */
    printf("Total: %d\n", total % 1000000);
    
    return 0;
}
