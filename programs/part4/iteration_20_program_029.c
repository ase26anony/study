/* test_mcf_coverage.c
 * 
 * This test program is designed to trigger GCC's Min-Cost Flow pass
 * during register allocation, specifically to cover the special node
 * printing logic in mcf.cc's print_node function.
 *
 * Compile with: gcc -O3 -fprofile-arcs -ftest-coverage -fdump-rtl-all -c test_mcf_coverage.c
 * Then run: ./test_mcf_coverage (to generate profile data)
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure each function is compiled separately */
#define NOINLINE __attribute__((noinline))
#define COLD __attribute__((cold))

/* Vector type for SIMD pressure */
typedef int v4si __attribute__((vector_size(16)));

/* Pattern A: Integer arithmetic chain with 30+ variables in a loop */
NOINLINE int pattern_a_int_pressure(int input) {
    /* 30+ integer variables to create register pressure */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int a11, a12, a13, a14, a15, a16, a17, a18, a19, a20;
    int a21, a22, a23, a24, a25, a26, a27, a28, a29, a30;
    int a31, a32, a33, a34, a35, a36, a37, a38, a39, a40;
    
    /* Complex initialization chain */
    a1 = input + 1;
    a2 = a1 * 2 - input;
    a3 = a2 + a1;
    a4 = a3 * 3 - a2;
    a5 = a4 + a3 + a1;
    a6 = a5 * 2 - a4;
    a7 = a6 + a5 + a3;
    a8 = a7 * 3 - a6;
    a9 = a8 + a7 + a5;
    a10 = a9 * 2 - a8;
    
    a11 = a10 + a9 + a7;
    a12 = a11 * 3 - a10;
    a13 = a12 + a11 + a9;
    a14 = a13 * 2 - a12;
    a15 = a14 + a13 + a11;
    a16 = a15 * 3 - a14;
    a17 = a16 + a15 + a13;
    a18 = a17 * 2 - a16;
    a19 = a18 + a17 + a15;
    a20 = a19 * 3 - a18;
    
    a21 = a20 + a19 + a17;
    a22 = a21 * 2 - a20;
    a23 = a22 + a21 + a19;
    a24 = a23 * 3 - a22;
    a25 = a24 + a23 + a21;
    a26 = a25 * 2 - a24;
    a27 = a26 + a25 + a23;
    a28 = a27 * 3 - a26;
    a29 = a28 + a27 + a25;
    a30 = a29 * 2 - a28;
    
    a31 = a30 + a29 + a27;
    a32 = a31 * 3 - a30;
    a33 = a32 + a31 + a29;
    a34 = a33 * 2 - a32;
    a35 = a34 + a33 + a31;
    a36 = a35 * 3 - a34;
    a37 = a36 + a35 + a33;
    a38 = a37 * 2 - a36;
    a39 = a38 + a37 + a35;
    a40 = a39 * 3 - a38;
    
    /* Tight interdependent loop to maximize pressure */
    for (int i = 0; i < 100; i++) {
        a1 = a2 + a3;
        a2 = a3 + a4;
        a3 = a4 + a5;
        a4 = a5 + a6;
        a5 = a6 + a7;
        a6 = a7 + a8;
        a7 = a8 + a9;
        a8 = a9 + a10;
        a9 = a10 + a11;
        a10 = a11 + a12;
        
        a11 = a12 + a13;
        a12 = a13 + a14;
        a13 = a14 + a15;
        a14 = a15 + a16;
        a15 = a16 + a17;
        a16 = a17 + a18;
        a17 = a18 + a19;
        a18 = a19 + a20;
        a19 = a20 + a21;
        a20 = a21 + a22;
        
        a21 = a22 + a23;
        a22 = a23 + a24;
        a23 = a24 + a25;
        a24 = a25 + a26;
        a25 = a26 + a27;
        a26 = a27 + a28;
        a27 = a28 + a29;
        a28 = a29 + a30;
        a29 = a30 + a31;
        a30 = a31 + a32;
        
        a31 = a32 + a33;
        a32 = a33 + a34;
        a33 = a34 + a35;
        a34 = a35 + a36;
        a35 = a36 + a37;
        a36 = a37 + a38;
        a37 = a38 + a39;
        a38 = a39 + a40;
        a39 = a40 + a1;
        a40 = a1 + a2;
        
        /* Prevent loop elimination */
        asm volatile("" : : "r"(a1), "r"(a20), "r"(a40));
    }
    
    /* Complex return to use all variables */
    return a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
           a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19 + a20 +
           a21 + a22 + a23 + a24 + a25 + a26 + a27 + a28 + a29 + a30 +
           a31 + a32 + a33 + a34 + a35 + a36 + a37 + a38 + a39 + a40;
}

