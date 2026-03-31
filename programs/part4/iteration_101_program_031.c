/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Prevent optimization */
#define KEEP(expr) do { \
    volatile __typeof__(expr) _tmp = (expr); \
    asm volatile("" : : "r"(&_tmp) : "memory"); \
} while(0)

/* Compiler barrier */
#define BARRIER() asm volatile("" : : : "memory")

/* Vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* AVX types */
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));

/* AVX-512 types */
typedef float v16sf __attribute__((vector_size(64)));
typedef double v8df __attribute__((vector_size(64)));

/* Complex shuffle with many operands - may expand to 10+ operands */
static v4sf __attribute__((noinline))
test_shuffle_complex(v4sf a, v4sf b, v4sf c, v4sf d, v4si mask)
{
    /* Multiple shuffle operations that might combine */
    v4sf t1 = __builtin_shuffle(a, b, (v4si){3, 1, 0, 2});
    BARRIER();
    
    v4sf t2 = __builtin_shuffle(c, d, (v4si){1, 3, 2, 0});
    BARRIER();
    
    /* Conditional blend based on mask */
    v4sf mask_f = __builtin_convertvector(mask, v4sf);
    v4sf blend = (mask_f > (v4sf){0}) ? t1 * t2 : t1 + t2;
    BARRIER();
    
    /* Another shuffle with the blend result */
    v4sf result = __builtin_shuffle(blend, a + b, (v4si){2, 3, 0, 1});
    
    KEEP(result);
    return result;
}

/* AVX operation chain - may require many operands during expansion */
static v8sf __attribute__((noinline))
test_avx_chain(v8sf a, v8sf b, v8sf c, v8sf d, v8sf e)
{
    /* Complex expression that might expand to many operands */
    v8sf t1 = a * b + c;
    BARRIER();
    
    v8sf t2 = __builtin_shufflevector(a, b, 0, 9, 2, 11, 4, 13, 6, 15);
    BARRIER();
    
    v8sf t3 = (t1 > t2) ? t1 * d : t2 * e;
    BARRIER();
    
    /* Mix with conversion */
    v4df hi = __builtin_convertvector((v4sf){t3[0], t3[1], t3[2], t3[3]}, v4df);
    v4df lo = __builtin_convertvector((v4sf){t3[4], t3[5], t3[6], t3[7]}, v4df);
    BARRIER();
    
    /* Force memory operations */
    volatile v4df vhi = hi;
    volatile v4df vlo = lo;
    BARRIER();
    
    /* Recombine */
    v4sf hi_f = __builtin_convertvector(vhi, v4sf);
    v4sf lo_f = __builtin_convertvector(vlo, v4sf);
    
    v8sf result = {lo_f[0], lo_f[1], lo_f[2], lo_f[3],
                   hi_f[0], hi_f[1], hi_f[2], hi_f[3]};
    
    KEEP(result);
    return result;
}

/* Test with intrinsics that might use many operands */
#ifdef __AVX512F__
static v8df __attribute__((noinline))
test_avx512_many_operands(v8df a, v8df b, v8df c, v8df d, 
                          v8df e, v8df f, __mmask8 mask)
{
    /* AVX-512 operations with masking - likely to need many operands */
    v8df t1 = _mm512_mask_add_pd(a, mask, b, c);
    BARRIER();
    
    v8df t2 = _mm512_mask_mul_pd(d, mask, e, f);
    BARRIER();
    
    v8df t3 = _mm512_mask_sub_pd(t1, mask, t2, a);
    BARRIER();
    
    /* Complex blend */
    __mmask8 mask2 = _mm512_cmp_pd_mask(t3, b, _CMP_GT_OQ);
    v8df result = _mm512_mask_blend_pd(mask2, t3, t1 + t2);
    
    KEEP(result);
    return result;
}
#endif

/* Main test function that combines everything */
static v4sf __attribute__((noinline, optimize("no-tree-vectorize")))
test_many_operands(v4sf a, v4sf b, v4sf c, v4sf d, 
                   v4si mask, v8sf avx_a, v8sf avx_b)
{
    /* Test 1: Complex shuffle with conditional */
    v4sf r1 = test_shuffle_complex(a, b, c, d, mask);
    BARRIER();
    
    /* Test 2: AVX chain */
    v8sf r2 = test_avx_chain(avx_a, avx_b, avx_a + avx_b, 
                            avx_a * avx_b, avx_a - avx_b);
    BARRIER();
    
    /* Convert AVX result to SSE for combination */
    v4sf r2_lo = {r2[0], r2[1], r2[2], r2[3]};
    v4sf r2_hi = {r2[4], r2[5], r2[6], r2[7]};
    
    /* Final combination with many operands */
    v4sf result = r1 + r2_lo * r2_hi - (r2_lo + r2_hi) / r1;
    
    /* Conditional with vector comparison */
    v4sf cmp_mask = (r1 > r2_lo);
    result = cmp_mask ? result * 2.0f : result / 2.0f;
    
    KEEP(result);
    return result;
}

int main(void)
{
    /* Initialize SSE vectors */
    v4sf a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf b = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf c = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf d = {13.0f, 14.0f, 15.0f, 16.0f};
    v4si mask = {0, 1, 0, 1};
    
    /* Initialize AVX vectors */
    v8sf avx_a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf avx_b = {9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    
    /* Call test function */
    v4sf result = test_many_operands(a, b, c, d, mask, avx_a, avx_b);
    
    /* Compute checksum to prevent dead code elimination */
    float checksum = 0.0f;
    for (int i = 0; i < 4; i++) {
        checksum += result[i];
    }
    
    /* Use checksum to affect program output */
    printf("Result checksum: %f\n", checksum);
    
    /* Return based on checksum to ensure execution */
    return (checksum > 100.0f) ? 1 : 0;
}
