#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

// Helper function to print results for debugging
void print_hex(const void* data, size_t size) {
    const unsigned char* bytes = (const unsigned char*)data;
    for (size_t i = 0; i < size; i++) {
        printf("%02x", bytes[i]);
        if ((i + 1) % 16 == 0) printf("\n");
        else if ((i + 1) % 4 == 0) printf(" ");
    }
    printf("\n");
}

// ==================== V64QImode (64x 8-bit integers) ====================
#ifdef __AVX512BW__
__m512i test_v64qimode_blend() {
    // Initialize arrays with distinct patterns
    int8_t src1[64], src2[64];
    for (int i = 0; i < 64; i++) {
        src1[i] = i;
        src2[i] = 64 + i;
    }
    
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    // Create alternating mask: 0xAAAA... (10101010 pattern)
    __mmask64 mask = _mm512_int2mask(0xAAAAAAAAAAAAAAAA);
    
    // Blend using vblendmb instruction
    __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
    
    // Force usage by storing
    int8_t out[64];
    _mm512_storeu_si512((__m512i*)out, result);
    
    // Compute checksum to prevent optimization
    int64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += out[i];
    }
    
    // Return result that depends on blend
    return _mm512_add_epi64(result, _mm512_set1_epi64(sum));
}
#endif

// ==================== V32HImode (32x 16-bit integers) ====================
#ifdef __AVX512BW__
__m512i test_v32himode_blend() {
    int16_t src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 100;
        src2[i] = 3200 + i * 200;
    }
    
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    // Create checkerboard mask: 0x5555... (01010101 pattern)
    __mmask32 mask = _mm512_int2mask(0x55555555);
    
    // Blend using vblendmw instruction
    __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
    
    // Process result to ensure it's not optimized away
    int16_t out[32];
    _mm512_storeu_si512((__m512i*)out, result);
    
    int32_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += out[i];
    }
    
    return _mm512_add_epi32(result, _mm512_set1_epi32(sum));
}
#endif

// ==================== V32HFmode (32x half-precision floats) ====================
#ifdef __AVX512FP16__
__m512h test_v32hfmode_blend() {
    _Float16 src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 0.5f);
        src2[i] = (_Float16)(16.0f + i * 0.25f);
    }
    
    __m512h v1 = _mm512_loadu_ph(src1);
    __m512h v2 = _mm512_loadu_ph(src2);
    
    // Create mask with alternating groups of 4
    __mmask32 mask = _mm512_int2mask(0xF0F0F0F0);
    
    // Blend using vblendmps for half-precision
    __m512h result = _mm512_mask_blend_ph(mask, v1, v2);
    
    // Store and compute checksum
    _Float16 out[32];
    _mm512_storeu_ph(out, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)out[i];
    }
    
    // Add sum to all elements to prevent optimization
    return _mm512_add_ph(result, _mm512_set1_ph((_Float16)sum));
}
#endif

// ==================== V32BFmode (32x brain floats) ====================
#ifdef __AVX512BF16__
__m512bh test_v32bfmode_blend() {
    // Note: __bfloat16 is available with AVX512_BF16
    __bfloat16 src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = bfloat16_from_float(i * 1.0f);
        src2[i] = bfloat16_from_float(32.0f + i * 2.0f);
    }
    
    __m512bh v1 = _mm512_loadu_bf16(src1);
    __m512bh v2 = _mm512_loadu_bf16(src2);
    
    // Create complex mask pattern
    __mmask32 mask = _mm512_int2mask(0x33333333);
    
    // Blend BF16 values - may use vblendmps under the hood
    // Note: There's no direct _mm512_mask_blend_* for BF16 in current intrinsics,
    // so we use a comparison to generate the blend
    __mmask32 cmp_mask = _mm512_cmp_epi16_mask(
        _mm512_castsi512_si512(_mm512_loadu_si512((__m512i*)src1)),
        _mm512_castsi512_si512(_mm512_loadu_si512((__m512i*)src2)),
        _MM_CMPINT_LT
    );
    
    __m512bh result = _mm512_mask_blend_epi16(cmp_mask, 
        _mm512_castsi512_si512(v1), 
        _mm512_castsi512_si512(v2));
    
    // Store result
    __bfloat16 out[32];
    _mm512_storeu_bf16(out, result);
    
    return result;
}
#endif

// ==================== V16SImode (16x 32-bit integers) ====================
#ifdef __AVX512F__
__m512i test_v16simode_blend() {
    int32_t src1[16], src2[16];
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1000;
        src2[i] = 16000 + i * 2000;
    }
    
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    // Create mask from comparison
    __mmask16 mask = _mm512_cmplt_epi32_mask(v1, _mm512_set1_epi32(8000));
    
    // Blend using vblendmd instruction
    __m512i result = _mm512_mask_blend_epi32(mask, v1, v2);
    
    // Process result
    int32_t out[16];
    _mm512_storeu_si512((__m512i*)out, result);
    
    int64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += out[i];
    }
    
    return _mm512_add_epi64(result, _mm512_set1_epi64(sum));
}
#endif

