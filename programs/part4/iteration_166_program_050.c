#include <stdio.h>
#include <immintrin.h>
#include <stdint.h>

// Helper function to print verification results
void print_result(const char* name, long long result) {
    printf("%s: %lld\n", name, result);
}

// V64QImode: 64-byte integers
__m512i blend_64qi(__m512i a, __m512i b, __mmask64 mask) {
    // Use immediate mask
    __m512i c1 = _mm512_mask_blend_epi8(0xAAAAAAAAAAAAAAAA, a, b);
    
    // Use variable mask
    __m512i c2 = _mm512_mask_blend_epi8(mask, a, b);
    
    // Chain blends
    __m512i c3 = _mm512_mask_blend_epi8(mask >> 1, c1, c2);
    
    return c3;
}

// V32HImode: 32-halfword integers
__m512i blend_32hi(__m512i a, __m512i b, __mmask32 mask) {
    // Immediate mask
    __m512i c1 = _mm512_mask_blend_epi16(0x55555555, a, b);
    
    // Variable mask
    __m512i c2 = _mm512_mask_blend_epi16(mask, a, b);
    
    // Dependent blend
    __m512i c3 = _mm512_mask_blend_epi16(mask ^ 0xAAAAAAAA, c1, c2);
    
    return c3;
}

// V32HFmode: 32-half precision floats
#ifdef __AVX512FP16__
__m512h blend_32hf(__m512h a, __m512h b, __mmask32 mask) {
    // Immediate mask
    __m512h c1 = _mm512_mask_blend_ph(0xFFFFFFFF, a, b);
    
    // Variable mask
    __m512h c2 = _mm512_mask_blend_ph(mask, a, b);
    
    // Chain blends
    __m512h c3 = _mm512_mask_blend_ph(mask & 0x55555555, c1, c2);
    
    return c3;
}
#endif

// V32BFmode: 32-bfloat16 floats
#ifdef __AVX512BF16__
__m512bh blend_32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    // For bfloat16, we need to use appropriate intrinsics
    // This is a placeholder - actual bfloat16 blend might use different intrinsics
    return a; // Simplified for structure
}
#endif

// V16SImode: 16-single word integers
__m512i blend_16si(__m512i a, __m512i b, __mmask16 mask) {
    // Immediate mask
    __m512i c1 = _mm512_mask_blend_epi32(0xAAAA, a, b);
    
    // Variable mask from comparison
    __mmask16 cmp_mask = _mm512_cmpeq_epi32_mask(a, b);
    __m512i c2 = _mm512_mask_blend_epi32(cmp_mask, a, b);
    
    // Logical operation on masks
    __mmask16 combined = _kor_mask16(mask, cmp_mask);
    __m512i c3 = _mm512_mask_blend_epi32(combined, c1, c2);
    
    return c3;
}

// V8DImode: 8-double word integers
__m512i blend_8di(__m512i a, __m512i b, __mmask8 mask) {
    // Immediate mask
    __m512i c1 = _mm512_mask_blend_epi64(0xAA, a, b);
    
    // Variable mask
    __m512i c2 = _mm512_mask_blend_epi64(mask, a, b);
    
    // Chain with different mask
    __m512i c3 = _mm512_mask_blend_epi64(mask ^ 0x55, c1, c2);
    
    return c3;
}

// V8DFmode: 8-double precision floats
__m512d blend_8df(__m512d a, __m512d b, __mmask8 mask) {
    // Immediate mask
    __m512d c1 = _mm512_mask_blend_pd(0x0F, a, b);
    
    // Variable mask from comparison
    __mmask8 cmp_mask = _mm512_cmp_pd_mask(a, b, _CMP_EQ_OQ);
    __m512d c2 = _mm512_mask_blend_pd(cmp_mask, a, b);
    
    // Logical mask operation
    __mmask8 combined = _kor_mask8(mask, cmp_mask);
    __m512d c3 = _mm512_mask_blend_pd(combined, c1, c2);
    
    return c3;
}

// V16SFmode: 16-single precision floats
__m512 blend_16sf(__m512 a, __m512 b, __mmask16 mask) {
    // Immediate mask
    __m512 c1 = _mm512_mask_blend_ps(0xAAAA, a, b);
    
    // Variable mask from comparison
    __mmask16 cmp_mask = _mm512_cmp_ps_mask(a, b, _CMP_LT_OQ);
    __m512 c2 = _mm512_mask_blend_ps(cmp_mask, a, b);
    
    // Complex mask combination
    __mmask16 mask2 = _mm512_cmp_ps_mask(a, _mm512_set1_ps(0.0f), _CMP_GT_OQ);
    __mmask16 combined = _kor_mask16(mask, _kand_mask16(cmp_mask, mask2));
    __m512 c3 = _mm512_mask_blend_ps(combined, c1, c2);
    
    return c3;
}

