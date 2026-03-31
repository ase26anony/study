/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Define large vector types */
typedef int v8si __attribute__((vector_size(32)));
typedef long long v4di __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));

/* SSE/AVX types for intrinsics */
#ifdef __SSE2__
#include <emmintrin.h>
#endif
#ifdef __AVX__
#include <immintrin.h>
#endif

/* Prevent optimization */
#define KEEP(expr) do { \
    volatile __typeof__(expr) _tmp = (expr); \
    asm volatile("" : : "r"(&_tmp) : "memory"); \
} while(0)

/* Compiler barrier */
#define BARRIER() asm volatile("" : : : "memory")

/* Test function with many vector operations - marked noinline */
__attribute__((noinline, target("avx2")))
v8si test_many_operands(v8si a, v8si b, v8si c, v8si d, 
                        v8si e, v8si f, v8si g, v8si h) {
    volatile v8si v1, v2, v3, v4, v5, v6, v7, v8;
    
    /* Complex shuffle operation - may require many operands */
    v8si shuffle_mask = {7, 6, 5, 4, 3, 2, 1, 0};
    v1 = __builtin_shuffle(a, b, shuffle_mask);
    KEEP(v1);
    BARRIER();
    
    /* Vector conditional with comparison */
    v8si cmp = a > b;
    v2 = cmp ? (c * d + e) : (f - g * h);
    KEEP(v2);
    BARRIER();
    
    /* Chain of operations that might expand to many operands */
    v3 = __builtin_shuffle(v1, v2, shuffle_mask);
    v4 = v3 + a * b - c / (d + (v8si){1,1,1,1,1,1,1,1});
    KEEP(v4);
    BARRIER();
    
    /* Another complex conditional */
    v8si cmp2 = v4 > (v8si){0,0,0,0,0,0,0,0};
    v5 = cmp2 ? (v1 * v2 + v3) : (v4 - v1);
    KEEP(v5);
    BARRIER();
    
    /* Blend-like operation using conditional */
    v6 = (a > c) ? b : d;
    v7 = (e > g) ? f : h;
    v8 = (v6 > v7) ? v5 : v4;
    KEEP(v8);
    BARRIER();
    
    /* Final combination */
    return v1 + v2 * v3 - v4 / (v5 + v6) + v7 * v8;
}

/* Test with floating point vectors */
__attribute__((noinline, target("avx")))
v4df test_fp_many_operands(v4df a, v4df b, v4df c, v4df d,
                           v4df e, v4df f, v4df g, v4df h) {
    volatile v4df v1, v2, v3, v4;
    
    /* Complex FP expression that might need many operands */
    v4df mask = {1.0, -1.0, 1.0, -1.0};
    v1 = a * b + c * d;
    KEEP(v1);
    BARRIER();
    
    /* Conditional with FP comparison */
    v4df cmp = a > b;
    v2 = cmp ? (c * d) : (e / f);
    KEEP(v2);
    BARRIER();
    
    /* Chain of operations */
    v3 = v1 + v2 * g - h / (a + 1.0);
    KEEP(v3);
    BARRIER();
    
    /* Another conditional blend */
    v4 = (v1 > v2) ? v3 : (g * h);
    KEEP(v4);
    BARRIER();
    
    return v1 * v2 + v3 / v4 - (a + b) * (c - d);
}

/* Test with 11 operand pattern using AVX-512 style operations */
#ifdef __AVX512F__
typedef int v16si __attribute__((vector_size(64)));
__attribute__((noinline, target("avx512f")))
v16si test_11_operands(v16si a, v16si b, v16si c, v16si d,
                       v16si e, v16si f, v16si g, v16si h,
                       v16si i, v16si j) {
    volatile v16si v1, v2, v3, v4;
    
    /* Very complex expression that might need 11 operands */
    v16si mask = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    v1 = __builtin_shuffle(a, b, mask);
    KEEP(v1);
    BARRIER();
    
    /* Multiple operations chained */
    v2 = (a > b) ? (c * d + e) : (f - g * h);
    KEEP(v2);
    BARRIER();
    
    v3 = v1 * v2 + i / (j + (v16si){1});
    KEEP(v3);
    BARRIER();
    
    /* Triple conditional blend */
    v4 = (v1 > v2) ? v3 : ((v2 > v3) ? i : j);
    KEEP(v4);
    BARRIER();
    
    return a + b * c - d / e + f * g - h / i + j * v1 - v2 / v3 + v4;
}
#endif

int main() {
    /* Initialize test vectors */
    v8si a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b = {8, 7, 6, 5, 4, 3, 2, 1};
    v8si c = {2, 3, 4, 5, 6, 7, 8, 9};
    v8si d = {9, 8, 7, 6, 5, 4, 3, 2};
    v8si e = {3, 4, 5, 6, 7, 8, 9, 10};
    v8si f = {10, 9, 8, 7, 6, 5, 4, 3};
    v8si g = {4, 5, 6, 7, 8, 9, 10, 11};
    v8si h = {11, 10, 9, 8, 7, 6, 5, 4};
    
    /* Call test function */
    v8si result = test_many_operands(a, b, c, d, e, f, g, h);
    
    /* Compute checksum to prevent elimination */
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    
    /* Test FP vectors */
    v4df fa = {1.0, 2.0, 3.0, 4.0};
    v4df fb = {4.0, 3.0, 2.0, 1.0};
    v4df fc = {2.0, 3.0, 4.0, 5.0};
    v4df fd = {5.0, 4.0, 3.0, 2.0};
    v4df fe = {3.0, 4.0, 5.0, 6.0};
    v4df ff = {6.0, 5.0, 4.0, 3.0};
    v4df fg = {4.0, 5.0, 6.0, 7.0};
    v4df fh = {7.0, 6.0, 5.0, 4.0};
    
    v4df fp_result = test_fp_many_operands(fa, fb, fc, fd, fe, ff, fg, fh);
    
    double fp_sum = 0.0;
    for (int i = 0; i < 4; i++) {
        fp_sum += fp_result[i];
    }
    
    /* Test 11 operand path if AVX-512 available */
#ifdef __AVX512F__
    v16si va = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    v16si vb = {16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1};
    v16si vc = {2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17};
    v16si vd = {17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2};
    v16si ve = {3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18};
    v16si vf = {18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3};
    v16si vg = {4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19};
    v16si vh = {19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4};
    v16si vi = {5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
    v16si vj = {20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5};
    
    v16si vresult = test_11_operands(va, vb, vc, vd, ve, vf, vg, vh, vi, vj);
    
    int vsum = 0;
    for (int i = 0; i < 16; i++) {
        vsum += vresult[i];
    }
    
    printf("Results: %d (int), %.2f (float), %d (avx512)\n", sum, fp_sum, vsum);
    return (sum != 0 && fp_sum != 0.0 && vsum != 0) ? 0 : 1;
#else
    printf("Results: %d (int), %.2f (float)\n", sum, fp_sum);
    return (sum != 0 && fp_sum != 0.0) ? 0 : 1;
#endif
}
