/* test_mcf_coverage.c
 * 
 * This test program stresses GCC's Min-Cost Flow register allocator
 * to trigger coverage of special node index printing in mcf.cc.
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

/* Pattern A: Integer arithmetic chain with 30+ variables in a loop */
NOINLINE static long pattern_a(int input) {
    /* 30+ integer variables to create register pressure */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int a11, a12, a13, a14, a15, a16, a17, a18, a19, a20;
    int a21, a22, a23, a24, a25, a26, a27, a28, a29, a30;
    long result = 0;
    
    /* Complex interdependent computations */
    a1 = input + 1;
    a2 = a1 * 2 - input;
    a3 = a2 + a1;
    a4 = a3 * 3 - a2;
    a5 = a4 + a3 - a1;
    a6 = a5 * 2 + a4;
    a7 = a6 - a5 + a3;
    a8 = a7 * 3 - a6;
    a9 = a8 + a7 - a5;
    a10 = a9 * 2 + a8;
    a11 = a10 - a9 + a7;
    a12 = a11 * 3 - a10;
    a13 = a12 + a11 - a9;
    a14 = a13 * 2 + a12;
    a15 = a14 - a13 + a11;
    a16 = a15 * 3 - a14;
    a17 = a16 + a15 - a13;
    a18 = a17 * 2 + a16;
    a19 = a18 - a17 + a15;
    a20 = a19 * 3 - a18;
    a21 = a20 + a19 - a17;
    a22 = a21 * 2 + a20;
    a23 = a22 - a21 + a19;
    a24 = a23 * 3 - a22;
    a25 = a24 + a23 - a21;
    a26 = a25 * 2 + a24;
    a27 = a26 - a25 + a23;
    a28 = a27 * 3 - a26;
    a29 = a28 + a27 - a25;
    a30 = a29 * 2 + a28;
    
    /* Use all variables in a loop to prevent optimization */
    for (int i = 0; i < 100; i++) {
        a1 += i;
        a2 += a1;
        a3 += a2;
        a4 += a3;
        a5 += a4;
        a6 += a5;
        a7 += a6;
        a8 += a7;
        a9 += a8;
        a10 += a9;
        a11 += a10;
        a12 += a11;
        a13 += a12;
        a14 += a13;
        a15 += a14;
        a16 += a15;
        a17 += a16;
        a18 += a17;
        a19 += a18;
        a20 += a19;
        a21 += a20;
        a22 += a21;
        a23 += a22;
        a24 += a23;
        a25 += a24;
        a26 += a25;
        a27 += a26;
        a28 += a27;
        a29 += a28;
        a30 += a29;
    }
    
    /* Force use of all variables */
    result = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
             a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19 + a20 +
             a21 + a22 + a23 + a24 + a25 + a26 + a27 + a28 + a29 + a30;
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(result));
    return result;
}

/* Pattern B: Floating-point intensive computation */
NOINLINE static double pattern_b(double input) {
    /* 20+ double variables to pressure FP registers */
    double b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    double b11, b12, b13, b14, b15, b16, b17, b18, b19, b20;
    double result;
    
    /* Complex FP computations */
    b1 = input + 1.0;
    b2 = b1 * 2.0 - input;
    b3 = b2 + b1;
    b4 = b3 * 3.0 - b2;
    b5 = b4 + b3 - b1;
    b6 = b5 * 2.0 + b4;
    b7 = b6 - b5 + b3;
    b8 = b7 * 3.0 - b6;
    b9 = b8 + b7 - b5;
    b10 = b9 * 2.0 + b8;
    b11 = b10 - b9 + b7;
    b12 = b11 * 3.0 - b10;
    b13 = b12 + b11 - b9;
    b14 = b13 * 2.0 + b12;
    b15 = b14 - b13 + b11;
    b16 = b15 * 3.0 - b14;
    b17 = b16 + b15 - b13;
    b18 = b17 * 2.0 + b16;
    b19 = b18 - b17 + b15;
    b20 = b19 * 3.0 - b18;
    
    /* Loop with FP operations */
    for (int i = 0; i < 50; i++) {
        double t = i * 0.1;
        b1 += t;
        b2 += b1 * t;
        b3 += b2 / (t + 1.0);
        b4 += b3 - t;
        b5 += b4 * t;
        b6 += b5 / (t + 1.0);
        b7 += b6 - t;
        b8 += b7 * t;
        b9 += b8 / (t + 1.0);
        b10 += b9 - t;
        b11 += b10 * t;
        b12 += b11 / (t + 1.0);
        b13 += b12 - t;
        b14 += b13 * t;
        b15 += b14 / (t + 1.0);
        b16 += b15 - t;
        b17 += b16 * t;
        b18 += b17 / (t + 1.0);
        b19 += b18 - t;
        b20 += b19 * t;
    }
    
    result = b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9 + b10 +
             b11 + b12 + b13 + b14 + b15 + b16 + b17 + b18 + b19 + b20;
    
    asm volatile ("" : : "r"(result));
    return result;
}

