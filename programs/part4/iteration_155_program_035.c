/* Test program to cover 10/11 operand expansion cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

/* Strategy 1: Vector operations with many operands */
#ifdef __AVX512F__
typedef double v8df __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));
#endif

#ifdef __AVX__
typedef double v4df __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));
#endif

/* Strategy 2: Complex inline assembly with many operands */
static inline void asm_10_operands(int *out, int a, int b, int c, int d, int e,
                                   int f, int g, int h, int i, int j) {
    /* 10 input operands + 1 output = 11 total operands in RTL */
    asm volatile (
        "imul %[a], %[b]\n\t"
        "imul %[c], %[d]\n\t"
        "imul %[e], %[f]\n\t"
        "imul %[g], %[h]\n\t"
        "imul %[i], %[j]\n\t"
        "add %[b], %[a]\n\t"
        "add %[d], %[c]\n\t"
        "add %[f], %[e]\n\t"
        "add %[h], %[g]\n\t"
        "add %[j], %[i]\n\t"
        "mov %[a], %[out]\n\t"
        : [out] "=r" (*out)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "memory"
    );
}

static inline void asm_11_operands_mixed(int *out, int a, int b, int c, int d, 
                                         int e, int f, int g, int h, int i, 
                                         int j, int k) {
    /* Mix of register, memory, and immediate constraints */
    int temp = k;
    asm volatile (
        "lea (%[a],%[b],2), %[out]\n\t"
        "add %[c], %[out]\n\t"
        "add %[d], %[out]\n\t"
        "add %[e], %[out]\n\t"
        "add %[f], %[out]\n\t"
        "add %[g], %[out]\n\t"
        "add %[h], %[out]\n\t"
        "add %[i], %[out]\n\t"
        "add %[j], %[out]\n\t"
        "add %[temp], %[out]\n\t"
        : [out] "=&r" (*out)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [temp] "rm" (temp),
          "i" (1), "i" (2), "i" (3)  /* Additional immediate operands */
        : "cc"
    );
}

/* Strategy 3: Complex vector expressions */
#ifdef __AVX512F__
static v8df vector_fma_chain(v8df a, v8df b, v8df c, v8df d, v8df e) {
    /* This may generate RTL with many operands for combined FMA operations */
    return __builtin_ia32_vfmaddpd512_mask(a, b, 
           __builtin_ia32_vfmaddpd512_mask(c, d, e, 
           (__mmask8)-1, 4), (__mmask8)-1, 4);
}
#endif

#ifdef __AVX__
static v4df avx_fma_chain(v4df a, v4df b, v4df c, v4df d, v4df e, v4df f) {
    /* Chain multiple FMA operations - may consolidate into complex RTL */
    v4df t1 = __builtin_ia32_vfmaddpd(a, b, c);
    v4df t2 = __builtin_ia32_vfmaddpd(d, e, f);
    return __builtin_ia32_vfmaddpd(t1, t2, a);
}
#endif

/* Strategy 4: Complex constant expressions */
static int complex_const_expr(void) {
    /* Force compiler to handle many constants in one expression */
    int result = 0;
    
    /* Use __builtin_constant_p to prevent early folding */
    if (__builtin_constant_p(1)) {
        result = 1 * 2 + 3 * 4 + 5 * 6 + 7 * 8 + 9 * 10 + 11 * 12 +
                 13 * 14 + 15 * 16 + 17 * 18 + 19 * 20 + 21;
    } else {
        result = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11;
    }
    
    return result;
}

/* Strategy 5: Template/generic approach (using macros for C) */
#define GENERATE_OPERATION(TYPE, SUFFIX) \
    static TYPE operation_##SUFFIX(TYPE a, TYPE b, TYPE c, TYPE d, TYPE e, \
                                   TYPE f, TYPE g, TYPE h, TYPE i, TYPE j) { \
        return ((a + b) * (c - d)) + ((e / f) * (g % h)) + (i & j) + \
               ((a | b) ^ (c & d)) + ((e << 2) >> 1); \
    }

GENERATE_OPERATION(int, int)
GENERATE_OPERATION(long, long)
GENERATE_OPERATION(float, float)
GENERATE_OPERATION(double, double)

/* Strategy 6: Vector shuffle with large constant mask */
#ifdef __AVX__
static v8sf vector_shuffle_complex(v8sf a, v8sf b) {
    /* Shuffle with a complex constant mask - may generate many immediate operands */
    const int mask[16] = {0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15};
    
    /* Use each element of the mask - forces compiler to handle many constants */
    v8sf result = a;
    for (int i = 0; i < 8; i++) {
        int idx = mask[i] % 16;
        if (idx < 8) {
            result[i] = a[idx];
        } else {
            result[i] = b[idx - 8];
        }
    }
    return result;
}
#endif

/* Main function that uses all patterns */
int main(void) {
    volatile int result = 0;
    
    /* Test 1: Inline assembly with many operands */
    int asm_result;
    asm_10_operands(&asm_result, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    result += asm_result;
    
    asm_11_operands_mixed(&asm_result, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    result += asm_result;
    
    /* Test 2: Complex constant expression */
    result += complex_const_expr();
    
    /* Test 3: Generic operations with many operands */
    result += operation_int(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    result += operation_long(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    
    /* Test 4: Vector operations (compile-time only, but triggers RTL expansion) */
    #ifdef __AVX__
    {
        v4df v1 = {1.0, 2.0, 3.0, 4.0};
        v4df v2 = {5.0, 6.0, 7.0, 8.0};
        v4df v3 = {9.0, 10.0, 11.0, 12.0};
        v4df v4 = {13.0, 14.0, 15.0, 16.0};
        v4df v5 = {17.0, 18.0, 19.0, 20.0};
        v4df v6 = {21.0, 22.0, 23.0, 24.0};
        
        v4df vresult = avx_fma_chain(v1, v2, v3, v4, v5, v6);
        /* Use result to prevent optimization */
        result += (int)vresult[0];
    }
    
    {
        v8sf v1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
        v8sf v2 = {9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
        v8sf vresult = vector_shuffle_complex(v1, v2);
        result += (int)vresult[0];
    }
    #endif
    
    #ifdef __AVX512F__
    {
        v8df v1 = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
        v8df v2 = {9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0};
        v8df v3 = {17.0, 18.0, 19.0, 20.0, 21.0, 22.0, 23.0, 24.0};
        v8df v4 = {25.0, 26.0, 27.0, 28.0, 29.0, 30.0, 31.0, 32.0};
        v8df v5 = {33.0, 34.0, 35.0, 36.0, 37.0, 38.0, 39.0, 40.0};
        
        v8df vresult = vector_fma_chain(v1, v2, v3, v4, v5);
        result += (int)vresult[0];
    }
    #endif
    
    /* Test 5: Large switch statement with many cases (generates jump table) */
    int switch_var = result % 20;
    switch (switch_var) {
        case 0: result += 100; break;
        case 1: result += 101; break;
        case 2: result += 102; break;
        case 3: result += 103; break;
        case 4: result += 104; break;
        case 5: result += 105; break;
        case 6: result += 106; break;
        case 7: result += 107; break;
        case 8: result += 108; break;
        case 9: result += 109; break;
        case 10: result += 110; break;
        case 11: result += 111; break;
        case 12: result += 112; break;
        case 13: result += 113; break;
        case 14: result += 114; break;
        case 15: result += 115; break;
        case 16: result += 116; break;
        case 17: result += 117; break;
        case 18: result += 118; break;
        case 19: result += 119; break;
        default: result += 120; break;
    }
    
    printf("Result: %d\n", result);
    return 0;
}
