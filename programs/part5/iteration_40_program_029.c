#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __AVX512F__

/* V16SFmode: 16 single-precision floats */
__attribute__((noinline))
float test_v16sf_blend() {
    __m512 a = _mm512_setr_ps(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                              9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f);
    __m512 b = _mm512_setr_ps(17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f, 24.0f,
                              25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f, 32.0f);
    
    // Create alternating mask: 0xAAAA = 1010101010101010
    __mmask16 mask = 0xAAAA;
    
    // This should generate vblendmps
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    // Extract and sum to prevent optimization
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
    __m512d b = _mm512_setr_pd(9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0);
    
    // Create mask: 0xAA = 10101010
    __mmask8 mask = 0xAA;
    
    // This should generate vblendmpd
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
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
    __m512i b = _mm512_setr_epi32(17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32);
    
    // Alternating mask
    __mmask16 mask = 0xAAAA;
    
    // This should generate vblendmd
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    int64_t sum = 0;
    int32_t* res = (int32_t*)&result;
    for (int i = 0; i < 16; i++) {
        sum += res[i];
    }
    return sum;
}

/* V8DImode: 8 64-bit integers */
__attribute__((noinline))
int64_t test_v8di_blend() {
    __m512i a = _mm512_setr_epi64(1, 2, 3, 4, 5, 6, 7, 8);
    __m512i b = _mm512_setr_epi64(9, 10, 11, 12, 13, 14, 15, 16);
    
    // Alternating mask
    __mmask8 mask = 0xAA;
    
    // This should generate vblendmq
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    int64_t sum = 0;
    int64_t* res = (int64_t*)&result;
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
    uint8_t a_arr[64], b_arr[64];
    for (int i = 0; i < 64; i++) {
        a_arr[i] = i;
        b_arr[i] = i + 64;
    }
    
    __m512i a = _mm512_loadu_si512(a_arr);
    __m512i b = _mm512_loadu_si512(b_arr);
    
    // Create complex mask pattern
    __mmask64 mask = 0xAAAAAAAAAAAAAAAAULL;
    
    // This should generate vblendmb
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Store and sum to prevent optimization
    uint8_t res_arr[64];
    _mm512_storeu_si512(res_arr, result);
    
    int64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += res_arr[i];
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
        b_arr[i] = i + 32;
    }
    
    __m512i a = _mm512_loadu_si512(a_arr);
    __m512i b = _mm512_loadu_si512(b_arr);
    
    // Alternating mask
    __mmask32 mask = 0xAAAAAAAA;
    
    // This should generate vblendmw
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    // Store and sum
    int16_t res_arr[32];
    _mm512_storeu_si512(res_arr, result);
    
    int64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += res_arr[i];
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
        a_arr[i] = (_Float16)(i + 1);
        b_arr[i] = (_Float16)(i + 33);
    }
    
    __m512h a = _mm512_loadu_ph(a_arr);
    __m512h b = _mm512_loadu_ph(b_arr);
    
    // Alternating mask
    __mmask32 mask = 0xAAAAAAAA;
    
    // This should generate vblendmps for half-precision
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    // Store and convert to float for summation
    _Float16 res_arr[32];
    _mm512_storeu_ph(res_arr, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)res_arr[i];
    }
    return sum;
}

/* V32BFmode: 32 brain float (bfloat16) */
__attribute__((noinline))
float test_v32bf_blend() {
    // Initialize arrays with bfloat16 pattern
    // Note: We'll use __m512bh which is the bfloat16 vector type
    __m512bh a = _mm512_set1_epi16(0x3F80); // 1.0 in bfloat16
    __m512bh b = _mm512_set1_epi16(0x4000); // 2.0 in bfloat16
    
    // Create alternating mask
    __mmask32 mask = 0x55555555; // Different pattern from HF mode
    
    // Use integer blend since there's no direct bfloat16 blend intrinsic
    // The compiler should recognize this as bfloat16 blend
    __m512i a_int = _mm512_castps_si512(_mm512_castbh_ps(a));
    __m512i b_int = _mm512_castps_si512(_mm512_castbh_ps(b));
    
    // This should generate vblendmps for bfloat16
    __m512i result_int = _mm512_mask_blend_epi16(mask, a_int, b_int);
    
    // Convert back to bfloat16 vector for storage
    __m512bh result = _mm512_castsi512_bh(result_int);
    
    // Store and process to prevent optimization
    uint16_t res_arr[32];
    _mm512_storeu_si512(res_arr, result_int);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        // Convert bfloat16 to float (simplified)
        uint32_t val = ((uint32_t)res_arr[i]) << 16;
        float f;
        memcpy(&f, &val, sizeof(float));
        sum += f;
    }
    return sum;
}

#endif // __AVX512FP16__

#endif // __AVX512F__

int main() {
    printf("Testing AVX-512 blend operations...\n");
    
#ifdef __AVX512F__
    printf("V16SFmode (single-precision): %f\n", test_v16sf_blend());
    printf("V8DFmode (double-precision): %f\n", test_v8df_blend());
    printf("V16SImode (32-bit integers): %ld\n", test_v16si_blend());
    printf("V8DImode (64-bit integers): %ld\n", test_v8di_blend());
#endif

#ifdef __AVX512BW__
    printf("V64QImode (8-bit integers): %ld\n", test_v64qi_blend());
    printf("V32HImode (16-bit integers): %ld\n", test_v32hi_blend());
#endif

#ifdef __AVX512FP16__
    printf("V32HFmode (half-precision): %f\n", test_v32hf_blend());
    printf("V32BFmode (bfloat16): %f\n", test_v32bf_blend());
#endif
    
    printf("Done.\n");
    return 0;
}
