/* test_mcf_coverage.c
 * 
 * This test program is designed to trigger GCC's Min-Cost Flow pass
 * during register allocation, specifically exercising the print_node
 * function with special node indices like ENTRY_BLOCK, EXIT_BLOCK,
 * new_exit_index, and new_entry_index.
 *
 * Compile with: gcc -O3 -fprofile-arcs -ftest-coverage -fdump-rtl-all test_mcf_coverage.c -o test_mcf
 * Run with: ./test_mcf
 * Generate coverage: gcov test_mcf_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))
#define COLD __attribute__((cold))

/* Stub for non-GCC compilers */
#ifndef __GNUC__
#define __builtin_cpu_supports(x) 0
#endif

/* Pattern A: Integer arithmetic chain with 30+ variables in a loop */
NOINLINE static long pattern_a(int input) {
    /* 30+ integer variables to create register pressure */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int a11, a12, a13, a14, a15, a16, a17, a18, a19, a20;
    int a21, a22, a23, a24, a25, a26, a27, a28, a29, a30;
    int a31, a32, a33, a34, a35;
    long result = 0;
    
    /* Complex initialization chain */
    a1 = input + 1;
    a2 = a1 * 2;
    a3 = a2 - a1;
    a4 = a3 + input;
    a5 = a4 * a3;
    a6 = a5 / (a1 ? a1 : 1);
    a7 = a6 << 2;
    a8 = a7 ^ a6;
    a9 = a8 | a7;
    a10 = a9 & a8;
    
    a11 = a10 + a9;
    a12 = a11 - a10;
    a13 = a12 * a11;
    a14 = a13 / (a12 ? a12 : 1);
    a15 = a14 << 1;
    a16 = a15 ^ a14;
    a17 = a16 | a15;
    a18 = a17 & a16;
    a19 = a18 + a17;
    a20 = a19 - a18;
    
    a21 = a20 * a19;
    a22 = a21 / (a20 ? a20 : 1);
    a23 = a22 << 3;
    a24 = a23 ^ a22;
    a25 = a24 | a23;
    a26 = a25 & a24;
    a27 = a26 + a25;
    a28 = a27 - a26;
    a29 = a28 * a27;
    a30 = a29 / (a28 ? a28 : 1);
    
    a31 = a30 << 2;
    a32 = a31 ^ a30;
    a33 = a32 | a31;
    a34 = a33 & a32;
    a35 = a34 + a33;
    
    /* Tight interdependent loop to force spill decisions */
    for (int i = 0; i < 1000; i++) {
        a1 = a35 + i;
        a2 = a1 * a34;
        a3 = a2 - a33;
        a4 = a3 + a32;
        a5 = a4 * a31;
        a6 = a5 / (a30 ? a30 : 1);
        a7 = a6 << (i & 3);
        a8 = a7 ^ a29;
        a9 = a8 | a28;
        a10 = a9 & a27;
        
        /* Use asm volatile to prevent elimination */
        asm volatile("" : "+r"(a1), "+r"(a2), "+r"(a3), "+r"(a4), "+r"(a5));
        asm volatile("" : "+r"(a6), "+r"(a7), "+r"(a8), "+r"(a9), "+r"(a10));
        
        /* More computations to increase pressure */
        a11 = a10 + a26;
        a12 = a11 - a25;
        a13 = a12 * a24;
        a14 = a13 / (a23 ? a23 : 1);
        a15 = a14 << (i & 1);
        a16 = a15 ^ a22;
        a17 = a16 | a21;
        a18 = a17 & a20;
        a19 = a18 + a19;  /* Self-modification */
        a20 = a19 - a18;
        
        a21 = a20 * a17;
        a22 = a21 / (a16 ? a16 : 1);
        a23 = a22 << (i & 2);
        a24 = a23 ^ a15;
        a25 = a24 | a14;
        a26 = a25 & a13;
        a27 = a26 + a12;
        a28 = a27 - a11;
        a29 = a28 * a10;
        a30 = a29 / (a9 ? a9 : 1);
        
        a31 = a30 << (i & 3);
        a32 = a31 ^ a8;
        a33 = a32 | a7;
        a34 = a33 & a6;
        a35 = a34 + a5;
        
        result += a35;
    }
    
    return result + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
           a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19 + a20 +
           a21 + a22 + a23 + a24 + a25 + a26 + a27 + a28 + a29 + a30 +
           a31 + a32 + a33 + a34 + a35;
}