/* Pattern B: Floating-point intensive computation */
NOINLINE double pattern_b_fp_pressure(double input) {
    /* 20+ double variables to pressure FP registers */
    double b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    double b11, b12, b13, b14, b15, b16, b17, b18, b19, b20;
    double b21, b22, b23, b24, b25;
    
    b1 = input;
    b2 = b1 * 1.1;
    b3 = b2 / 1.2;
    b4 = b3 + b1;
    b5 = b4 * 0.9;
    b6 = b5 - b2;
    b7 = b6 * 1.3;
    b8 = b7 / 1.4;
    b9 = b8 + b3;
    b10 = b9 * 0.8;
    
    b11 = b10 - b4;
    b12 = b11 * 1.5;
    b13 = b12 / 1.6;
    b14 = b13 + b5;
    b15 = b14 * 0.7;
    b16 = b15 - b6;
    b17 = b16 * 1.7;
    b18 = b17 / 1.8;
    b19 = b18 + b7;
    b20 = b19 * 0.6;
    
    b21 = b20 - b8;
    b22 = b21 * 1.9;
    b23 = b22 / 2.0;
    b24 = b23 + b9;
    b25 = b24 * 0.5;
    
    /* Nested loops with complex control flow */
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 10; j++) {
            if (j % 3 == 0) {
                b1 = b2 + b3;
                b2 = b3 * b4;
                continue;
            } else if (j % 3 == 1) {
                b3 = b4 - b5;
                b4 = b5 / b6;
                break;
            } else {
                b5 = b6 + b7;
                b6 = b7 * b8;
            }
            
            /* Switch inside loop for CFG complexity */
            switch (i % 5) {
                case 0: b7 = b8 + b9; break;
                case 1: b8 = b9 * b10; break;
                case 2: b9 = b10 - b11; break;
                case 3: b10 = b11 / b12; break;
                case 4: b11 = b12 + b13; break;
            }
        }
        
        /* Prevent dead code elimination */
        asm volatile("" : : "r"(b1), "r"(b10), "r"(b25));
    }
    
    return b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9 + b10 +
           b11 + b12 + b13 + b14 + b15 + b16 + b17 + b18 + b19 + b20 +
           b21 + b22 + b23 + b24 + b25;
}

/* Pattern C: Complex control flow with switch statements */
NOINLINE int pattern_c_cfg_complexity(int input) {
    int result = input;
    
    /* Large switch with many cases creates many basic blocks */
    for (int i = 0; i < 100; i++) {
        switch (i % 21) {  /* 21 cases for complex CFG */
            case 0: result += i * 2; break;
            case 1: result -= i * 3; break;
            case 2: result *= i + 1; break;
            case 3: result /= (i % 7) + 1; break;
            case 4: result ^= i; break;
            case 5: result |= i << 2; break;
            case 6: result &= ~i; break;
            case 7: result = result << (i % 4); break;
            case 8: result = result >> (i % 4); break;
            case 9: result += result % 17; break;
            case 10: result -= result % 13; break;
            case 11: result *= (i % 5) + 1; break;
            case 12: result /= (i % 3) + 1; break;
            case 13: result = ~result; break;
            case 14: result = result ^ (i << 3); break;
            case 15: result = result | 0xFF; break;
            case 16: result = result & 0xFFFF; break;
            case 17: result = result + (i << 4); break;
            case 18: result = result - (i << 2); break;
            case 19: result = result * (i % 9); break;
            case 20: result = result / ((i % 7) + 2); break;
        }
        
        /* Nested control flow with breaks and continues */
        for (int j = 0; j < 10; j++) {
            if (j % 2 == 0) {
                result += j;
                continue;
            }
            if (j % 3 == 0) {
                result -= j;
                break;
            }
            result *= (j + 1);
        }
    }
    
    return result;
}

