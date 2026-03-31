/* test_mcf_coverage.c
 * 
 * This test program is designed to trigger GCC's Min-Cost Flow pass
 * during register allocation, specifically exercising the print_node
 * function with special node indices (ENTRY_BLOCK, EXIT_BLOCK,
 * new_exit_index, new_entry_index).
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

/* Vector type for SIMD pressure */
typedef int v4si __attribute__((vector_size(16)));

/* Stub for non-GCC compilers */
#ifndef __GNUC__
#define __builtin_cpu_supports(x) 0
#endif

/* Pattern A: Integer arithmetic chain with 30+ variables in a loop
 * Creates massive register pressure for integer registers */
NOINLINE static int pattern_a_int_pressure(int input) {
    /* Declare 30+ integer variables to pressure registers */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int a11, a12, a13, a14, a15, a16, a17, a18, a19, a20;
    int a21, a22, a23, a24, a25, a26, a27, a28, a29, a30;
    int a31, a32, a33, a34, a35;
    
    /* Initialize with input to prevent constant folding */
    a1 = input;
    
    /* Complex interdependent chain of operations */
    for (int i = 0; i < 100; i++) {
        a2 = a1 + i;
        a3 = a2 * a1;
        a4 = a3 - a2;
        a5 = a4 ^ a3;
        a6 = a5 | a4;
        a7 = a6 & a5;
        a8 = a7 << 2;
        a9 = a8 >> 1;
        a10 = a9 + a8;
        a11 = a10 - a9;
        a12 = a11 * a10;
        a13 = a12 / (a11 + 1);
        a14 = a13 % (a12 + 1);
        a15 = a14 ^ a13;
        a16 = a15 | a14;
        a17 = a16 & a15;
        a18 = a17 << 3;
        a19 = a18 >> 2;
        a20 = a19 + a18;
        a21 = a20 - a19;
        a22 = a21 * a20;
        a23 = a22 / (a21 + 1);
        a24 = a23 % (a22 + 1);
        a25 = a24 ^ a23;
        a26 = a25 | a24;
        a27 = a26 & a25;
        a28 = a27 << 1;
        a29 = a28 >> 1;
        a30 = a29 + a28;
        a31 = a30 - a29;
        a32 = a31 * a30;
        a33 = a32 / (a31 + 1);
        a34 = a33 % (a32 + 1);
        a35 = a34 ^ a33;
        
        /* Use all variables to prevent dead code elimination */
        a1 = a35 + i;
        
        /* Complex control flow with break/continue */
        if (i % 7 == 0) continue;
        if (i % 13 == 0) break;
        if (i % 17 == 0) {
            a1 = a2 + a3;
            continue;
        }
    }
    
    /* Return a complex expression using many variables */
    return a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
           a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19 + a20 +
           a21 + a22 + a23 + a24 + a25 + a26 + a27 + a28 + a29 + a30 +
           a31 + a32 + a33 + a34 + a35;
}

/* Pattern B: Floating-point intensive computation
 * Pressures floating-point registers */
NOINLINE static double pattern_b_float_pressure(double input) {
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    double d11, d12, d13, d14, d15, d16, d17, d18, d19, d20;
    double d21, d22, d23, d24, d25;
    
    d1 = input;
    
    for (int i = 0; i < 50; i++) {
        d2 = d1 * 1.1;
        d3 = d2 / 1.2;
        d4 = d3 + d2;
        d5 = d4 - d3;
        d6 = d5 * 0.9;
        d7 = d6 / 0.8;
        d8 = d7 + d6;
        d9 = d8 - d7;
        d10 = d9 * 1.3;
        d11 = d10 / 1.4;
        d12 = d11 + d10;
        d13 = d12 - d11;
        d14 = d13 * 0.7;
        d15 = d14 / 0.6;
        d16 = d15 + d14;
        d17 = d16 - d15;
        d18 = d17 * 1.5;
        d19 = d18 / 1.6;
        d20 = d19 + d18;
        d21 = d20 - d19;
        d22 = d21 * 0.5;
        d23 = d22 / 0.4;
        d24 = d23 + d22;
        d25 = d24 - d23;
        
        d1 = d25 * (i + 1);
        
        /* Mix with integer operations */
        if ((int)d1 % 11 == 0) continue;
        if ((int)d1 % 19 == 0) break;
    }
    
    return d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10 +
           d11 + d12 + d13 + d14 + d15 + d16 + d17 + d18 + d19 + d20 +
           d21 + d22 + d23 + d24 + d25;
}

/* Pattern C: Complex control flow with switch statement
 * Creates many basic blocks for CFG complexity */