/* Pattern C: Complex control flow with switch and many basic blocks */
NOINLINE static int pattern_c(int input) {
    int result = input;
    
    /* Complex loop with switch creating many basic blocks */
    for (int i = 0; i < 100; i++) {
        switch (i % 20) {
            case 0: result += i * 2; break;
            case 1: result -= i * 3; break;
            case 2: result *= (i + 1); break;
            case 3: result /= (i + 2); break;
            case 4: result ^= i; break;
            case 5: result |= i << 2; break;
            case 6: result &= ~i; break;
            case 7: result = result << (i % 4); break;
            case 8: result = result >> (i % 4); break;
            case 9: result += result * i; break;
            case 10: result -= result / (i + 1); break;
            case 11: result = ~result; break;
            case 12: result = result ^ (i << 8); break;
            case 13: result = result | 0xFF; break;
            case 14: result = result & 0xFFFF; break;
            case 15: result = result + (i * i); break;
            case 16: result = result - (i / 2); break;
            case 17: result = result * 3; break;
            case 18: result = result / 2; break;
            case 19: result = result % 1000; break;
        }
        
        /* Nested loop with break/continue for more CFG complexity */
        for (int j = 0; j < 10; j++) {
            if (j == 5) continue;
            if (j == 8) break;
            result += j;
        }
    }
    
    asm volatile ("" : : "r"(result));
    return result;
}

/* Pattern D: Vector extensions for SIMD register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

NOINLINE static v4si pattern_d(v4si input) {
    v4si v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    v4si result;
    
    /* Initialize vectors */
    v1 = input + (v4si){1, 2, 3, 4};
    v2 = v1 * (v4si){2, 2, 2, 2};
    v3 = v2 + v1;
    v4 = v3 * (v4si){3, 3, 3, 3};
    v5 = v4 - v2;
    v6 = v5 + v3;
    v7 = v6 * (v4si){2, 2, 2, 2};
    v8 = v7 - v5;
    v9 = v8 + v6;
    v10 = v9 * (v4si){3, 3, 3, 3};
    
    /* Loop with vector operations */
    for (int i = 0; i < 50; i++) {
        v4si t = {i, i+1, i+2, i+3};
        v1 += t;
        v2 = v1 * t;
        v3 = v2 + t;
        v4 = v3 - t;
        v5 = v4 * t;
        v6 = v5 + t;
        v7 = v6 - t;
        v8 = v7 * t;
        v9 = v8 + t;
        v10 = v9 - t;
    }
    
    result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    
    /* Prevent optimization */
    asm volatile ("" : : "r"(result));
    return result;
}

/* Pattern E: Explicit register variables conflicting with allocator */
NOINLINE static int pattern_e(int input) {
    /* Try to use specific registers */
    register int r1 asm ("r12") = input;
    register int r2 asm ("r13") = input * 2;
    register int r3 asm ("r14") = input * 3;
    register int r4 asm ("r15") = input * 4;
    
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    
    /* Force spill decisions with many variables */
    a = r1 + 1;
    b = a + r2;
    c = b + r3;
    d = c + r4;
    e = d + r1;
    f = e + r2;
    g = f + r3;
    h = g + r4;
    i = h + r1;
    j = i + r2;
    k = j + r3;
    l = k + r4;
    m = l + r1;
    n = m + r2;
    o = n + r3;
    p = o + r4;
    q = p + r1;
    r = q + r2;
    s = r + r3;
    t = s + r4;
    
    /* Complex loop with register pressure */
    for (int x = 0; x < 100; x++) {
        r1 += x;
        r2 += r1;
        r3 += r2;
        r4 += r3;
        a += r4;
        b += a;
        c += b;
        d += c;
        e += d;
        f += e;
        g += f;
        h += g;
        i += h;
        j += i;
        k += j;
        l += k;
        m += l;
        n += m;
        o += n;
        p += o;
        q += p;
        r += q;
        s += r;
        t += s;
    }
    
    int result = r1 + r2 + r3 + r4 + a + b + c + d + e + f + g + h + i + j +
                 k + l + m + n + o + p + q + r + s + t;
    
    asm volatile ("" : : "r"(result));
    return result;
}