/* Pattern D: SIMD vector operations */
NOINLINE v4si pattern_d_simd_pressure(v4si input) {
    v4si v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    v4si v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    
    /* Initialize vectors */
    v1 = input + (v4si){1, 2, 3, 4};
    v2 = v1 * (v4si){2, 3, 4, 5};
    v3 = v2 - (v4si){1, 1, 1, 1};
    v4 = v3 / (v4si){2, 2, 2, 2};
    v5 = v4 + v1;
    v6 = v5 * v2;
    v7 = v6 - v3;
    v8 = v7 / v4;
    v9 = v8 + v5;
    v10 = v9 * v6;
    
    v11 = v10 - v7;
    v12 = v11 / v8;
    v13 = v12 + v9;
    v14 = v13 * v10;
    v15 = v14 - v11;
    v16 = v15 / v12;
    v17 = v16 + v13;
    v18 = v17 * v14;
    v19 = v18 - v15;
    v20 = v19 / v16;
    
    /* Vector loop with complex indexing */
    for (int i = 0; i < 50; i++) {
        v1 = v2 + v3;
        v2 = v3 * v4;
        v3 = v4 - v5;
        v4 = v5 / v6;
        v5 = v6 + v7;
        v6 = v7 * v8;
        v7 = v8 - v9;
        v8 = v9 / v10;
        v9 = v10 + v11;
        v10 = v11 * v12;
        
        v11 = v12 - v13;
        v12 = v13 / v14;
        v13 = v14 + v15;
        v14 = v15 * v16;
        v15 = v16 - v17;
        v16 = v17 / v18;
        v17 = v18 + v19;
        v18 = v19 * v20;
        v19 = v20 - v1;
        v20 = v1 / v2;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(v1), "r"(v10), "r"(v20));
    }
    
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
}

/* Pattern E: Explicit register variables to conflict with allocator */
NOINLINE int pattern_e_register_conflict(int input) {
    /* Explicit register variables that may conflict with allocator's choices */
    register int r1 asm ("r12") = input + 1;
    register int r2 asm ("r13") = r1 * 2;
    register int r3 asm ("r14") = r2 + input;
    register int r4 asm ("r15") = r3 * 3;
    
    int a, b, c, d, e, f, g, h, i, j;
    int k, l, m, n, o, p, q, r, s, t;
    
    /* Mix register variables with regular variables */
    a = r1 + 1;
    b = a + r2;
    c = b * r3;
    d = c - r4;
    e = d + a;
    f = e * b;
    g = f - c;
    h = g + d;
    i = h * e;
    j = i - f;
    
    k = j + g;
    l = k * h;
    m = l - i;
    n = m + j;
    o = n * k;
    p = o - l;
    q = p + m;
    r = q * n;
    s = r - o;
    t = s + p;
    
    /* Complex loop mixing all variables */
    for (int idx = 0; idx < 100; idx++) {
        r1 = r2 + a;
        r2 = r3 + b;
        r3 = r4 + c;
        r4 = r1 + d;
        
        a = b + e;
        b = c + f;
        c = d + g;
        d = e + h;
        e = f + i;
        f = g + j;
        g = h + k;
        h = i + l;
        i = j + m;
        j = k + n;
        
        k = l + o;
        l = m + p;
        m = n + q;
        n = o + r;
        o = p + s;
        p = q + t;
        q = r + a;
        r = s + b;
        s = t + c;
        t = a + d;
        
        /* Use computed goto for extra CFG complexity (GCC extension) */
        void* labels[] = {&&L0, &&L1, &&L2, &&L3, &&L4};
        goto *labels[idx % 5];
        
        L0: a += idx; continue;
        L1: b -= idx; continue;
        L2: c *= idx; continue;
        L3: d /= (idx % 7) + 1; continue;
        L4: e ^= idx; continue;
    }
    
    return r1 + r2 + r3 + r4 + a + b + c + d + e + f + g + h + i + j +
           k + l + m + n + o + p + q + r + s + t;
}

/* Main function that calls all patterns */
COLD int main(int argc, char** argv) {
    int total = 0;
    double fp_total = 0.0;
    
    /* Use CPU feature check to engage target-specific optimizations */
#ifdef __GNUC__
    if (__builtin_cpu_supports("avx2")) {
        /* This may affect register allocation decisions */
        asm volatile("" ::: "memory");
    }
#endif
    
    /* Call each pattern multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        total += pattern_a_int_pressure(i);
        fp_total += pattern_b_fp_pressure(i * 0.1);
        total += pattern_c_cfg_complexity(i);
        
        v4si vec_input = {i, i+1, i+2, i+3};
        v4si vec_result = pattern_d_simd_pressure(vec_input);
        total += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
        
        total += pattern_e_register_conflict(i);
    }
    
    /* Print result to prevent complete optimization */
    printf("Result: %d, FP: %f\n", total, fp_total);
    
    return 0;
}