NOINLINE static int pattern_c_cfg_complexity(int input) {
    int result = input;
    
    for (int i = 0; i < 100; i++) {
        /* Large switch with 20+ cases */
        switch (i % 23) {
            case 0: result += i * 2; break;
            case 1: result -= i * 3; break;
            case 2: result ^= i; break;
            case 3: result |= 0xFF; break;
            case 4: result &= 0xF0; break;
            case 5: result <<= 2; break;
            case 6: result >>= 1; break;
            case 7: result = ~result; break;
            case 8: result += result * 2; break;
            case 9: result -= result / 2; break;
            case 10: result ^= 0xAAAA; break;
            case 11: result |= 0x5555; break;
            case 12: result &= 0x3333; break;
            case 13: result <<= 3; break;
            case 14: result >>= 2; break;
            case 15: result = -result; break;
            case 16: result += 0x1000; break;
            case 17: result -= 0x2000; break;
            case 18: result ^= 0xCCCC; break;
            case 19: result |= 0x9999; break;
            case 20: result &= 0x6666; break;
            case 21: result <<= 1; break;
            case 22: result >>= 3; break;
            default: result = 0; break;
        }
        
        /* Nested loops with breaks */
        for (int j = 0; j < 10; j++) {
            if (j % 3 == 0) continue;
            if (j == 7) break;
            result += j;
            
            for (int k = 0; k < 5; k++) {
                if (k == 2) continue;
                if (k == 4) break;
                result ^= k;
            }
        }
    }
    
    return result;
}

/* Pattern D: SIMD vector operations
 * Pressures vector/SIMD registers */
NOINLINE static v4si pattern_d_simd_pressure(v4si input) {
    v4si v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    v4si v11, v12, v13, v14, v15;
    
    v1 = input;
    v4si inc = {1, 2, 3, 4};
    
    for (int i = 0; i < 50; i++) {
        v2 = v1 + inc;
        v3 = v2 * v1;
        v4 = v3 - v2;
        v5 = v4 ^ v3;
        v6 = v5 | v4;
        v7 = v6 & v5;
        v8 = v7 << 2;
        v9 = v8 >> 1;
        v10 = v9 + v8;
        v11 = v10 - v9;
        v12 = v11 * v10;
        v13 = v12 / (v11 + inc);
        v14 = v13 ^ v12;
        v15 = v14 | v13;
        
        v1 = v15 + inc;
        
        /* Prevent optimization */
        asm volatile("" : "+x"(v1) : : "memory");
    }
    
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15;
}

/* Pattern E: Explicit register variables causing conflicts */
NOINLINE static int pattern_e_register_conflict(int input) {
    /* Try to force specific register usage */
    register int r1 asm ("r12") = input;
    register int r2 asm ("r13") = input * 2;
    register int r3 asm ("r14") = input * 3;
    register int r4 asm ("r15") = input * 4;
    
    int a, b, c, d, e, f, g, h, i, j;
    int k, l, m, n, o, p, q, r, s, t;
    
    /* Complex computation mixing register and stack variables */
    for (int idx = 0; idx < 100; idx++) {
        a = r1 + idx;
        b = r2 + a;
        c = r3 + b;
        d = r4 + c;
        e = a * b;
        f = b * c;
        g = c * d;
        h = d * e;
        i = e * f;
        j = f * g;
        k = g * h;
        l = h * i;
        m = i * j;
        n = j * k;
        o = k * l;
        p = l * m;
        q = m * n;
        r = n * o;
        s = o * p;
        t = p * q;
        
        /* Update register variables */
        r1 = a + b;
        r2 = c + d;
        r3 = e + f;
        r4 = g + h;
        
        /* Use computed goto for extra CFG complexity (GCC extension) */
        void* labels[] = {&&L0, &&L1, &&L2, &&L3, &&L4};
        goto *labels[idx % 5];
        
        L0: a += 1; continue;
        L1: b += 2; continue;
        L2: c += 3; continue;
        L3: d += 4; continue;
        L4: e += 5; continue;
    }
    
    return r1 + r2 + r3 + r4 + a + b + c + d + e + f +
           g + h + i + j + k + l + m + n + o + p +
           q + r + s + t;
}

/* Main function that calls all patterns */
COLD int main(int argc, char** argv) {
    int total = 0;
    double ftotal = 0.0;
    
    /* Use CPU feature detection to engage target-specific optimizations */
    if (__builtin_cpu_supports("avx2")) {
        /* This may affect register allocation decisions */
        asm volatile("" ::: "memory");
    }
    
    /* Array of inputs to prevent constant folding */
    int inputs[] = {1, 2, 3, 5, 7, 11, 13, 17, 19, 23};
    double dinputs[] = {1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.0};
    v4si vinput = {1, 2, 3, 4};
    
    /* Call each pattern multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        total += pattern_a_int_pressure(inputs[i]);
        ftotal += pattern_b_float_pressure(dinputs[i]);
        total += pattern_c_cfg_complexity(inputs[i]);
        
        v4si vresult = pattern_d_simd_pressure(vinput);
        total += vresult[0] + vresult[1] + vresult[2] + vresult[3];
        
        total += pattern_e_register_conflict(inputs[i]);
        
        /* Modify inputs slightly */
        inputs[i] += i;
        dinputs[i] += i * 0.1;
        vinput[0]++; vinput[1]++; vinput[2]++; vinput[3]++;
    }
    
    /* Prevent dead code elimination */
    asm volatile("" : "+r"(total), "+r"(ftotal) : : "memory");
    
    /* Optional debug output (doesn't affect coverage) */
    if (argc > 1) {
        printf("Result: %d, %f\n", total, ftotal);
    }
    
    return total % 256;
}
