/* test_mcf_coverage.c
 * 
 * This test program is designed to stress GCC's Min-Cost Flow register allocator
 * to trigger coverage of special node indices in the print_node function.
 * 
 * Compile with: gcc -O3 -fprofile-arcs -ftest-coverage -fdump-rtl-all test_mcf_coverage.c -o test_mcf
 * Run with: ./test_mcf
 * Generate coverage: gcov test_mcf_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure each function is compiled independently */
#define NOINLINE __attribute__((noinline))
#define COLD __attribute__((cold))

/* Pattern A: Integer arithmetic chain with 30+ variables in a loop */
NOINLINE static long pattern_a(int iterations, int seed) {
    /* 30+ integer variables to create register pressure */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int a11, a12, a13, a14, a15, a16, a17, a18, a19, a20;
    int a21, a22, a23, a24, a25, a26, a27, a28, a29, a30;
    int a31, a32, a33, a34, a35;
    long sum = 0;
    
    /* Initialize with seed to prevent constant folding */
    a1 = seed;
    
    /* Complex interdependent chain of operations */
    for (int i = 0; i < iterations; i++) {
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
        a13 = a12 / (a11 ? a11 : 1);
        a14 = a13 % (a12 ? a12 : 1);
        a15 = a14 ^ a13;
        a16 = a15 | a14;
        a17 = a16 & a15;
        a18 = a17 << 3;
        a19 = a18 >> 2;
        a20 = a19 + a18;
        a21 = a20 - a19;
        a22 = a21 * a20;
        a23 = a22 / (a21 ? a21 : 1);
        a24 = a23 % (a22 ? a22 : 1);
        a25 = a24 ^ a23;
        a26 = a25 | a24;
        a27 = a26 & a25;
        a28 = a27 << 1;
        a29 = a28 >> 1;
        a30 = a29 + a28;
        a31 = a30 - a29;
        a32 = a31 * a30;
        a33 = a32 / (a31 ? a31 : 1);
        a34 = a33 % (a32 ? a32 : 1);
        a35 = a34 ^ a33;
        
        /* Use all variables to prevent elimination */
        sum += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
               a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19 + a20 +
               a21 + a22 + a23 + a24 + a25 + a26 + a27 + a28 + a29 + a30 +
               a31 + a32 + a33 + a34 + a35;
        
        /* Update a1 for next iteration */
        a1 = (a35 + i) % 1000;
    }
    
    return sum;
}

/* Pattern B: Floating-point intensive computation */
NOINLINE static double pattern_b(int iterations, double seed) {
    /* 20+ double variables to pressure FP registers */
    double b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    double b11, b12, b13, b14, b15, b16, b17, b18, b19, b20;
    double b21, b22, b23, b24, b25;
    double sum = 0.0;
    
    b1 = seed;
    
    for (int i = 0; i < iterations; i++) {
        double fi = (double)i;
        b2 = b1 + fi;
        b3 = b2 * b1;
        b4 = b3 - b2;
        b5 = b4 / (b3 != 0.0 ? b3 : 1.0);
        b6 = b5 * b4;
        b7 = b6 + b5;
        b8 = b7 - b6;
        b9 = b8 * b7;
        b10 = b9 / (b8 != 0.0 ? b8 : 1.0);
        b11 = b10 + b9;
        b12 = b11 - b10;
        b13 = b12 * b11;
        b14 = b13 / (b12 != 0.0 ? b12 : 1.0);
        b15 = b14 + b13;
        b16 = b15 - b14;
        b17 = b16 * b15;
        b18 = b17 / (b16 != 0.0 ? b16 : 1.0);
        b19 = b18 + b17;
        b20 = b19 - b18;
        b21 = b20 * b19;
        b22 = b21 / (b20 != 0.0 ? b20 : 1.0);
        b23 = b22 + b21;
        b24 = b23 - b22;
        b25 = b24 * b23;
        
        sum += b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9 + b10 +
               b11 + b12 + b13 + b14 + b15 + b16 + b17 + b18 + b19 + b20 +
               b21 + b22 + b23 + b24 + b25;
        
        b1 = b25 * 0.99;
    }
    
    return sum;
}

/* Pattern C: Complex control flow with switch statement */
NOINLINE static int pattern_c(int iterations, int seed) {
    int result = seed;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex switch with many cases creates many basic blocks */
        switch (i % 20) {
            case 0: result += i * 2; break;
            case 1: result -= i * 3; break;
            case 2: result ^= i; break;
            case 3: result |= i << 1; break;
            case 4: result &= ~i; break;
            case 5: result = result * 7 + i; break;
            case 6: result = result / (i ? i : 1); break;
            case 7: result = result % (i ? i : 1); break;
            case 8: result = (result << 3) | i; break;
            case 9: result = (result >> 2) ^ i; break;
            case 10: result = -result + i; break;
            case 11: result = ~result & i; break;
            case 12: result = result | (i << 4); break;
            case 13: result = result ^ (i * 5); break;
            case 14: result = result + (i % 16); break;
            case 15: result = result - (i & 0xFF); break;
            case 16: result = result * 3 - i; break;
            case 17: result = result / 2 + i; break;
            case 18: result = result % 100 + i; break;
            case 19: result = (result << 1) | (i & 1); break;
        }
        
        /* Nested loop with break/continue for additional CFG complexity */
        for (int j = 0; j < 5; j++) {
            if (j == 2) continue;
            if (j == 4) break;
            result += j;
        }
    }
    
    return result;
}

