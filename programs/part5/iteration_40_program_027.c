#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifdef __AVX512F__

/* V16SFmode: 16 single-precision floats */
__attribute__((noinline))
__m512 test_v16sf_blend(__m512 a, __m512 b, __mmask16 mask) {
    return _mm512_mask_blend_ps(mask, a, b);
}

/* V8DFmode: 8 double-precision floats */
__attribute__((noinline))
__m512d test_v8df_blend(__m512d a, __m512d b, __mmask8 mask) {
    return _mm512_mask_blend_pd(mask, a, b);
}

/* V16SImode: 16 32-bit integers */
__attribute__((noinline))
__m512i test_v16si_blend(__m512i a, __m512i b, __mmask16 mask) {
    return _mm512_mask_blend_epi32(mask, a, b);
}

/* V8DImode: 8 64-bit integers */
__attribute__((noinline))
__m512i test_v8di_blend(__m512i a, __m512i b, __mmask8 mask) {
    return _mm512_mask_blend_epi64(mask, a, b);
}

#ifdef __AVX512BW__
/* V64QImode: 64 8-bit integers */
__attribute__((noinline))
__m512i test_v64qi_blend(__m512i a, __m512i b, __mmask64 mask) {
    return _mm512_mask_blend_epi8(mask, a, b);
}

/* V32HImode: 32 16-bit integers */
__attribute__((noinline))
__m512i test_v32hi_blend(__m512i a, __m512i b, __mmask32 mask) {
    return _mm512_mask_blend_epi16(mask, a, b);
}
#endif /* __AVX512BW__ */

#ifdef __AVX512FP16__
/* V32HFmode: 32 half-precision floats */
__attribute__((noinline))
__m512h test_v32hf_blend(__m512h a, __m512h b, __mmask32 mask) {
    return _mm512_mask_blend_ph(mask, a, b);
}

/* V32BFmode: 32 brain floats (bfloat16) */
__attribute__((noinline))
__m512bh test_v32bf_blend(__m512bh a, __m512bh b, __mmask32 mask) {
    return _mm512_mask_blend_epi16(mask, 
                                   (__m512i)a, 
                                   (__m512i)b);
}
#endif /* __AVX512FP16__ */

#endif /* __AVX512F__ */

/* Helper function to prevent dead code elimination */
__attribute__((noinline))
int checksum_512i(__m512i v) {
    int sum = 0;
    alignas(64) int32_t arr[16];
    _mm512_store_si512(arr, v);
    for (int i = 0; i < 16; i++) {
        sum += arr[i];
    }
    return sum;
}

__attribute__((noinline))
double checksum_512d(__m512d v) {
    double sum = 0.0;
    alignas(64) double arr[8];
    _mm512_store_pd(arr, v);
    for (int i = 0; i < 8; i++) {
        sum += arr[i];
    }
    return sum;
}

__attribute__((noinline))
float checksum_512(__m512 v) {
    float sum = 0.0f;
    alignas(64) float arr[16];
    _mm512_store_ps(arr, v);
    for (int i = 0; i < 16; i++) {
        sum += arr[i];
    }
    return sum;
}

#ifdef __AVX512BW__
__attribute__((noinline))
int64_t checksum_512i_64qi(__m512i v) {
    int64_t sum = 0;
    alignas(64) int8_t arr[64];
    _mm512_store_si512(arr, v);
    for (int i = 0; i < 64; i++) {
        sum += arr[i];
    }
    return sum;
}
#endif

