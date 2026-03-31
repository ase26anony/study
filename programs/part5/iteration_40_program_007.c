#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

// Helper function to print results (for debugging)
void print_hex(const void* data, size_t size) {
    const unsigned char* bytes = (const unsigned char*)data;
    for (size_t i = 0; i < size; i++) {
        printf("%02x", bytes[i]);
        if ((i + 1) % 16 == 0) printf("\n");
        else if ((i + 1) % 2 == 0) printf(" ");
    }
    printf("\n");
}

// ==================== V64QImode (64x 8-bit integers) ====================
#ifdef __AVX512BW__
__m512i test_v64qimode_blend() {
    // Create two vectors with distinct patterns
    __m512i a = _mm512_set1_epi8(0xAA);  // 10101010 pattern
    __m512i b = _mm512_set1_epi8(0x55);  // 01010101 pattern
    
    // Create a mask: alternating true/false
    __mmask64 mask = 0xAAAAAAAAAAAAAAAA;  // Alternating bits
    
    // This should generate vblendmb
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Force usage by returning
    return result;
}
#endif

// ==================== V32HImode (32x 16-bit integers) ====================
#ifdef __AVX512BW__
__m512i test_v32himode_blend() {
    __m512i a = _mm512_set1_epi16(0xAAAA);
    __m512i b = _mm512_set1_epi16(0x5555);
    
    // Create alternating mask for 32 elements
    __mmask32 mask = 0xAAAAAAAA;  // Alternating bits
    
    // This should generate vblendmw
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    return result;
}
#endif

// ==================== V32HFmode (32x half-precision floats) ====================
#ifdef __AVX512FP16__
__m512h test_v32hfmode_blend() {
    // Initialize with distinct half-precision values
    _Float16 a_val = 1.5f;
    _Float16 b_val = 2.5f;
    __m512h a = _mm512_set1_ph(a_val);
    __m512h b = _mm512_set1_ph(b_val);
    
    // Create mask using comparison
    __m512h cmp_a = _mm512_set1_ph(2.0f);
    __m512h cmp_b = _mm512_set1_ph(1.0f);
    
    // Compare for greater than
    __mmask32 mask = _mm512_cmp_ph_mask(cmp_a, cmp_b, _CMP_GT_OQ);
    
    // This should generate vblendmps for half-precision
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    return result;
}
#endif

// ==================== V32BFmode (32x brain float) ====================
#ifdef __AVX512BF16__
__m512bh test_v32bfmode_blend() {
    // Initialize bfloat16 vectors
    __m512bh a = _mm512_set1_epi16(0x3F80);  // bfloat16 1.0
    __m512bh b = _mm512_set1_epi16(0x4000);  // bfloat16 2.0
    
    // Create alternating mask
    __mmask32 mask = 0x55555555;  // Every other element
    
    // Use generic blend intrinsic that works with bfloat16
    // Note: There's no direct _mm512_mask_blend_* for BF16 in current intrinsics,
    // but the compiler should still generate the appropriate blend
    __m512bh result = _mm512_mask_blend_epi16(mask, 
        (__m512i)a, (__m512i)b);
    
    return result;
}
#endif

// ==================== V16SImode (16x 32-bit integers) ====================
#ifdef __AVX512F__
__m512i test_v16simode_blend() {
    __m512i a = _mm512_set1_epi32(0xAAAAAAAA);
    __m512i b = _mm512_set1_epi32(0x55555555);
    
    // Create mask using comparison
    __m512i cmp_a = _mm512_set1_epi32(10);
    __m512i cmp_b = _mm512_set1_epi32(5);
    
    __mmask16 mask = _mm512_cmp_epi32_mask(cmp_a, cmp_b, _MM_CMPINT_GT);
    
    // This should generate vblendmd
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    return result;
}
#endif

// ==================== V8DImode (8x 64-bit integers) ====================
#ifdef __AVX512F__
__m512i test_v8dimode_blend() {
    __m512i a = _mm512_set1_epi64(0xAAAAAAAAAAAAAAAA);
    __m512i b = _mm512_set1_epi64(0x5555555555555555);
    
    // Create mask
    __m512i cmp_a = _mm512_set1_epi64(100);
    __m512i cmp_b = _mm512_set1_epi64(50);
    
    __mmask8 mask = _mm512_cmp_epi64_mask(cmp_a, cmp_b, _MM_CMPINT_GT);
    
    // This should generate vblendmq
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    return result;
}
#endif