/* Pattern B: Floating-point intensive computation */
NOINLINE static double pattern_b(double input) {
    /* 20+ double variables to pressure FP registers */
    double b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    double b11, b12, b13, b14, b15, b16, b17, b18, b19, b20;
    double b21, b22, b23, b24, b25;
    double result = 0.0;
    
    b1 = input + 1.0;
    b2 = b1 * 2.0;
    b3 = b2 - b1;
    b4 = b3 + input;
    b5 = b4 * b3;
    b6 = b5 / (b1 != 0.0 ? b1 : 1.0);
    b7 = b6 * 3.14159;
    b8 = b7 / 2.71828;
    b9 = b8 + b7;
    b10 = b9 - b8;
    
    b11 = b10 * b9;
    b12 = b11 / (b10 != 0.0 ? b10 : 1.0);
    b13 = b12 * 1.41421;
    b14 = b13 / 1.73205;
    b15 = b14 + b13;
    b16 = b15 - b14;
    b17 = b16 * b15;
    b18 = b17 / (b16 != 0.0 ? b16 : 1.0);
    b19 = b18 * 2.23607;
    b20 = b19 / 2.64575;
    
    b21 = b20 + b19;
    b22 = b21 - b20;
    b23 = b22 * b21;
    b24 = b23 / (b22 != 0.0 ? b22 : 1.0);
    b25 = b24 * 3.31662;
    
    /* Complex FP loop with many live variables */
    for (int i = 0; i < 500; i++) {
        double t = (double)i;
        b1 = b25 + t;
        b2 = b1 * b24;
        b3 = b2 - b23;
        b4 = b3 + b22;
        b5 = b4 * b21;
        b6 = b5 / (b20 != 0.0 ? b20 : 1.0);
        b7 = b6 * (t + 1.0);
        b8 = b7 / (t + 2.0);
        b9 = b8 + b19;
        b10 = b9 - b18;
        
        b11 = b10 * b17;
        b12 = b11 / (b16 != 0.0 ? b16 : 1.0);
        b13 = b12 * (t * 0.5);
        b14 = b13 / (t * 0.25 + 1.0);
        b15 = b14 + b15;  /* Self-modification */
        b16 = b15 - b14;
        
        b17 = b16 * b13;
        b18 = b17 / (b12 != 0.0 ? b12 : 1.0);
        b19 = b18 * (t * 0.3);
        b20 = b19 / (t * 0.7 + 1.0);
        
        b21 = b20 + b11;
        b22 = b21 - b10;
        b23 = b22 * b9;
        b24 = b23 / (b8 != 0.0 ? b8 : 1.0);
        b25 = b24 * (t * 0.9);
        
        result += b25;
        
        /* Prevent dead code elimination */
        asm volatile("" : "+x"(b1), "+x"(b2), "+x"(b3), "+x"(b4), "+x"(b5));
        asm volatile("" : "+x"(b6), "+x"(b7), "+x"(b8), "+x"(b9), "+x"(b10));
    }
    
    return result + b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9 + b10 +
           b11 + b12 + b13 + b14 + b15 + b16 + b17 + b18 + b19 + b20 +
           b21 + b22 + b23 + b24 + b25;
}

