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

/* Prevent inlining to ensure separate compilation units in same file */
#define NOINLINE __attribute__((noinline))
#define COLD __attribute__((cold))

/* Pattern A: Integer arithmetic chain with 30+ variables in a loop */
NOINLINE static long pattern_a(int input) {
    /* 30+ integer variables to create register pressure */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int a11, a12, a13, a14, a15, a16, a17, a18, a19, a20;
    int a21, a22, a23, a24, a25, a26, a27, a28, a29, a30;
    long result = 0;
    
    /* Complex interdependent calculations */
    a1 = input + 1;
    a2 = a1 * 2;
    a3 = a2 - a1;
    a4 = a3 + input;
    a5 = a4 * a2;
    a6 = a5 / (a1 + 1);
    a7 = a6 ^ a5;
    a8 = a7 & 0xFF;
    a9 = a8 << 2;
    a10 = a9 >> 1;
    
    a11 = a10 + a9;
    a12 = a11 * a8;
    a13 = a12 - a7;
    a14 = a13 + a6;
    a15 = a14 * a5;
    a16 = a15 / (a4 + 1);
    a17 = a16 ^ a15;
    a18 = a17 & 0xFFFF;
    a19 = a18 << 1;
    a20 = a19 >> 2;
    
    a21 = a20 + a19;
    a22 = a21 * a18;
    a23 = a22 - a17;
    a24 = a23 + a16;
    a25 = a24 * a15;
    a26 = a25 / (a14 + 1);
    a27 = a26 ^ a25;
    a28 = a27 & 0xFFFFFF;
    a29 = a28 << 3;
    a30 = a29 >> 1;
    
    /* Loop with register pressure */
    for (int i = 0; i < 100; i++) {
        a1 = (a1 + a2) & 0xFF;
        a2 = (a2 + a3) & 0xFF;
        a3 = (a3 + a4) & 0xFF;
        a4 = (a4 + a5) & 0xFF;
        a5 = (a5 + a6) & 0xFF;
        a6 = (a6 + a7) & 0xFF;
        a7 = (a7 + a8) & 0xFF;
        a8 = (a8 + a9) & 0xFF;
        a9 = (a9 + a10) & 0xFF;
        a10 = (a10 + a11) & 0xFF;
        a11 = (a11 + a12) & 0xFF;
        a12 = (a12 + a13) & 0xFF;
        a13 = (a13 + a14) & 0xFF;
        a14 = (a14 + a15) & 0xFF;
        a15 = (a15 + a16) & 0xFF;
        a16 = (a16 + a17) & 0xFF;
        a17 = (a17 + a18) & 0xFF;
        a18 = (a18 + a19) & 0xFF;
        a19 = (a19 + a20) & 0xFF;
        a20 = (a20 + a21) & 0xFF;
        a21 = (a21 + a22) & 0xFF;
        a22 = (a22 + a23) & 0xFF;
        a23 = (a23 + a24) & 0xFF;
        a24 = (a24 + a25) & 0xFF;
        a25 = (a25 + a26) & 0xFF;
        a26 = (a26 + a27) & 0xFF;
        a27 = (a27 + a28) & 0xFF;
        a28 = (a28 + a29) & 0xFF;
        a29 = (a29 + a30) & 0xFF;
        a30 = (a30 + a1) & 0xFF;
        
        /* Prevent loop elimination */
        asm volatile("" : "+r"(a1), "+r"(a2), "+r"(a3), "+r"(a4), "+r"(a5));
        asm volatile("" : "+r"(a6), "+r"(a7), "+r"(a8), "+r"(a9), "+r"(a10));
    }
    
    result = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
             a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19 + a20 +
             a21 + a22 + a23 + a24 + a25 + a26 + a27 + a28 + a29 + a30;
    
    return result;
}

/* Pattern B: Floating-point intensive computation */
NOINLINE static double pattern_b(double input) {
    /* 20+ double variables to pressure FP registers */
    double b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    double b11, b12, b13, b14, b15, b16, b17, b18, b19, b20;
    double result;
    
    b1 = input + 1.0;
    b2 = b1 * 2.0;
    b3 = b2 - b1;
    b4 = b3 + input;
    b5 = b4 * b2;
    b6 = b5 / (b1 + 1.0);
    b7 = b6 * 0.5;
    b8 = b7 + 3.14159;
    b9 = b8 * b6;
    b10 = b9 / b5;
    
    b11 = b10 + b9;
    b12 = b11 * b8;
    b13 = b12 - b7;
    b14 = b13 + b6;
    b15 = b14 * b5;
    b16 = b15 / (b4 + 1.0);
    b17 = b16 * 0.25;
    b18 = b17 + 2.71828;
    b19 = b18 * b16;
    b20 = b19 / b15;
    
    /* Complex FP loop */
    for (int i = 0; i < 50; i++) {
        b1 = b1 * 0.99 + b2;
        b2 = b2 * 0.98 + b3;
        b3 = b3 * 0.97 + b4;
        b4 = b4 * 0.96 + b5;
        b5 = b5 * 0.95 + b6;
        b6 = b6 * 0.94 + b7;
        b7 = b7 * 0.93 + b8;
        b8 = b8 * 0.92 + b9;
        b9 = b9 * 0.91 + b10;
        b10 = b10 * 0.90 + b11;
        b11 = b11 * 0.89 + b12;
        b12 = b12 * 0.88 + b13;
        b13 = b13 * 0.87 + b14;
        b14 = b14 * 0.86 + b15;
        b15 = b15 * 0.85 + b16;
        b16 = b16 * 0.84 + b17;
        b17 = b17 * 0.83 + b18;
        b18 = b18 * 0.82 + b19;
        b19 = b19 * 0.81 + b20;
        b20 = b20 * 0.80 + b1;
        
        /* Prevent optimization */
        asm volatile("" : "+f"(b1), "+f"(b2), "+f"(b3), "+f"(b4));
    }
    
    result = b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9 + b10 +
             b11 + b12 + b13 + b14 + b15 + b16 + b17 + b18 + b19 + b20;
    
    return result;
}