// ==================== V8DFmode (8x double-precision floats) ====================
#ifdef __AVX512F__
__m512d test_v8dfmode_blend() {
    __m512d a = _mm512_set1_pd(1.0);
    __m512d b = _mm512_set1_pd(2.0);
    
    // Create mask using comparison
    __m512d cmp_a = _mm512_set1_pd(1.5);
    __m512d cmp_b = _mm512_set1_pd(1.0);
    
    __mmask8 mask = _mm512_cmp_pd_mask(cmp_a, cmp_b, _CMP_GT_OQ);
    
    // This should generate vblendmpd
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    return result;
}
#endif

// ==================== V16SFmode (16x single-precision floats) ====================
#ifdef __AVX512F__
__m512 test_v16sfmode_blend() {
    __m512 a = _mm512_set1_ps(1.0f);
    __m512 b = _mm512_set1_ps(2.0f);
    
    // Create mask using comparison
    __m512 cmp_a = _mm512_set1_ps(1.5f);
    __m512 cmp_b = _mm512_set1_ps(1.0f);
    
    __mmask16 mask = _mm512_cmp_ps_mask(cmp_a, cmp_b, _CMP_GT_OQ);
    
    // This should generate vblendmps
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    return result;
}
#endif

// ==================== Main test driver ====================
int main() {
    int checksum = 0;
    
    printf("Testing AVX-512 blend operations...\n");
    
#ifdef __AVX512BW__
    {
        __m512i result = test_v64qimode_blend();
        // Add first few bytes to checksum
        uint8_t* bytes = (uint8_t*)&result;
        for (int i = 0; i < 8; i++) checksum += bytes[i];
        printf("V64QImode blend tested\n");
    }
    
    {
        __m512i result = test_v32himode_blend();
        uint16_t* words = (uint16_t*)&result;
        for (int i = 0; i < 4; i++) checksum += words[i];
        printf("V32HImode blend tested\n");
    }
#endif
    
#ifdef __AVX512FP16__
    {
        __m512h result = test_v32hfmode_blend();
        _Float16* halves = (_Float16*)&result;
        // Use integer representation for checksum
        for (int i = 0; i < 4; i++) checksum += *(uint16_t*)&halves[i];
        printf("V32HFmode blend tested\n");
    }
#endif
    
#ifdef __AVX512BF16__
    {
        __m512bh result = test_v32bfmode_blend();
        uint16_t* bf16 = (uint16_t*)&result;
        for (int i = 0; i < 4; i++) checksum += bf16[i];
        printf("V32BFmode blend tested\n");
    }
#endif
    
#ifdef __AVX512F__
    {
        __m512i result = test_v16simode_blend();
        uint32_t* dwords = (uint32_t*)&result;
        for (int i = 0; i < 4; i++) checksum += dwords[i];
        printf("V16SImode blend tested\n");
    }
    
    {
        __m512i result = test_v8dimode_blend();
        uint64_t* qwords = (uint64_t*)&result;
        for (int i = 0; i < 2; i++) checksum += (int)qwords[i];
        printf("V8DImode blend tested\n");
    }
    
    {
        __m512d result = test_v8dfmode_blend();
        double* doubles = (double*)&result;
        for (int i = 0; i < 2; i++) checksum += (int)doubles[i];
        printf("V8DFmode blend tested\n");
    }
    
    {
        __m512 result = test_v16sfmode_blend();
        float* floats = (float*)&result;
        for (int i = 0; i < 4; i++) checksum += (int)floats[i];
        printf("V16SFmode blend tested\n");
    }
#endif
    
    printf("Final checksum: %d\n", checksum);
    printf("All applicable AVX-512 blend operations tested.\n");
    
    // Return non-zero if no tests were run
#ifdef __AVX512F__
    return 0;
#else
    printf("Warning: AVX-512 not supported by compiler\n");
    return 1;
#endif
}