/* Pattern C: Complex control flow with switch statement */
NOINLINE static int pattern_c(int input) {
    int result = input;
    
    /* Many small basic blocks from switch */
    for (int i = 0; i < 100; i++) {
        switch ((input + i) % 25) {
            case 0: result += i * 2; break;
            case 1: result -= i * 3; break;
            case 2: result *= (i + 1); break;
            case 3: result /= (i ? i : 1); break;
            case 4: result ^= i; break;
            case 5: result |= i << 1; break;
            case 6: result &= ~i; break;
            case 7: result = result << (i & 3); break;
            case 8: result = result >> (i & 3); break;
            case 9: result += result * i; break;
            case 10: result -= result / (i ? i : 1); break;
            case 11: result = ~result; break;
            case 12: result = result + (i << 2); break;
            case 13: result = result - (i << 1); break;
            case 14: result = result * (i + 2); break;
            case 15: result = result / ((i & 7) + 1); break;
            case 16: result = result ^ (i << 3); break;
            case 17: result = result | (i << 4); break;
            case 18: result = result & (i << 5); break;
            case 19: result = result << ((i & 1) + 1); break;
            case 20: result = result >> ((i & 2) + 1); break;
            case 21: result = result + (i * i); break;
            case 22: result = result - (i * 3); break;
            case 23: result = result * (i * 2); break;
            case 24: result = result / ((i & 3) + 1); break;
        }
        
        /* Nested loop with break/continue to complicate CFG */
        for (int j = 0; j < 10; j++) {
            if (j == 5) continue;
            if (j == 8) break;
            result += j;
            
            /* Another level of control flow */
            if (result > 1000) {
                result -= 500;
                if (result < 0) {
                    result = 0;
                    continue;
                }
            } else if (result < -1000) {
                result += 500;
                if (result > 0) {
                    result = 0;
                    break;
                }
            }
        }
    }
    
    return result;
}

/* Pattern D: Vector extensions for SIMD register pressure */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
#endif

NOINLINE static long pattern_d(int input) {
#ifdef __GNUC__
    /* Multiple vector variables */
    v4si v1 = {input, input + 1, input + 2, input + 3};
    v4si v2 = {input + 4, input + 5, input + 6, input + 7};
    v4si v3 = {input + 8, input + 9, input + 10, input + 11};
    v4si v4 = {input + 12, input + 13, input + 14, input + 15};
    v4si v5 = {input + 16, input + 17, input + 18, input + 19};
    v4si v6 = {input + 20, input + 21, input + 22, input + 23};
    v4si v7 = {input + 24, input + 25, input + 26, input + 27};
    v4si v8 = {input + 28, input + 29, input + 30, input + 31};
    
    v4sf f1 = {input * 0.1f, input * 0.2f, input * 0.3f, input * 0.4f};
    v4sf f2 = {input * 0.5f, input * 0.6f, input * 0.7f, input * 0.8f};
    v4sf f3 = {input * 0.9f, input * 1.0f, input * 1.1f, input * 1.2f};
    v4sf f4 = {input * 1.3f, input * 1.4f, input * 1.5f, input * 1.6f};
    
    v2df d1 = {input * 0.01, input * 0.02};
    v2df d2 = {input * 0.03, input * 0.04};
    v2df d3 = {input * 0.05, input * 0.06};
    v2df d4 = {input * 0.07, input * 0.08};
    
    long result = 0;
    
    /* Vector operations in loop */
    for (int i = 0; i < 200; i++) {
        v1 = v1 + v2;
        v2 = v2 - v3;
        v3 = v3 * v4;
        v4 = v4 & v5;
        v5 = v5 | v6;
        v6 = v6 ^ v7;
        v7 = v7 << (i & 3);
        v8 = v8 >> (i & 3);
        
        f1 = f1 + f2;
        f2 = f2 - f3;
        f3 = f3 * f4;
        f4 = f4 / (f1 + 1.0f);
        
        d1 = d1 + d2;
        d2 = d2 - d3;
        d3 = d3 * d4;
        d4 = d4 / (d1 + 1.0);
        
        /* Mix vector types */
        v1 = v1 + (v4si){i, i, i, i};
        f1 = f1 + (v4sf){i * 0.1f, i * 0.1f, i * 0.1f, i * 0.1f};
        d1 = d1 + (v2df){i * 0.01, i * 0.01};
        
        /* Prevent elimination */
        asm volatile("" : "+x"(v1), "+x"(v2), "+x"(v3), "+x"(v4));
        asm volatile("" : "+x"(v5), "+x"(v6), "+x"(v7), "+x"(v8));
        asm volatile("" : "+x"(f1), "+x"(f2), "+x"(f3), "+x"(f4));
        asm volatile("" : "+x"(d1), "+x"(d2), "+x"(d3), "+x"(d4));
        
        /* Extract and accumulate */
        int arr[4];
        __builtin_memcpy(arr, &v1, sizeof(v1));
        result += arr[0] + arr[1] + arr[2] + arr[3];
    }
    
    return result;
#else
    return input;  /* Non-GCC fallback */
#endif
}

