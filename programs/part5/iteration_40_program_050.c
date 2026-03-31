#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __AVX512F__

/* V16SFmode: 16 single-precision floats */
__attribute__((noinline))
float test_v16sf_blend() {
    __m512 a = _mm512_setr_ps(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                              9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f);
    __m512 b = _mm512_setr_ps(100.0f, 200.0f, 300.0f, 400.0f, 500.0f, 600.0f, 700.0f, 800.0f,
                              900.0f, 1000.0f, 1100.0f, 1200.0f, 1300.0f, 1400.0f, 1500.0f, 1600.0f);
    
    // Create alternating mask: 0xAAAA = 1010101010101010
    __mmask16 mask = 0xAAAA;
    
    // Blend using mask: vblendmps
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    // Force usage by extracting and summing
    float sum = 0.0f;
    float* res = (float*)&result;
    for (int i = 0; i < 16; i++) {
        sum += res[i];
    }
    return sum;
}

/* V8DFmode: 8 double-precision floats */
__attribute__((noinline))
double test_v8df_blend() {
    __m512d a = _mm512_setr_pd(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
    __m512d b = _mm512_setr_pd(100.0, 200.0, 300.0, 400.0, 500.0, 600.0, 700.0, 800.0);
    
    // Create mask: 0xAA = 10101010
    __mmask8 mask = 0xAA;
    
    // Blend using mask: vblendmpd
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    // Force usage
    double sum = 0.0;
    double* res = (double*)&result;
    for (int i = 0; i < 8; i++) {
        sum += res[i];
    }
    return sum;
}

/* V16SImode: 16 32-bit integers */
__attribute__((noinline))
int64_t test_v16si_blend() {
    __m512i a = _mm512_setr_epi32(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    __m512i b = _mm512_setr_epi32(100, 200, 300, 400, 500, 600, 700, 800, 
                                  900, 1000, 1100, 1200, 1300, 1400, 1500, 1600);
    
    // Create alternating mask
    __mmask16 mask = 0xAAAA;
    
    // Blend using mask: vblendmd
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    // Force usage
    int32_t* res = (int32_t*)&result;
    int64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += res[i];
    }
    return sum;
}

/* V8DImode: 8 64-bit integers */
__attribute__((noinline))
int64_t test_v8di_blend() {
    __m512i a = _mm512_setr_epi64(1, 2, 3, 4, 5, 6, 7, 8);
    __m512i b = _mm512_setr_epi64(100, 200, 300, 400, 500, 600, 700, 800);
    
    // Create alternating mask
    __mmask8 mask = 0xAA;
    
    // Blend using mask: vblendmq
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    // Force usage
    int64_t* res = (int64_t*)&result;
    int64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += res[i];
    }
    return sum;
}

#ifdef __AVX512BW__

/* V64QImode: 64 8-bit integers */
__attribute__((noinline))
int64_t test_v64qi_blend() {
    // Initialize with pattern
    int8_t a_arr[64], b_arr[64];
    for (int i = 0; i < 64; i++) {
        a_arr[i] = i;
        b_arr[i] = i + 100;
    }
    
    __m512i a = _mm512_loadu_si512((__m512i*)a_arr);
    __m512i b = _mm512_loadu_si512((__m512i*)b_arr);
    
    // Create complex mask pattern
    __mmask64 mask = 0xAAAAAAAAAAAAAAAAULL;
    
    // Blend using mask: vblendmb
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Force usage
    int8_t* res = (int8_t*)&result;
    int64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += res[i];
    }
    return sum;
}

/* V32HImode: 32 16-bit integers */
__attribute__((noinline))
int64_t test_v32hi_blend() {
    // Initialize with pattern
    int16_t a_arr[32], b_arr[32];
    for (int i = 0; i < 32; i++) {
        a_arr[i] = i;
        b_arr[i] = i + 100;
    }
    
    __m512i a = _mm512_loadu_si512((__m512i*)a_arr);
    __m512i b = _mm512_loadu_si512((__m512i*)b_arr);
    
    // Create alternating mask
    __mmask32 mask = 0xAAAAAAAA;
    
    // Blend using mask: vblendmw
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    // Force usage
    int16_t* res = (int16_t*)&result;
    int64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += res[i];
    }
    return sum;
}

#endif // __AVX512BW__

#ifdef __AVX512FP16__

/* V32HFmode: 32 half-precision floats */
__attribute__((noinline))
float test_v32hf_blend() {
    // Initialize arrays
    _Float16 a_arr[32], b_arr[32];
    for (int i = 0; i < 32; i++) {
        a_arr[i] = (_Float16)(i * 0.5f);
        b_arr[i] = (_Float16)(i * 2.0f);
    }
    
    __m512h a = _mm512_loadu_ph(a_arr);
    __m512h b = _mm512_loadu_ph(b_arr);
    
    // Create alternating mask
    __mmask32 mask = 0xAAAAAAAA;
    
    // Blend using mask: vblendmps for half-precision
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    // Force usage
    _Float16* res = (_Float16*)&result;
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)res[i];
    }
    return sum;
}

/* V32BFmode: 32 brain float (bfloat16) */
__attribute__((noinline))
float test_v32bf_blend() {
    // Initialize arrays with bfloat16 values
    // Note: We'll use __m512bh type and appropriate intrinsics
    __m512bh a, b;
    
    // Create some test data
    uint16_t a_data[32], b_data[32];
    for (int i = 0; i < 32; i++) {
        // Simple bfloat16 pattern
        a_data[i] = i << 8;  // bfloat16 representation
        b_data[i] = (i + 32) << 8;
    }
    
    a = _mm512_loadu_si512((__m512i*)a_data);
    b = _mm512_loadu_si512((__m512i*)b_data);
    
    // Create alternating mask
    __mmask32 mask = 0x55555555;  // Different pattern
    
    // For bfloat16, we can use integer blend since there's no direct BF16 blend intrinsic
    // This should still trigger the V32BFmode case
    __m512bh result = _mm512_mask_blend_epi16(mask, a, b);
    
    // Force usage
    uint16_t* res = (uint16_t*)&result;
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        // Convert bfloat16 to float for sum
        uint32_t temp = res[i] << 16;
        float f;
        memcpy(&f, &temp, sizeof(float));
        sum += f;
    }
    return sum;
}

#endif // __AVX512FP16__

#endif // __AVX512F__

int main() {
    double total = 0.0;
    
#ifdef __AVX512F__
    printf("Testing AVX-512F blend operations...\n");
    
    total += test_v16sf_blend();
    printf("  V16SFmode blend test completed\n");
    
    total += test_v8df_blend();
    printf("  V8DFmode blend test completed\n");
    
    total += test_v16si_blend();
    printf("  V16SImode blend test completed\n");
    
    total += test_v8di_blend();
    printf("  V8DImode blend test completed\n");
    
#ifdef __AVX512BW__
    printf("Testing AVX-512BW blend operations...\n");
    
    total += test_v64qi_blend();
    printf("  V64QImode blend test completed\n");
    
    total += test_v32hi_blend();
    printf("  V32HImode blend test completed\n");
#endif
    
#ifdef __AVX512FP16__
    printf("Testing AVX-512FP16 blend operations...\n");
    
    total += test_v32hf_blend();
    printf("  V32HFmode blend test completed\n");
    
    total += test_v32bf_blend();
    printf("  V32BFmode blend test completed\n");
#endif
    
    printf("All tests completed. Total sum: %f\n", total);
#else
    printf("AVX-512 not supported on this compiler/platform.\n");
#endif
    
    return 0;
}