/* Pattern F: Mixed types and computed goto for extreme CFG complexity */
NOINLINE static int pattern_f(int input) {
    static void* labels[] = {
        &&label0, &&label1, &&label2, &&label3, &&label4,
        &&label5, &&label6, &&label7, &&label8, &&label9,
        &&label10, &&label11, &&label12, &&label13, &&label14,
        &&label15, &&label16, &&label17, &&label18, &&label19
    };
    
    int result = input;
    int counter = 0;
    
    /* Computed goto creates complex control flow */
    goto *labels[input % 20];
    
    label0: result += 1; goto next;
    label1: result += 2; goto next;
    label2: result += 3; goto next;
    label3: result += 4; goto next;
    label4: result += 5; goto next;
    label5: result += 6; goto next;
    label6: result += 7; goto next;
    label7: result += 8; goto next;
    label8: result += 9; goto next;
    label9: result += 10; goto next;
    label10: result += 11; goto next;
    label11: result += 12; goto next;
    label12: result += 13; goto next;
    label13: result += 14; goto next;
    label14: result += 15; goto next;
    label15: result += 16; goto next;
    label16: result += 17; goto next;
    label17: result += 18; goto next;
    label18: result += 19; goto next;
    label19: result += 20; goto next;
    
    next:
    /* Loop with mixed integer/float computations */
    for (int i = 0; i < 50; i++) {
        double dval = i * 0.5;
        float fval = i * 0.25f;
        int ival = i;
        
        /* Use all types to pressure different register classes */
        result += (int)dval;
        result += (int)fval;
        result += ival;
        
        /* Conditional with many variables */
        if (i % 3 == 0) {
            int t1 = result * 2;
            int t2 = t1 + i;
            int t3 = t2 * 3;
            int t4 = t3 - i;
            int t5 = t4 / 2;
            result = t5;
        } else if (i % 3 == 1) {
            double d1 = result * 1.5;
            double d2 = d1 + i;
            double d3 = d2 * 2.5;
            result = (int)d3;
        } else {
            float f1 = result * 1.25f;
            float f2 = f1 - i;
            float f3 = f2 / 1.75f;
            result = (int)f3;
        }
        
        counter++;
        if (counter > 100) break;
    }
    
    asm volatile ("" : : "r"(result));
    return result;
}

/* Helper to use CPU features - may affect register allocation */
static void use_cpu_features(void) {
#ifdef __GNUC__
    /* Check for various CPU features to engage target-specific optimizations */
    if (__builtin_cpu_supports("avx2")) {
        asm volatile ("# AVX2 supported" : : : );
    }
    if (__builtin_cpu_supports("sse4.2")) {
        asm volatile ("# SSE4.2 supported" : : : );
    }
    if (__builtin_cpu_supports("popcnt")) {
        asm volatile ("# POPCNT supported" : : : );
    }
#endif
}

COLD int main(int argc, char** argv) {
    long total = 0;
    int iterations = 10;
    
    use_cpu_features();
    
    /* Call all patterns with varying inputs to prevent constant folding */
    for (int i = 0; i < iterations; i++) {
        total += pattern_a(i);
        total += (long)pattern_b(i * 1.5);
        total += pattern_c(i);
        
        v4si vec_input = {i, i+1, i+2, i+3};
        v4si vec_result = pattern_d(vec_input);
        total += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
        
        total += pattern_e(i);
        total += pattern_f(i);
    }
    
    /* Print result to prevent complete optimization */
    printf("Result: %ld\n", total);
    
    return 0;
}
