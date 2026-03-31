/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Define vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Larger vector types for AVX */
#ifdef __AVX__
typedef int v8si __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));
#endif

/* AVX-512 types if available */
#ifdef __AVX512F__
typedef int v16si __attribute__((vector_size(64)));
typedef double v8df __attribute__((vector_size(64)));
#endif

/* Force no inlining to prevent optimization */
__attribute__((noinline, noipa))
static v4si test_10_operands(v4si a, v4si b, v4si c, v4si d, 
                             v4si mask1, v4si mask2, v4si shuffle_mask)
{
    volatile v4si temp1, temp2, temp3;
    
    /* Complex shuffle operation - may require many operands */
    v4si shuffled1 = __builtin_shuffle(a, b, shuffle_mask);
    temp1 = shuffled1;
    
    /* Vector conditional with arithmetic - generates VEC_COND_EXPR */
    v4si cmp_result = (mask1 > mask2) ? a * b + c : d - a;
    temp2 = cmp_result;
    
    /* Another shuffle with different inputs */
    v4si shuffled2 = __builtin_shuffle(c, d, shuffle_mask + (v4si){1,2,3,4});
    
    /* Blend-like operation using conditional */
    v4si blended = (mask1 != (v4si){0}) ? shuffled1 : shuffled2;
    
    /* Compiler barrier to prevent reordering */
    asm volatile("" ::: "memory");
    
    /* Complex expression that might expand to multiple operations */
    v4si result = blended * cmp_result + shuffled2 - a;
    
    /* Store to volatile to force all operations */
    temp3 = result;
    
    return result;
}

__attribute__((noinline, noipa))
static v4df test_11_operands_fp(v4df a, v4df b, v4df c, v4df d,
                                v4df e, v4df f, v4df mask)
{
    volatile v4df temp1, temp2;
    
    /* Complex FP vector operations */
    v4df t1 = a * b + c;
    v4df t2 = d - e * f;
    
    /* Vector conditional with FP comparison */
    v4df cmp = (mask > (v4df){0.5, 0.5, 0.5, 0.5}) ? t1 : t2;
    temp1 = cmp;
    
    /* Mix of operations that might require many operands */
    v4df result = __builtin_round(cmp) + 
                  __builtin_trunc(t1) - 
                  __builtin_ceil(t2);
    
    /* Additional barrier */
    asm volatile("" ::: "memory");
    
    /* Another conditional */
    result = (a > b) ? result * 2.0 : result / 2.0;
    
    temp2 = result;
    
    return result;
}

/* Test with SSE2 intrinsics as well */
#include <emmintrin.h>
#include <immintrin.h>

__attribute__((noinline, noipa))
static __m128i test_mmx_operands(__m128i a, __m128i b, __m128i c, 
                                 __m128i d, __m128i mask)
{
    volatile __m128i temp;
    
    /* Chain of operations that might require many operands */
    __m128i t1 = _mm_add_epi32(a, b);
    __m128i t2 = _mm_sub_epi32(c, d);
    __m128i t3 = _mm_mullo_epi16(a, b);
    
    /* Blend operation - can require many operands */
    __m128i blended = _mm_blendv_epi8(t1, t2, mask);
    
    /* Shuffle */
    __m128i shuffled = _mm_shuffle_epi32(blended, _MM_SHUFFLE(1,0,3,2));
    
    /* Another blend */
    __m128i result = _mm_blend_epi16(shuffled, t3, 0xCC);
    
    temp = result;
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    return result;
}

/* Main test function that combines everything */
__attribute__((noinline, noipa))
static int run_vector_tests(void)
{
    /* Initialize vectors with pattern */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {9, 10, 11, 12};
    v4si d = {13, 14, 15, 16};
    v4si mask1 = {0, 1, 0, 1};
    v4si mask2 = {1, 0, 1, 0};
    v4si shuffle_mask = {3, 2, 1, 0};
    
    /* Test 10 operand path */
    v4si result1 = test_10_operands(a, b, c, d, mask1, mask2, shuffle_mask);
    
    /* Test with FP vectors if AVX available */
#ifdef __AVX__
    v4df fa = {1.0, 2.0, 3.0, 4.0};
    v4df fb = {5.0, 6.0, 7.0, 8.0};
    v4df fc = {9.0, 10.0, 11.0, 12.0};
    v4df fd = {13.0, 14.0, 15.0, 16.0};
    v4df fe = {17.0, 18.0, 19.0, 20.0};
    v4df ff = {21.0, 22.0, 23.0, 24.0};
    v4df fmask = {0.1, 0.6, 0.1, 0.6};
    
    v4df result2 = test_11_operands_fp(fa, fb, fc, fd, fe, ff, fmask);
#endif
    
    /* Test with MMX/SSE intrinsics */
    __m128i ma = _mm_set_epi32(1, 2, 3, 4);
    __m128i mb = _mm_set_epi32(5, 6, 7, 8);
    __m128i mc = _mm_set_epi32(9, 10, 11, 12);
    __m128i md = _mm_set_epi32(13, 14, 15, 16);
    __m128i mmask = _mm_set_epi32(0xFF00FF00, 0x00FF00FF, 0xFF00FF00, 0x00FF00FF);
    
    __m128i result3 = test_mmx_operands(ma, mb, mc, md, mmask);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 4; i++) {
        checksum += result1[i];
    }
    
#ifdef __AVX__
    for (int i = 0; i < 4; i++) {
        checksum += (int)result2[i];
    }
#endif
    
    int* r3 = (int*)&result3;
    for (int i = 0; i < 4; i++) {
        checksum += r3[i];
    }
    
    return checksum;
}

int main(void)
{
    int result = run_vector_tests();
    
    /* Print result to ensure execution */
    printf("Vector test checksum: %d\n", result);
    
    /* Return non-zero if checksum is 0 (unlikely) */
    return (result == 0) ? 1 : 0;
}
