/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* GCC vector types */
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

/* AVX-512 types if available */
#ifdef __AVX512F__
typedef float v16sf __attribute__((vector_size(64)));
typedef double v8df __attribute__((vector_size(64)));
typedef int v16si __attribute__((vector_size(64)));
#endif

/* Prevent optimization */
#define KEEP(expr) do { \
    volatile __typeof__(expr) _tmp = (expr); \
    asm volatile("" : : "r"(&_tmp) : "memory"); \
} while(0)

/* Compiler barrier */
#define BARRIER() asm volatile("" : : : "memory")

/* Noinline test function to prevent inlining */
__attribute__((noinline, target("sse2,sse3,ssse3,sse4.1,sse4.2,avx")))
v4si test_10_11_operands(v4si a, v4si b, v4si c, v4si d,
                         v4sf fa, v4sf fb, v4sf fc, v4sf fd) {
    volatile v4si v1, v2, v3, v4;
    volatile v4sf fv1, fv2, fv3, fv4;
    
    /* Complex shuffle operation - can expand to many operands */
    v4si shuffle_mask = {3, 2, 1, 0};
    v4si shuffled = __builtin_shuffle(a, b, shuffle_mask);
    KEEP(shuffled);
    
    /* Vector conditional expression - generates VEC_COND_EXPR */
    v4si cmp_result = (a > b) ? (c + d) : (c - d);
    KEEP(cmp_result);
    
    /* Chain of operations that might require many temporaries */
    v4si complex_op1 = (a * b) + (c / (d + 1));
    v4si complex_op2 = (b * c) - (a / (d + 2));
    v4si blend_result = (a > c) ? complex_op1 : complex_op2;
    KEEP(blend_result);
    
    BARRIER();
    
    /* Float vector operations with conversions */
    v4sf fcmp = (fa > fb) ? (fc * fd) : (fc / fd);
    KEEP(fcmp);
    
    /* Mixed-type operations requiring conversions */
    v4si converted = __builtin_convertvector(fa, v4si);
    v4si mixed = converted + a;
    KEEP(mixed);
    
    /* Another complex shuffle with variable mask */
    v4si mask = a ^ b;
    v4si shuffle2 = __builtin_shuffle(c, d, mask);
    KEEP(shuffle2);
    
    BARRIER();
    
#ifdef __AVX__
    /* AVX operations that might use more operands */
    v8si avx_a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si avx_b = {8, 7, 6, 5, 4, 3, 2, 1};
    v8si avx_c = {2, 3, 4, 5, 6, 7, 8, 9};
    
    /* Complex AVX expression */
    v8si avx_result = (avx_a > avx_b) ? (avx_a + avx_c) : (avx_b - avx_c);
    volatile v8si keep_avx = avx_result;
    (void)keep_avx;
#endif
    
    /* Final combination of results */
    v4si final_result = shuffled + cmp_result + blend_result + mixed + shuffle2;
    
    /* Add some float influence */
    v4si float_influence = __builtin_convertvector(fcmp, v4si);
    final_result += float_influence;
    
    return final_result;
}

/* Second test function focusing on builtins */
__attribute__((noinline, target("sse4.1,avx")))
v4sf test_builtins_many_operands(v4sf a, v4sf b, v4sf c, v4sf d) {
    volatile v4sf v1, v2, v3, v4;
    
    /* Use SSE4.1 blend builtin if available */
#ifdef __SSE4_1__
    __m128 ma = _mm_load_ps((float*)&a);
    __m128 mb = _mm_load_ps((float*)&b);
    __m128 mc = _mm_load_ps((float*)&c);
    __m128 md = _mm_load_ps((float*)&d);
    
    /* Chain blend operations */
    __m128 blend1 = _mm_blendv_ps(ma, mb, mc);
    __m128 blend2 = _mm_blendv_ps(mb, mc, md);
    __m128 blend3 = _mm_blendv_ps(blend1, blend2, ma);
    
    /* Rounding operations */
    __m128 round1 = _mm_round_ps(blend1, _MM_FROUND_TO_NEAREST_INT);
    __m128 round2 = _mm_round_ps(blend2, _MM_FROUND_TO_NEG_INF);
    
    v4sf result;
    _mm_store_ps((float*)&result, round1 + round2 + blend3);
    KEEP(result);
    
    BARRIER();
#endif
    
    /* Complex conditional with multiple operations */
    v4sf cmp = (a > b);
    v4sf true_val = (c * d) + (a / b);
    v4sf false_val = (c / d) - (a * b);
    v4sf cond_result = cmp ? true_val : false_val;
    
    /* Another layer */
    v4sf cmp2 = (cond_result > a);
    v4sf true_val2 = cond_result * b;
    v4sf false_val2 = cond_result / b;
    v4sf final = cmp2 ? true_val2 : false_val2;
    
    return final;
}

int main() {
    /* Initialize test vectors */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {9, 10, 11, 12};
    v4si d = {13, 14, 15, 16};
    
    v4sf fa = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fb = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf fc = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf fd = {13.0f, 14.0f, 15.0f, 16.0f};
    
    /* Call test functions */
    v4si int_result = test_10_11_operands(a, b, c, d, fa, fb, fc, fd);
    v4sf float_result = test_builtins_many_operands(fa, fb, fc, fd);
    
    /* Compute checksums to prevent dead code elimination */
    int int_sum = 0;
    float float_sum = 0.0f;
    
    for (int i = 0; i < 4; i++) {
        int_sum += int_result[i];
        float_sum += float_result[i];
    }
    
    /* Use results */
    printf("Int checksum: %d\n", int_sum);
    printf("Float checksum: %f\n", float_sum);
    
    /* Return based on results to ensure execution */
    return (int_sum > 0 && float_sum > 0.0f) ? 0 : 1;
}
