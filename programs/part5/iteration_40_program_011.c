#include <immintrin.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

// Helper function to print results for debugging
void print_hex(const void* data, size_t size) {
    const unsigned char* bytes = (const unsigned char*)data;
    for (size_t i = 0; i < size; i++) {
        printf("%02x", bytes[i]);
        if ((i + 1) % 16 == 0) printf("\n");
        else if ((i + 1) % 8 == 0) printf(" ");
    }
    printf("\n");
}

#ifdef __AVX512BW__
// V64QImode: 64 x 8-bit integers
__m512i test_v64qimode_blend() {
    // Initialize arrays with distinct patterns
    uint8_t src1[64], src2[64];
    for (int i = 0; i < 64; i++) {
        src1[i] = i;           // 0, 1, 2, ...
        src2[i] = 255 - i;     // 255, 254, 253, ...
    }
    
    __m512i a = _mm512_loadu_si512(src1);
    __m512i b = _mm512_loadu_si512(src2);
    
    // Create alternating mask: 0xAA...AA (alternating 1/0 pattern)
    __mmask64 mask = 0xAAAAAAAAAAAAAAAA;
    
    // Blend using vblendmb
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Force usage by storing
    uint8_t out[64];
    _mm512_storeu_si512(out, result);
    
    // Compute checksum to prevent optimization
    uint64_t sum = 0;
    for (int i = 0; i < 64; i++) sum += out[i];
    
    // Return something dependent on the blend
    return _mm512_set1_epi8(sum & 0xFF);
}

// V32HImode: 32 x 16-bit integers
__m512i test_v32himode_blend() {
    int16_t src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 100;
        src2[i] = -i * 100;
    }
    
    __m512i a = _mm512_loadu_si512(src1);
    __m512i b = _mm512_loadu_si512(src2);
    
    // Create checkerboard mask
    __mmask32 mask = 0x55555555;  // alternating 0/1
    
    // Blend using vblendmw
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    // Use result in computation
    int16_t out[32];
    _mm512_storeu_si512(out, result);
    
    int32_t sum = 0;
    for (int i = 0; i < 32; i++) sum += out[i];
    
    return _mm512_set1_epi16(sum & 0xFFFF);
}
#endif // __AVX512BW__

#ifdef __AVX512FP16__
// V32HFmode: 32 x half-precision floats
__m512h test_v32hfmode_blend() {
    _Float16 src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 1.5f;
        src2[i] = i * 2.5f;
    }
    
    __m512h a = _mm512_loadu_ph(src1);
    __m512h b = _mm512_loadu_ph(src2);
    
    // Compare to create dynamic mask
    __m512h threshold = _mm512_set1_ph(20.0f);
    __mmask32 mask = _mm512_cmp_ph_mask(a, threshold, _CMP_LT_OQ);
    
    // Blend using vblendmps for half-precision
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    // Store and compute checksum
    _Float16 out[32];
    _mm512_storeu_ph(out, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) sum += out[i];
    
    return _mm512_set1_ph(sum / 32.0f);
}

// V32BFmode: 32 x brain float (bfloat16)
__m512bh test_v32bfmode_blend() {
    // Note: BF16 support varies; using __m512i as container
    // and casting to appropriate type
    uint16_t src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        // Simple bfloat16-like patterns
        src1[i] = i << 8;      // Increment exponent part
        src2[i] = (31 - i) << 8;
    }
    
    __m512i a = _mm512_loadu_si512(src1);
    __m512i b = _mm512_loadu_si512(src2);
    
    // Create mask based on comparison of integer representation
    __mmask32 mask = 0xAAAAAAAA;  // alternating pattern
    
    // For BF16, we might need to use integer blend and cast
    // This should still trigger the blend expansion
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    // Convert to BF16 vector type if supported
    __m512bh result_bf = _mm512_castsi512_ph(result);
    
    // Store and verify
    uint16_t out[32];
    _mm512_storeu_si512(out, result);
    
    uint32_t sum = 0;
    for (int i = 0; i < 32; i++) sum += out[i];
    
    return _mm512_castsi512_ph(_mm512_set1_epi16(sum & 0xFFFF));
}
#endif // __AVX512FP16__

