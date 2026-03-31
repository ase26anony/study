/* AVX-512 Blend Coverage Test Program
 * Compile with: gcc -O3 -mavx512f -mavx512bw -mavx512fp16 -march=native -o avx512_blend_test avx512_blend_test.c
 */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifdef __AVX512F__

/* Helper function to print results for debugging */
void print_hex(const void* data, size_t size) {
    const unsigned char* p = (const unsigned char*)data;
    for (size_t i = 0; i < size; i++) {
        printf("%02x", p[i]);
        if ((i + 1) % 16 == 0) printf("\n");
        else if ((i + 1) % 8 == 0) printf(" ");
    }
    printf("\n");
}

/* V64QImode: 64 x 8-bit integers */
#ifdef __AVX512BW__
__m512i test_v64qimode_blend() {
    /* Create two vectors with distinct patterns */
    __m512i a = _mm512_set1_epi8(0xAA);  /* 10101010 */
    __m512i b = _mm512_set1_epi8(0x55);  /* 01010101 */
    
    /* Create alternating mask: 0x5555... (01010101 pattern) */
    __mmask64 mask = 0x5555555555555555ULL;
    
    /* This should generate vblendmb */
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    /* Force usage to prevent optimization */
    volatile __m512i volatile_result = result;
    return volatile_result;
}
#endif

/* V32HImode: 32 x 16-bit integers */
#ifdef __AVX512BW__
__m512i test_v32himode_blend() {
    __m512i a = _mm512_set1_epi16(0xAAAA);
    __m512i b = _mm512_set1_epi16(0x5555);
    
    /* Alternating mask pattern */
    __mmask32 mask = 0x55555555;
    
    /* This should generate vblendmw */
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    volatile __m512i volatile_result = result;
    return volatile_result;
}
#endif

/* V32HFmode: 32 x half-precision floats */
#ifdef __AVX512FP16__
__m512h test_v32hfmode_blend() {
    /* Use _Float16 type for half precision */
    _Float16 a_vals[32], b_vals[32];
    for (int i = 0; i < 32; i++) {
        a_vals[i] = (_Float16)(i * 1.0f);
        b_vals[i] = (_Float16)(i * 2.0f);
    }
    
    __m512h a = _mm512_loadu_ph(a_vals);
    __m512h b = _mm512_loadu_ph(b_vals);
    
    /* Create mask where even elements select from b, odd from a */
    __mmask32 mask = 0xAAAAAAAA;  /* 10101010... pattern */
    
    /* This should generate vblendmps for half-precision */
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    volatile __m512h volatile_result = result;
    return volatile_result;
}
#endif

/* V32BFmode: 32 x brain float (bfloat16) */
#ifdef __AVX512BF16__
__m512bh test_v32bfmode_blend() {
    /* Initialize bfloat16 arrays */
    __m512bh a = _mm512_set1_epi16(0x3F80);  /* bfloat16 1.0 */
    __m512bh b = _mm512_set1_epi16(0x4000);  /* bfloat16 2.0 */
    
    /* Alternating mask */
    __mmask32 mask = 0xAAAAAAAA;
    
    /* Use integer blend for bfloat16 (same as V32HImode for blending) */
    __m512bh result = _mm512_mask_blend_epi16(mask, a, b);
    
    volatile __m512bh volatile_result = result;
    return volatile_result;
}
#endif

/* V16SImode: 16 x 32-bit integers */
__m512i test_v16simode_blend() {
    __m512i a = _mm512_set1_epi32(0xAAAAAAAA);
    __m512i b = _mm512_set1_epi32(0x55555555);
    
    /* Mask with alternating pattern */
    __mmask16 mask = 0xAAAA;  /* 1010101010101010 */
    
    /* This should generate vblendmd */
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    volatile __m512i volatile_result = result;
    return volatile_result;
}

/* V8DImode: 8 x 64-bit integers */
__m512i test_v8dimode_blend() {
    __m512i a = _mm512_set1_epi64(0xAAAAAAAAAAAAAAAAULL);
    __m512i b = _mm512_set1_epi64(0x5555555555555555ULL);
    
    /* Alternating mask for 8 elements */
    __mmask8 mask = 0xAA;  /* 10101010 */
    
    /* This should generate vblendmq */
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    volatile __m512i volatile_result = result;
    return volatile_result;
}