/* Pattern C: Complex control flow with switch statement */
NOINLINE static int pattern_c(int input) {
    int result = input;
    
    /* Switch with many cases creates complex CFG */
    for (int i = 0; i < 100; i++) {
        switch (result & 0xF) {  /* 16 cases */
            case 0: result += 1; break;
            case 1: result *= 2; break;
            case 2: result -= 3; break;
            case 3: result ^= 4; break;
            case 4: result |= 5; break;
            case 5: result &= 6; break;
            case 6: result <<= 1; break;
            case 7: result >>= 2; break;
            case 8: result = ~result; break;
            case 9: result = result * result; break;
            case 10: result = result / 2; break;
            case 11: result = result % 13; break;
            case 12: result = result + result; break;
            case 13: result = result - result / 2; break;
            case 14: result = result ^ 0xAAAA; break;
            case 15: result = result | 0x5555; break;
        }
        
        /* Nested loop with break/continue */
        for (int j = 0; j < 10; j++) {
            if (j == result % 3) continue;
            result += j;
            if (result > 10000) break;
        }
        
        /* Another switch inside loop */
        switch (i & 0x3) {
            case 0: result += i; break;
            case 1: result -= i; break;
            case 2: result ^= i; break;
            case 3: result |= i; break;
        }
    }
    
    return result;
}

/* Pattern D: Vector extensions for SIMD register pressure */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

NOINLINE static long pattern_d(int input) {
#ifdef __GNUC__
    v4si v1 = {input, input + 1, input + 2, input + 3};
    v4si v2 = {input + 4, input + 5, input + 6, input + 7};
    v4si v3 = {input + 8, input + 9, input + 10, input + 11};
    v4si v4 = {input + 12, input + 13, input + 14, input + 15};
    v4si v5 = {input + 16, input + 17, input + 18, input + 19};
    v4si v6, v7, v8, v9, v10;
    long result = 0;
    
    /* Vector operations */
    for (int i = 0; i < 50; i++) {
        v6 = v1 + v2;
        v7 = v3 * v4;
        v8 = v5 & v6;
        v9 = v7 | v8;
        v10 = v9 << 1;
        
        v1 = v2 + v3;
        v2 = v4 * v5;
        v3 = v6 & v7;
        v4 = v8 | v9;
        v5 = v10 >> 1;
        
        /* Mix with scalar to force spills */
        int temp = input + i;
        v1[0] += temp;
        v2[1] -= temp;
        v3[2] ^= temp;
        v4[3] |= temp;
        
        asm volatile("" : "+x"(v1), "+x"(v2), "+x"(v3));
    }
    
    result = v1[0] + v1[1] + v1[2] + v1[3] +
             v2[0] + v2[1] + v2[2] + v2[3] +
             v3[0] + v3[1] + v3[2] + v3[3];
    
    return result;
#else
    return input;  /* Fallback for non-GCC */
#endif
}

/* Pattern E: Explicit register variables to conflict with allocator */
NOINLINE static int pattern_e(int input) {
    /* Try to use specific registers */
    register int r1 asm ("r12") = input;
    register int r2 asm ("r13") = input + 1;
    register int r3 asm ("r14") = input + 2;
    register int r4 asm ("r15") = input + 3;
    
    int a = 0, b = 0, c = 0, d = 0, e = 0, f = 0, g = 0, h = 0;
    
    /* Force conflicts between explicit and implicit registers */
    for (int i = 0; i < 100; i++) {
        a = r1 + r2;
        b = r3 * r4;
        c = a ^ b;
        d = c & r1;
        e = d | r2;
        f = e << (r3 & 3);
        g = f >> (r4 & 3);
        h = g + i;
        
        /* Rotate register values */
        r1 = r2;
        r2 = r3;
        r3 = r4;
        r4 = h;
        
        asm volatile("" : "+r"(r1), "+r"(r2), "+r"(r3), "+r"(r4));
        asm volatile("" : "+r"(a), "+r"(b), "+r"(c), "+r"(d));
    }
    
    return r1 + r2 + r3 + r4 + a + b + c + d + e + f + g + h;
}

/* Main function that calls all patterns */
COLD int main(int argc, char **argv) {
    long total = 0;
    int iterations = 10;
    
    /* Use CPU features to engage target-specific optimizations */
#ifdef __GNUC__
    if (__builtin_cpu_supports("avx2")) {
        iterations = 15;  /* More iterations if AVX2 available */
    }
#endif
    
    /* Call each pattern multiple times with different inputs */
    for (int i = 0; i < iterations; i++) {
        total += pattern_a(i * 7 + 1);
        total += (long)pattern_b(i * 0.5 + 1.0);
        total += pattern_c(i * 11 + 3);
        total += pattern_d(i * 13 + 5);
        total += pattern_e(i * 17 + 7);
        
        /* Vary control flow based on iteration */
        if (i % 3 == 0) {
            total += pattern_a(i * 19);
        } else if (i % 3 == 1) {
            total += pattern_c(i * 23);
        } else {
            total += pattern_e(i * 29);
        }
    }
    
    /* Prevent dead code elimination */
    volatile long dummy = total;
    
    /* Optional debug output */
    if (argc > 1) {
        printf("Total: %ld\n", total);
    }
    
    return (int)(total % 1000);
}