// ==================== V8DImode (8x 64-bit integers) ====================
#ifdef __AVX512F__
__m512i test_v8dimode_blend() {
    int64_t src1[8], src2[8];
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 10000LL;
        src2[i] = 80000LL + i * 20000LL;
    }
    
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    // Create mask: select elements where index is even
    __mmask8 mask = _mm512_int2mask(0xAA);  // 10101010
    
    // Blend using vblendmq instruction
    __m512i result = _mm512_mask_blend_epi64(mask, v1, v2);
    
    // Process result
    int64_t out[8];
    _mm512_storeu_si512((__m512i*)out, result);
    
    int64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += out[i];
    }
    
    return _mm512_add_epi64(result, _mm512_set1_epi64(sum));
}
#endif

// ==================== V8DFmode (8x double-precision floats) ====================
#ifdef __AVX512F__
__m512d test_v8dfmode_blend() {
    double src1[8], src2[8];
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.5;
        src2[i] = 12.0 + i * 2.5;
    }
    
    __m512d v1 = _mm512_loadu_pd(src1);
    __m512d v2 = _mm512_loadu_pd(src2);
    
    // Create mask from comparison
    __mmask8 mask = _mm512_cmp_pd_mask(v1, _mm512_set1_pd(6.0), _CMP_LT_OQ);
    
    // Blend using vblendmpd instruction
    __m512d result = _mm512_mask_blend_pd(mask, v1, v2);
    
    // Process result
    double out[8];
    _mm512_storeu_pd(out, result);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += out[i];
    }
    
    return _mm512_add_pd(result, _mm512_set1_pd(sum));
}
#endif

// ==================== V16SFmode (16x single-precision floats) ====================
#ifdef __AVX512F__
__m512 test_v16sfmode_blend() {
    float src1[16], src2[16];
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.75f;
        src2[i] = 12.0f + i * 1.25f;
    }
    
    __m512 v1 = _mm512_loadu_ps(src1);
    __m512 v2 = _mm512_loadu_ps(src2);
    
    // Create complex mask pattern
    __mmask16 mask = _mm512_cmp_ps_mask(v1, _mm512_set1_ps(6.0f), _CMP_LT_OQ);
    
    // Blend using vblendmps instruction
    __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
    
    // Process result
    float out[16];
    _mm512_storeu_ps(out, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += out[i];
    }
    
    return _mm512_add_ps(result, _mm512_set1_ps(sum));
}
#endif

// ==================== Main driver ====================
int main() {
    int checksum = 0;
    
    printf("Testing AVX-512 blend operations...\n");
    
#ifdef __AVX512BW__
    printf("Testing V64QImode (64x int8)...\n");
    __m512i r1 = test_v64qimode_blend();
    int8_t temp1[64];
    _mm512_storeu_si512((__m512i*)temp1, r1);
    for (int i = 0; i < 64; i++) checksum += temp1[i];
    
    printf("Testing V32HImode (32x int16)...\n");
    __m512i r2 = test_v32himode_blend();
    int16_t temp2[32];
    _mm512_storeu_si512((__m512i*)temp2, r2);
    for (int i = 0; i < 32; i++) checksum += temp2[i];
#endif

#ifdef __AVX512FP16__
    printf("Testing V32HFmode (32x half-precision)...\n");
    __m512h r3 = test_v32hfmode_blend();
    _Float16 temp3[32];
    _mm512_storeu_ph(temp3, r3);
    for (int i = 0; i < 32; i++) checksum += (int)temp3[i];
#endif

#ifdef __AVX512BF16__
    printf("Testing V32BFmode (32x bfloat16)...\n");
    __m512bh r4 = test_v32bfmode_blend();
    __bfloat16 temp4[32];
    _mm512_storeu_bf16(temp4, r4);
    for (int i = 0; i < 32; i++) checksum += temp4[i];
#endif

#ifdef __AVX512F__
    printf("Testing V16SImode (16x int32)...\n");
    __m512i r5 = test_v16simode_blend();
    int32_t temp5[16];
    _mm512_storeu_si512((__m512i*)temp5, r5);
    for (int i = 0; i < 16; i++) checksum += temp5[i];
    
    printf("Testing V8DImode (8x int64)...\n");
    __m512i r6 = test_v8dimode_blend();
    int64_t temp6[8];
    _mm512_storeu_si512((__m512i*)temp6, r6);
    for (int i = 0; i < 8; i++) checksum += (int)temp6[i];
    
    printf("Testing V8DFmode (8x double)...\n");
    __m512d r7 = test_v8dfmode_blend();
    double temp7[8];
    _mm512_storeu_pd(temp7, r7);
    for (int i = 0; i < 8; i++) checksum += (int)temp7[i];
    
    printf("Testing V16SFmode (16x float)...\n");
    __m512 r8 = test_v16sfmode_blend();
    float temp8[16];
    _mm512_storeu_ps(temp8, r8);
    for (int i = 0; i < 16; i++) checksum += (int)temp8[i];
#endif
    
    printf("Final checksum: %d\n", checksum);
    printf("All blend operations tested.\n");
    
    return 0;
}
