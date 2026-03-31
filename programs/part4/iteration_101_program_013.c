/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Prevent optimization */
#define NOOPT __attribute__((noinline, noclone))
#define BARRIER() asm volatile("" ::: "memory")

/* Vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Larger vector types for AVX */
#ifdef __AVX__
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));
#endif

#ifdef __AVX512F__
typedef double v8df __attribute__((vector_size(64)));
typedef int v16si __attribute__((vector_size(64)));
#endif

/* Complex operation that may expand to many operands */
NOOPT v4si test_10_operands(v4si a, v4si b, v4si c, v4si d, v4si mask) {
    /* Force memory operations to prevent optimization */
    volatile v4si temp1, temp2, temp3;
    
    /* Complex shuffle with variable mask - may need many operands */
    v4si shuffled = __builtin_shuffle(a, b, mask);
    BARRIER();
    
    /* Vector conditional with arithmetic - generates VEC_COND_EXPR */
    v4si cmp = a > b;
    v4si sel = cmp ? (c * d) : (c + d);
    BARRIER();
    
    /* Blend operation using conditional */
    v4si blended = (shuffled > sel) ? shuffled : sel;
    BARRIER();
    
    /* More complex expression with multiple temporaries */
    temp1 = a * b + c;
    temp2 = d * shuffled - sel;
    temp3 = temp1 | temp2;
    BARRIER();
    
    /* Final complex expression that might need many operands */
    v4si result = (blended & temp3) | (temp1 ^ temp2);
    
    /* Use volatile store to force the operation */
    volatile v4si final_store;
    final_store = result;
    
    return result;
}

#ifdef __AVX__
NOOPT v8sf test_avx_many_ops(v8sf a, v8sf b, v8sf c, v8sf d, v8sf e, v8sf f) {
    volatile v8sf temp1, temp2, temp3, temp4;
    
    /* Complex AVX expression chain */
    temp1 = a * b + c;
    BARRIER();
    temp2 = d - e * f;
    BARRIER();
    
    /* Vector comparison and conditional */
    v8sf cmp = a > b;
    v8sf sel = cmp ? temp1 : temp2;
    BARRIER();
    
    /* Shuffle with computation in mask */
    v8si mask = {0, 7, 1, 6, 2, 5, 3, 4};
    v8sf shuffled = __builtin_shufflevector(sel, temp1, 7, 6, 5, 4, 3, 2, 1, 0);
    BARRIER();
    
    /* Complex fused multiply-add like pattern */
    temp3 = a * b + c * d;
    temp4 = e * f - a * c;
    BARRIER();
    
    /* Final blend with condition */
    v8sf result = (shuffled > temp3) ? (temp4 + shuffled) : (temp3 - temp4);
    
    volatile v8sf final_store;
    final_store = result;
    
    return result;
}
#endif

#ifdef __AVX512F__
/* AVX-512 can have patterns with many operands due to masking */
NOOPT v8df test_avx512_masked(v8df a, v8df b, v8df c, v8df d, 
                              v8df e, v8df f, v8df g, v8df h,
                              v8df mask_vec) {
    volatile v8df temp[8];
    
    /* Create a mask from comparison */
    __mmask8 mask = _mm512_cmp_pd_mask(a, b, _CMP_GT_OQ);
    BARRIER();
    
    /* Masked operations - may expand to many operands */
    v8df masked_add = _mm512_mask_add_pd(a, mask, c, d);
    BARRIER();
    
    v8df masked_mul = _mm512_mask_mul_pd(b, mask, e, f);
    BARRIER();
    
    /* Complex blend */
    v8df blended = _mm512_mask_blend_pd(mask, g, h);
    BARRIER();
    
    /* Multiple masked operations in one expression */
    v8df result = _mm512_mask_add_pd(
        _mm512_mask_mul_pd(a, mask, b, c),
        mask,
        _mm512_mask_sub_pd(d, mask, e, f),
        blended
    );
    
    volatile v8df final_store;
    final_store = result;
    
    return result;
}
#endif

/* Test function that mixes many operations */
NOOPT v4si complex_mixed_ops(v4si a, v4si b, v4si c, v4si d,
                            v4sf fa, v4sf fb, v4sf fc,
                            v2df da, v2df db) {
    volatile v4si vi_temp[4];
    volatile v4sf vf_temp[4];
    volatile v2df vd_temp[4];
    
    /* Integer vector operations */
    v4si shuffle_mask = {3, 2, 1, 0};
    v4si shuffled = __builtin_shuffle(a, b, shuffle_mask);
    BARRIER();
    
    /* Cross-type conversions (may need many operands) */
    v4sf converted = __builtin_convertvector(shuffled, v4sf);
    BARRIER();
    
    /* Float vector operations */
    v4sf fcmp = fa > fb;
    v4sf fsel = fcmp ? (fb * fc) : (fb + fc);
    BARRIER();
    
    /* Double vector operations */
    v2df dblend = __builtin_shufflevector(da, db, 1, 0);
    BARRIER();
    
    /* Mixed type expression */
    v4sf to_int = converted + fsel;
    v4si back_to_int = __builtin_convertvector(to_int, v4si);
    BARRIER();
    
    /* Final complex expression */
    v4si result = back_to_int + shuffled * c - d;
    
    /* Force all operations */
    vi_temp[0] = shuffled;
    vf_temp[0] = converted;
    vf_temp[1] = fsel;
    vd_temp[0] = dblend;
    vi_temp[1] = back_to_int;
    vi_temp[2] = result;
    
    return result;
}

int main() {
    /* Initialize test vectors */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {9, 10, 11, 12};
    v4si d = {13, 14, 15, 16};
    v4si mask = {3, 2, 1, 0};
    
    v4sf fa = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fb = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf fc = {9.0f, 10.0f, 11.0f, 12.0f};
    
    v2df da = {1.0, 2.0};
    v2df db = {3.0, 4.0};
    
    printf("Testing 10/11 operand expansion patterns...\n");
    
    /* Call test functions to trigger expansion */
    v4si res1 = test_10_operands(a, b, c, d, mask);
    BARRIER();
    
    v4si res2 = complex_mixed_ops(a, b, c, d, fa, fb, fc, da, db);
    BARRIER();
    
#ifdef __AVX__
    v8sf avx_a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf avx_b = {9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    v8sf avx_c = {17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f, 24.0f};
    v8sf avx_d = {25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f, 32.0f};
    v8sf avx_e = {33.0f, 34.0f, 35.0f, 36.0f, 37.0f, 38.0f, 39.0f, 40.0f};
    v8sf avx_f = {41.0f, 42.0f, 43.0f, 44.0f, 45.0f, 46.0f, 47.0f, 48.0f};
    
    v8sf res3 = test_avx_many_ops(avx_a, avx_b, avx_c, avx_d, avx_e, avx_f);
    BARRIER();
#endif
    
    /* Compute checksum to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += res1[i];
        sum += res2[i];
    }
    
    printf("Checksum: %d\n", sum);
    
    /* Return based on checksum to ensure execution */
    return (sum != 0) ? 0 : 1;
}