/* Pattern D: Vector operations using GCC vector extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

NOINLINE static v4si pattern_d(int iterations, v4si seed) {
    v4si v1 = seed;
    v4si v2 = {2, 3, 4, 5};
    v4si v3 = {6, 7, 8, 9};
    v4si v4 = {10, 11, 12, 13};
    v4si v5 = {14, 15, 16, 17};
    v4si v6 = {18, 19, 20, 21};
    v4si v7 = {22, 23, 24, 25};
    v4si v8 = {26, 27, 28, 29};
    v4si v9 = {30, 31, 32, 33};
    v4si v10 = {34, 35, 36, 37};
    
    for (int i = 0; i < iterations; i++) {
        v4si vi = {i, i+1, i+2, i+3};
        
        /* Chain of vector operations */
        v1 = v1 + vi;
        v2 = v2 * v1;
        v3 = v3 - v2;
        v4 = v4 ^ v3;
        v5 = v5 | v4;
        v6 = v6 & v5;
        v7 = v7 << 2;
        v8 = v8 >> 1;
        v9 = v9 + v8;
        v10 = v10 - v9;
        
        /* Mix them up */
        v1 = v1 + v10;
        v2 = v2 * v9;
        v3 = v3 - v8;
        v4 = v4 ^ v7;
        v5 = v5 | v6;
    }
    
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
}

/* Pattern E: Explicit register variables to conflict with allocator */
NOINLINE static int pattern_e(int iterations, int seed) {
    /* Explicit register variables that may conflict with allocator's choices */
    register int r1 asm ("r12") = seed;
    register int r2 asm ("r13") = seed + 1;
    register int r3 asm ("r14") = seed + 2;
    register int r4 asm ("r15") = seed + 3;
    
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Mix explicit and automatic variables */
        r1 = r1 + a;
        r2 = r2 * b;
        r3 = r3 - c;
        r4 = r4 ^ d;
        
        a = a + r1;
        b = b * r2;
        c = c - r3;
        d = d ^ r4;
        
        e = e + a;
        f = f * b;
        g = g - c;
        h = h ^ d;
        
        i = i + e;
        j = j * f;
        k = k - g;
        l = l ^ h;
        
        m = m + i;
        n = n * j;
        o = o - k;
        p = p ^ l;
        
        /* Force spills by using all variables */
        asm volatile ("" : : "r"(r1), "r"(r2), "r"(r3), "r"(r4),
                         "r"(a), "r"(b), "r"(c), "r"(d),
                         "r"(e), "r"(f), "r"(g), "r"(h),
                         "r"(i), "r"(j), "r"(k), "r"(l),
                         "r"(m), "r"(n), "r"(o), "r"(p));
    }
    
    return r1 + r2 + r3 + r4 + a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
}

/* Helper to prevent dead code elimination */
static void use_result(long a, double b, int c, v4si d, int e) {
    /* Use inline asm to prevent elimination without external calls */
    asm volatile ("" : : "r"(a), "r"(*(long long*)&b), "r"(c), "r"(d[0]), "r"(e));
}

/* Main function marked as cold to potentially affect block ordering */
COLD int main(int argc, char **argv) {
    int iterations = 1000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 100) iterations = 100;
        if (iterations > 10000) iterations = 10000;
    }
    
    /* Use CPU features to engage target-specific optimizations */
#ifdef __GNUC__
    if (__builtin_cpu_supports("avx2")) {
        /* This may affect register allocation decisions */
        asm volatile ("# AVX2 supported" : : : );
    }
#endif
    
    /* Array of inputs to prevent constant folding */
    int inputs[] = {1, 2, 3, 5, 7, 11, 13, 17, 19, 23};
    double f_inputs[] = {1.1, 2.2, 3.3, 5.5, 7.7, 11.11, 13.13, 17.17, 19.19, 23.23};
    v4si v_inputs[] = {
        {1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12},
        {13, 14, 15, 16}, {17, 18, 19, 20}
    };
    
    long total_a = 0;
    double total_b = 0.0;
    int total_c = 0;
    v4si total_d = {0, 0, 0, 0};
    int total_e = 0;
    
    /* Call patterns with different inputs */
    for (int i = 0; i < 10; i++) {
        total_a += pattern_a(iterations / 10, inputs[i]);
        total_b += pattern_b(iterations / 10, f_inputs[i]);
        total_c += pattern_c(iterations / 10, inputs[i]);
        if (i < 5) {
            v4si result = pattern_d(iterations / 20, v_inputs[i]);
            total_d += result;
        }
        total_e += pattern_e(iterations / 10, inputs[i]);
    }
    
    /* Use results to prevent elimination */
    use_result(total_a, total_b, total_c, total_d, total_e);
    
    /* Optional debug output (doesn't affect coverage collection) */
    printf("Results: %ld %f %d %d %d\n", 
           total_a, total_b, total_c, total_d[0], total_e);
    
    return 0;
}