/* Pattern E: Explicit register variables to conflict with allocator */
NOINLINE static int pattern_e(int input) {
    /* Explicit register variables that may conflict */
    register int r1 asm ("r12") = input;
    register int r2 asm ("r13") = input + 1;
    register int r3 asm ("r14") = input + 2;
    register int r4 asm ("r15") = input + 3;
    
    int a = 0, b = 0, c = 0, d = 0, e = 0, f = 0, g = 0, h = 0;
    int i = 0, j = 0, k = 0, l = 0, m = 0, n = 0, o = 0, p = 0;
    
    /* Force use of explicit registers alongside many other variables */
    for (int iter = 0; iter < 300; iter++) {
        a = r1 + iter;
        b = r2 - iter;
        c = r3 * iter;
        d = r4 / (iter ? iter : 1);
        
        e = a + b;
        f = c - d;
        g = e * f;
        h = g / (e ? e : 1);
        
        i = h + r1;
        j = i - r2;
        k = j * r3;
        l = k / (r4 ? r4 : 1);
        
        m = l + a;
        n = m - b;
        o = n * c;
        p = o / (d ? d : 1);
        
        /* Update register variables */
        r1 = p + 1;
        r2 = r1 * 2;
        r3 = r2 - r1;
        r4 = r3 + p;
        
        /* Force spills by using many variables */
        asm volatile("" : "+r"(r1), "+r"(r2), "+r"(r3), "+r"(r4));
        asm volatile("" : "+r"(a), "+r"(b), "+r"(c), "+r"(d));
        asm volatile("" : "+r"(e), "+r"(f), "+r"(g), "+r"(h));
        asm volatile("" : "+r"(i), "+r"(j), "+r"(k), "+r"(l));
        asm volatile("" : "+r"(m), "+r"(n), "+r"(o), "+r"(p));
    }
    
    return r1 + r2 + r3 + r4 + a + b + c + d + e + f + g + h +
           i + j + k + l + m + n + o + p;
}

/* Main function to drive all patterns */
COLD int main(int argc, char **argv) {
    long total = 0;
    int iterations = 10;
    
    /* Use CPU feature check to engage target-specific optimizations */
    if (__builtin_cpu_supports("avx2")) {
        iterations += 5;  /* More iterations if AVX2 available */
    }
    
    /* Array of inputs to prevent constant folding */
    int inputs[] = {1, 2, 3, 5, 7, 11, 13, 17, 19, 23};
    double fp_inputs[] = {1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1};
    
    /* Call each pattern multiple times with different inputs */
    for (int i = 0; i < iterations; i++) {
        int idx = i % 10;
        
        total += pattern_a(inputs[idx]);
        total += (long)pattern_b(fp_inputs[idx]);
        total += pattern_c(inputs[idx]);
        total += pattern_d(inputs[idx]);
        total += pattern_e(inputs[idx]);
        
        /* Mix up inputs */
        inputs[idx] = (inputs[idx] * 13 + 7) % 100;
        fp_inputs[idx] = fp_inputs[idx] * 1.1 + 0.5;
    }
    
    /* Optional debug output (doesn't affect coverage) */
    if (argc > 1) {
        printf("Total: %ld\n", total);
    }
    
    return (int)(total % 1000);
}