/* V8DFmode: 8 x double-precision floats */
__m512d test_v8dfmode_blend() {
    __m512d a = _mm512_set1_pd(1.0);
    __m512d b = _mm512_set1_pd(2.0);
    
    /* Mask selecting even elements from b, odd from a */
    __mmask8 mask = 0xAA;  /* 10101010 */
    
    /* This should generate vblendmpd */
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    volatile __m512d volatile_result = result;
    return volatile_result;
}

/* V16SFmode: 16 x single-precision floats */
__m512 test_v16sfmode_blend() {
    __m512 a = _mm512_set1_ps(1.0f);
    __m512 b = _mm512_set1_ps(2.0f);
    
    /* Alternating mask */
    __mmask16 mask = 0xAAAA;  /* 1010101010101010 */
    
    /* This should generate vblendmps */
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    volatile __m512 volatile_result = result;
    return volatile_result;
}

/* Array-based test to ensure blends aren't optimized away */
#ifdef __AVX512BW__
void test_array_v64qimode(uint8_t* out, const uint8_t* a, const uint8_t* b, size_t size) {
    /* Process in chunks of 64 bytes */
    for (size_t i = 0; i < size; i += 64) {
        __m512i va = _mm512_loadu_si512((const __m512i*)(a + i));
        __m512i vb = _mm512_loadu_si512((const __m512i*)(b + i));
        
        /* Create dynamic mask based on data */
        __mmask64 mask = _mm512_cmpeq_epi8_mask(va, _mm512_set1_epi8(0));
        
        /* Force blend generation */
        __m512i result = _mm512_mask_blend_epi8(mask, va, vb);
        
        _mm512_storeu_si512((__m512i*)(out + i), result);
    }
}
#endif

int main() {
    printf("Testing AVX-512 blend operations...\n");
    
    /* Test each vector mode */
#ifdef __AVX512F__
    printf("Testing V16SImode (32-bit integers)...\n");
    __m512i v16si = test_v16simode_blend();
    
    printf("Testing V8DImode (64-bit integers)...\n");
    __m512i v8di = test_v8dimode_blend();
    
    printf("Testing V8DFmode (double-precision floats)...\n");
    __m512d v8df = test_v8dfmode_blend();
    
    printf("Testing V16SFmode (single-precision floats)...\n");
    __m512 v16sf = test_v16sfmode_blend();
#endif
    
#ifdef __AVX512BW__
    printf("Testing V64QImode (8-bit integers)...\n");
    __m512i v64qi = test_v64qimode_blend();
    
    printf("Testing V32HImode (16-bit integers)...\n");
    __m512i v32hi = test_v32himode_blend();
    
    /* Array test to ensure runtime execution */
    const size_t ARRAY_SIZE = 1024;
    uint8_t array_a[ARRAY_SIZE];
    uint8_t array_b[ARRAY_SIZE];
    uint8_t array_out[ARRAY_SIZE];
    
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        array_a[i] = (uint8_t)(i % 256);
        array_b[i] = (uint8_t)(255 - (i % 256));
    }
    
    test_array_v64qimode(array_out, array_a, array_b, ARRAY_SIZE);
    
    /* Verify some results */
    uint32_t checksum = 0;
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        checksum += array_out[i];
    }
    printf("V64QImode array checksum: %u\n", checksum);
#endif
    
#ifdef __AVX512FP16__
    printf("Testing V32HFmode (half-precision floats)...\n");
    #ifdef __AVX512FP16__
    __m512h v32hf = test_v32hfmode_blend();
    #endif
#endif
    
#ifdef __AVX512BF16__
    printf("Testing V32BFmode (bfloat16)...\n");
    #ifdef __AVX512BF16__
    __m512bh v32bf = test_v32bfmode_blend();
    #endif
#endif
    
    printf("All blend tests completed.\n");
    
    /* Return non-zero if any required feature was missing */
    #ifndef __AVX512F__
    printf("WARNING: AVX-512F not supported\n");
    return 1;
    #endif
    
    return 0;
}