#ifdef __AVX512F__
// V16SImode: 16 x 32-bit integers
__m512i test_v16simode_blend() {
    int32_t src1[16], src2[16];
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1000;
        src2[i] = i * 2000;
    }
    
    __m512i a = _mm512_loadu_si512(src1);
    __m512i b = _mm512_loadu_si512(src2);
    
    // Create mask using comparison
    __m512i threshold = _mm512_set1_epi32(8000);
    __mmask16 mask = _mm512_cmp_epi32_mask(a, threshold, _CMP_LT_OQ);
    
    // Blend using vblendmd
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    int32_t out[16];
    _mm512_storeu_si512(out, result);
    
    int64_t sum = 0;
    for (int i = 0; i < 16; i++) sum += out[i];
    
    return _mm512_set1_epi32(sum & 0xFFFFFFFF);
}

// V8DImode: 8 x 64-bit integers
__m512i test_v8dimode_blend() {
    int64_t src1[8], src2[8];
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 10000LL;
        src2[i] = i * 30000LL;
    }
    
    __m512i a = _mm512_loadu_si512(src1);
    __m512i b = _mm512_loadu_si512(src2);
    
    // Create alternating mask
    __mmask8 mask = 0xAA;  // 0b10101010
    
    // Blend using vblendmq
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    int64_t out[8];
    _mm512_storeu_si512(out, result);
    
    int64_t sum = 0;
    for (int i = 0; i < 8; i++) sum += out[i];
    
    return _mm512_set1_epi64(sum);
}

// V8DFmode: 8 x double-precision floats
__m512d test_v8dfmode_blend() {
    double src1[8], src2[8];
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.1;
        src2[i] = i * 2.2;
    }
    
    __m512d a = _mm512_loadu_pd(src1);
    __m512d b = _mm512_loadu_pd(src2);
    
    // Create mask using floating comparison
    __m512d threshold = _mm512_set1_pd(4.0);
    __mmask8 mask = _mm512_cmp_pd_mask(a, threshold, _CMP_LT_OQ);
    
    // Blend using vblendmpd
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    double out[8];
    _mm512_storeu_pd(out, result);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) sum += out[i];
    
    return _mm512_set1_pd(sum / 8.0);
}

// V16SFmode: 16 x single-precision floats
__m512 test_v16sfmode_blend() {
    float src1[16], src2[16];
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.5f;
        src2[i] = i * 1.5f;
    }
    
    __m512 a = _mm512_loadu_ps(src1);
    __m512 b = _mm512_loadu_ps(src2);
    
    // Create complex mask pattern
    __m512 threshold = _mm512_set1_ps(4.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(a, threshold, _CMP_LT_OQ);
    
    // Blend using vblendmps
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    float out[16];
    _mm512_storeu_ps(out, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) sum += out[i];
    
    return _mm512_set1_ps(sum / 16.0f);
}
#endif // __AVX512F__

int main() {
    printf("Testing AVX-512 blend operations...\n");
    
    // Force compiler to generate all blend variants
    // by calling functions and using results
    
#ifdef __AVX512BW__
    {
        __m512i r1 = test_v64qimode_blend();
        __m512i r2 = test_v32himode_blend();
        uint8_t temp1[64], temp2[64];
        _mm512_storeu_si512(temp1, r1);
        _mm512_storeu_si512(temp2, r2);
        printf("V64QI and V32HI blends completed\n");
    }
#endif
    
#ifdef __AVX512FP16__
    {
        __m512h r3 = test_v32hfmode_blend();
        __m512bh r4 = test_v32bfmode_blend();
        _Float16 temp3[32];
        uint16_t temp4[32];
        _mm512_storeu_ph(temp3, r3);
        _mm512_storeu_si512(temp4, _mm512_castph_si512(r4));
        printf("V32HF and V32BF blends completed\n");
    }
#endif
    
#ifdef __AVX512F__
    {
        __m512i r5 = test_v16simode_blend();
        __m512i r6 = test_v8dimode_blend();
        __m512d r7 = test_v8dfmode_blend();
        __m512 r8 = test_v16sfmode_blend();
        
        int32_t temp5[16];
        int64_t temp6[8];
        double temp7[8];
        float temp8[16];
        
        _mm512_storeu_si512(temp5, r5);
        _mm512_storeu_si512(temp6, r6);
        _mm512_storeu_pd(temp7, r7);
        _mm512_storeu_ps(temp8, r8);
        
        printf("V16SI, V8DI, V8DF, V16SF blends completed\n");
    }
#endif
    
    printf("All blend tests completed successfully!\n");
    return 0;
}