int main() {
    int total_checksum = 0;
    
#ifdef __AVX512F__
    printf("Testing AVX-512F vector blends...\n");
    
    /* Test V16SFmode */
    {
        __m512 a = _mm512_setr_ps(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                                  9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f);
        __m512 b = _mm512_setr_ps(100.0f, 200.0f, 300.0f, 400.0f, 500.0f, 600.0f, 700.0f, 800.0f,
                                  900.0f, 1000.0f, 1100.0f, 1200.0f, 1300.0f, 1400.0f, 1500.0f, 1600.0f);
        __mmask16 mask = 0xAAAA; /* 1010101010101010 pattern */
        __m512 result = test_v16sf_blend(a, b, mask);
        float sum = checksum_512(result);
        total_checksum += (int)sum;
        printf("  V16SFmode checksum: %f\n", sum);
    }
    
    /* Test V8DFmode */
    {
        __m512d a = _mm512_setr_pd(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
        __m512d b = _mm512_setr_pd(100.0, 200.0, 300.0, 400.0, 500.0, 600.0, 700.0, 800.0);
        __mmask8 mask = 0xAA; /* 10101010 pattern */
        __m512d result = test_v8df_blend(a, b, mask);
        double sum = checksum_512d(result);
        total_checksum += (int)sum;
        printf("  V8DFmode checksum: %f\n", sum);
    }
    
    /* Test V16SImode */
    {
        __m512i a = _mm512_setr_epi32(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
        __m512i b = _mm512_setr_epi32(100, 200, 300, 400, 500, 600, 700, 800, 
                                      900, 1000, 1100, 1200, 1300, 1400, 1500, 1600);
        __mmask16 mask = 0x5555; /* 0101010101010101 pattern */
        __m512i result = test_v16si_blend(a, b, mask);
        int sum = checksum_512i(result);
        total_checksum += sum;
        printf("  V16SImode checksum: %d\n", sum);
    }
    
    /* Test V8DImode */
    {
        __m512i a = _mm512_setr_epi64(1, 2, 3, 4, 5, 6, 7, 8);
        __m512i b = _mm512_setr_epi64(100, 200, 300, 400, 500, 600, 700, 800);
        __mmask8 mask = 0x55; /* 01010101 pattern */
        __m512i result = test_v8di_blend(a, b, mask);
        int sum = checksum_512i(result);
        total_checksum += sum;
        printf("  V8DImode checksum: %d\n", sum);
    }
#endif /* __AVX512F__ */
    
#ifdef __AVX512BW__
    printf("\nTesting AVX-512BW vector blends...\n");
    
    /* Test V64QImode */
    {
        alignas(64) int8_t a_arr[64];
        alignas(64) int8_t b_arr[64];
        for (int i = 0; i < 64; i++) {
            a_arr[i] = i;
            b_arr[i] = i + 100;
        }
        __m512i a = _mm512_load_si512(a_arr);
        __m512i b = _mm512_load_si512(b_arr);
        __mmask64 mask = 0xAAAAAAAAAAAAAAAA; /* Alternating pattern */
        __m512i result = test_v64qi_blend(a, b, mask);
        int64_t sum = checksum_512i_64qi(result);
        total_checksum += (int)sum;
        printf("  V64QImode checksum: %ld\n", sum);
    }
    
    /* Test V32HImode */
    {
        alignas(64) int16_t a_arr[32];
        alignas(64) int16_t b_arr[32];
        for (int i = 0; i < 32; i++) {
            a_arr[i] = i;
            b_arr[i] = i + 1000;
        }
        __m512i a = _mm512_load_si512(a_arr);
        __m512i b = _mm512_load_si512(b_arr);
        __mmask32 mask = 0xAAAAAAAA; /* Alternating pattern */
        __m512i result = test_v32hi_blend(a, b, mask);
        int sum = checksum_512i(result);
        total_checksum += sum;
        printf("  V32HImode checksum: %d\n", sum);
    }
#endif /* __AVX512BW__ */
    
#ifdef __AVX512FP16__
    printf("\nTesting AVX-512FP16 vector blends...\n");
    
    /* Test V32HFmode */
    {
        alignas(64) _Float16 a_arr[32];
        alignas(64) _Float16 b_arr[32];
        for (int i = 0; i < 32; i++) {
            a_arr[i] = (_Float16)i;
            b_arr[i] = (_Float16)(i + 100);
        }
        __m512h a = _mm512_load_ph(a_arr);
        __m512h b = _mm512_load_ph(b_arr);
        __mmask32 mask = 0x55555555; /* Alternating pattern */
        __m512h result = test_v32hf_blend(a, b, mask);
        
        /* Compute checksum */
        _Float16 sum = 0.0f16;
        alignas(64) _Float16 result_arr[32];
        _mm512_store_ph(result_arr, result);
        for (int i = 0; i < 32; i++) {
            sum += result_arr[i];
        }
        total_checksum += (int)sum;
        printf("  V32HFmode checksum: %f\n", (float)sum);
    }
    
    /* Test V32BFmode */
    {
        alignas(64) __bf16 a_arr[32];
        alignas(64) __bf16 b_arr[32];
        for (int i = 0; i < 32; i++) {
            a_arr[i] = (__bf16)(i * 1.5f);
            b_arr[i] = (__bf16)((i + 100) * 1.5f);
        }
        __m512bh a = _mm512_load_si512((__m512i*)a_arr);
        __m512bh b = _mm512_load_si512((__m512i*)b_arr);
        __mmask32 mask = 0xAAAAAAAA; /* Alternating pattern */
        __m512bh result = test_v32bf_blend(a, b, mask);
        
        /* Compute checksum */
        float sum = 0.0f;
        alignas(64) __bf16 result_arr[32];
        _mm512_store_si512((__m512i*)result_arr, (__m512i)result);
        for (int i = 0; i < 32; i++) {
            sum += (float)result_arr[i];
        }
        total_checksum += (int)sum;
        printf("  V32BFmode checksum: %f\n", sum);
    }
#endif /* __AVX512FP16__ */
    
    printf("\nTotal checksum: %d\n", total_checksum);
    
    /* Use the result to prevent dead code elimination */
    if (total_checksum > 0) {
        return 0;
    } else {
        return 1;
    }
}