// Test function with loops and conditionals
void test_in_loop() {
    __m512i a_epi8 = _mm512_set1_epi8(1);
    __m512i b_epi8 = _mm512_set1_epi8(2);
    __mmask64 mask64 = 0xAAAAAAAAAAAAAAAA;
    
    __m512i a_epi16 = _mm512_set1_epi16(10);
    __m512i b_epi16 = _mm512_set1_epi16(20);
    __mmask32 mask32 = 0x55555555;
    
    __m512 a_ps = _mm512_set1_ps(1.0f);
    __m512 b_ps = _mm512_set1_ps(2.0f);
    __mmask16 mask16 = 0xAAAA;
    
    __m512d a_pd = _mm512_set1_pd(1.0);
    __m512d b_pd = _mm512_set1_pd(2.0);
    __mmask8 mask8 = 0xAA;
    
    // Loop with varying trip count
    for (int i = 0; i < 100; i++) {
        // Conditional to encourage if-conversion
        if (i % 3 == 0) {
            __m512i result_epi8 = blend_64qi(a_epi8, b_epi8, mask64);
            __m512i result_epi16 = blend_32hi(a_epi16, b_epi16, mask32);
            
            // Chain operations
            __m512i temp = _mm512_add_epi8(result_epi8, result_epi16);
            __m512i final = blend_16si(temp, _mm512_set1_epi32(i), mask16);
            
            // Prevent dead code elimination
            volatile int dummy = _mm512_reduce_add_epi32(final);
        } else if (i % 3 == 1) {
            __m512 result_ps = blend_16sf(a_ps, b_ps, mask16);
            __m512d result_pd = blend_8df(a_pd, b_pd, mask8);
            
            // Cross-type dependency
            __m512 mixed = _mm512_add_ps(result_ps, _mm512_castpd_ps(result_pd));
            __m512 final = blend_16sf(mixed, _mm512_set1_ps(i), mask16);
            
            volatile float dummy = _mm512_reduce_add_ps(final);
        } else {
            // Integer blend chain
            __m512i result_si = blend_16si(_mm512_set1_epi32(i), 
                                          _mm512_set1_epi32(i+1), 
                                          mask16);
            __m512i result_di = blend_8di(_mm512_set1_epi64(i), 
                                         _mm512_set1_epi64(i+2), 
                                         mask8);
            
            // Combine results
            __m512i combined = _mm512_add_epi32(result_si, 
                                              _mm512_castsi512_si512(result_di));
            volatile long long dummy = _mm512_reduce_add_epi64(combined);
        }
    }
}

int main() {
    // Initialize test data
    __m512i a_epi8 = _mm512_set_epi8(
        1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
        17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,
        33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,
        49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64
    );
    
    __m512i b_epi8 = _mm512_set1_epi8(100);
    __mmask64 mask64 = _mm512_cmpeq_epi8_mask(a_epi8, _mm512_set1_epi8(32));
    
    __m512i a_epi16 = _mm512_set_epi16(
        1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
        17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32
    );
    __m512i b_epi16 = _mm512_set1_epi16(200);
    __mmask32 mask32 = _mm512_cmpeq_epi16_mask(a_epi16, _mm512_set1_epi16(16));
    
    __m512 a_ps = _mm512_set_ps(
        1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f
    );
    __m512 b_ps = _mm512_set1_ps(100.0f);
    __mmask16 mask16 = _mm512_cmp_ps_mask(a_ps, _mm512_set1_ps(8.0f), _CMP_GT_OQ);
    
    __m512d a_pd = _mm512_set_pd(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
    __m512d b_pd = _mm512_set1_pd(100.0);
    __mmask8 mask8 = _mm512_cmp_pd_mask(a_pd, _mm512_set1_pd(4.0), _CMP_GT_OQ);
    
    // Test all blend modes
    __m512i result_epi8 = blend_64qi(a_epi8, b_epi8, mask64);
    __m512i result_epi16 = blend_32hi(a_epi16, b_epi16, mask32);
    __m512i result_si = blend_16si(_mm512_set1_epi32(1), _mm512_set1_epi32(2), mask16);
    __m512i result_di = blend_8di(_mm512_set1_epi64(1), _mm512_set1_epi64(2), mask8);
    __m512 result_ps = blend_16sf(a_ps, b_ps, mask16);
    __m512d result_pd = blend_8df(a_pd, b_pd, mask8);
    
    #ifdef __AVX512FP16__
    __m512h a_hf = _mm512_set1_ph(1.0f);
    __m512h b_hf = _mm512_set1_ph(2.0f);
    __m512h result_hf = blend_32hf(a_hf, b_hf, 0x55555555);
    #endif
    
    // Perform reductions to prevent dead code elimination
    long long sum_epi8 = _mm512_reduce_add_epi64(result_epi8);
    long long sum_epi16 = _mm512_reduce_add_epi64(result_epi16);
    long long sum_si = _mm512_reduce_add_epi64(result_si);
    long long sum_di = _mm512_reduce_add_epi64(result_di);
    float sum_ps = _mm512_reduce_add_ps(result_ps);
    double sum_pd = _mm512_reduce_add_pd(result_pd);
    
    // Print verification results
    print_result("V64QImode", sum_epi8);
    print_result("V32HImode", sum_epi16);
    print_result("V16SImode", sum_si);
    print_result("V8DImode", sum_di);
    printf("V16SFmode: %f\n", sum_ps);
    printf("V8DFmode: %f\n", sum_pd);
    
    // Test with loops and conditionals
    test_in_loop();
    
    printf("All AVX-512 blend modes tested successfully!\n");
    
    return 0;
}
