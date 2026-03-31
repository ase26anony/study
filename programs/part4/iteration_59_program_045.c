/* Test program to cover 10-11 operand cases in optabs.cc */
/* Compile with: gcc -O3 -mavx512f -mfma -mavx512vl -ftree-vectorize -c test.c -o test.o */

#include <immintrin.h>
#include <stdio.h>

/* Force inline expansion */
#define FORCE_INLINE __attribute__((always_inline)) inline

/* Complex AVX-512 operation that should generate many operands */
FORCE_INLINE __m512 test_avx512_10_operand(__m512 a, __m512 b, __m512 c, 
                                          __mmask16 k, __m512 src) {
    /* _mm512_mask3_fmadd_ps has 5 explicit operands but expands to more */
    /* Use multiple masked operations in sequence to force complex pattern */
    __m512 t1 = _mm512_mask_add_ps(src, k, a, b);  /* mask, src, a, b */
    __m512 t2 = _mm512_mask_mul_ps(t1, k, b, c);   /* mask, src, b, c */
    
    /* Fused multiply-add with mask - likely expands to many operands */
    __m512 t3 = _mm512_mask_fmadd_ps(a, k, b, c);  /* a, mask, b, c */
    
    /* Blend with mask - adds more operands */
    __m512 result = _mm512_mask_blend_ps(k, t2, t3);
    
    return result;
}

/* Test with 11 operands using gather/scatter operations */
FORCE_INLINE __m512i test_avx512_11_operand(__m512i index, __m512i src,
                                           __mmask16 k, int scale) {
    /* _mm512_mask_i32gather_epi32 has: src, mask, index, addr, scale */
    /* But the address calculation adds more operands */
    const int* base_addr = (const int*)0x1000;
    
    /* This should expand to many operands during RTL generation */
    __m512i gathered = _mm512_mask_i32gather_epi32(src, k, index, 
                                                  base_addr, scale);
    
    /* Add another operation to ensure all operands are used */
    __m512i result = _mm512_add_epi32(gathered, _mm512_set1_epi32(1));
    
    return result;
}

/* Use GCC vector extensions for complex reduction */
typedef float v16sf __attribute__((vector_size(64)));
typedef int v16si __attribute__((vector_size(64)));

FORCE_INLINE v16sf complex_vector_op(v16sf a, v16sf b, v16sf c, v16sf d) {
    /* Complex expression that might generate multi-operand pattern */
    v16sf t1 = a * b + c;
    v16sf t2 = b * c + d;
    v16sf t3 = c * d + a;
    
    /* Nested FMA-like operations */
    v16sf result = t1 * t2 + t3;
    
    /* Add conditional blend */
    v16si mask = (v16si)(a > b);
    v16sf blended = __builtin_shuffle(t2, t3, (v16si)mask);
    
    return result + blended;
}

/* Inline assembly with many operands as fallback */
void test_asm_11_operand(void) {
    long a, b, c, d, e, f, g, h, i, j, k;
    
    /* Initialize to prevent optimization */
    a = 1; b = 2; c = 3; d = 4; e = 5;
    f = 6; g = 7; h = 8; i = 9; j = 10; k = 11;
    
    /* 11-operand asm statement */
    __asm__ volatile (
        "add %[a], %[b]\n\t"
        "add %[c], %[d]\n\t"
        "add %[e], %[f]\n\t"
        "add %[g], %[h]\n\t"
        "add %[i], %[j]\n\t"
        : [a] "+r" (a), [b] "+r" (b), [c] "+r" (c),
          [d] "+r" (d), [e] "+r" (e), [f] "+r" (f)
        : [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j), [k] "r" (k)
        : "cc", "memory"
    );
    
    /* Use results to prevent dead code elimination */
    printf("ASM result: %ld\n", a + b + c + d + e + f);
}

/* Main test function */
int main(void) {
    /* Initialize AVX-512 vectors */
    __m512 a = _mm512_set1_ps(1.0f);
    __m512 b = _mm512_set1_ps(2.0f);
    __m512 c = _mm512_set1_ps(3.0f);
    __m512 src = _mm512_set1_ps(0.0f);
    __mmask16 mask = 0xAAAA;  /* Alternating bits */
    
    /* Test 10-operand pattern */
    __m512 result1 = test_avx512_10_operand(a, b, c, mask, src);
    
    /* Test 11-operand pattern */
    __m512i index = _mm512_set1_epi32(0);
    __m512i src_i = _mm512_set1_epi32(0);
    __m512i result2 = test_avx512_11_operand(index, src_i, mask, 4);
    
    /* Test GCC vector extensions */
    v16sf va = {0};
    v16sf vb = {0};
    v16sf vc = {0};
    v16sf vd = {0};
    
    for (int i = 0; i < 16; i++) {
        va[i] = i * 0.1f;
        vb[i] = i * 0.2f;
        vc[i] = i * 0.3f;
        vd[i] = i * 0.4f;
    }
    
    v16sf result3 = complex_vector_op(va, vb, vc, vd);
    
    /* Test inline assembly */
    test_asm_11_operand();
    
    /* Use results to prevent optimization */
    float sum = 0;
    float* r1 = (float*)&result1;
    int* r2 = (int*)&result2;
    float* r3 = (float*)&result3;
    
    for (int i = 0; i < 16; i++) {
        sum += r1[i] + r2[i] + r3[i];
    }
    
    printf("Result sum: %f\n", sum);
    
    return (int)sum;
}
