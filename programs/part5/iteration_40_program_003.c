#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

// Helper function to print results for debugging
void print_hex(const void* data, size_t size) {
    const unsigned char* p = (const unsigned char*)data;
    for (size_t i = 0; i < size; i++) {
        printf("%02x", p[i]);
        if ((i + 1) % 16 == 0) printf("\n");
        else if ((i + 1) % 4 == 0) printf(" ");
    }
    printf("\n");
}

// ==================== V64QImode (64 x 8-bit integers) ====================
#ifdef __AVX512BW__
__attribute__((noinline))
uint64_t test_v64qimode_blend() {
    // Initialize arrays with distinct patterns
    uint8_t src1[64], src2[64];
    for (int i = 0; i < 64; i++) {
        src1[i] = i;           // 0, 1, 2, ..., 63
        src2[i] = 0xFF - i;    // 255, 254, 253, ..., 192
    }
    
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    // Create alternating mask: 0xAA...AA (10101010 pattern)
    __mmask64 mask = 0xAAAAAAAAAAAAAAAA;
    
    // This should generate vblendmb
    __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
    
    // Store and compute checksum to prevent optimization
    uint8_t out[64];
    _mm512_storeu_si512((__m512i*)out, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum += out[i];
    }
    return checksum;
}
#endif

// ==================== V32HImode (32 x 16-bit integers) ====================
#ifdef __AVX512BW__
__attribute__((noinline))
uint64_t test_v32himode_blend() {
    uint16_t src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 100;
        src2[i] = 0xFFFF - i * 100;
    }
    
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    // Create checkerboard mask: 0x5555... (01010101 pattern)
    __mmask32 mask = 0x55555555;
    
    // This should generate vblendmw
    __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
    
    uint16_t out[32];
    _mm512_storeu_si512((__m512i*)out, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += out[i];
    }
    return checksum;
}
#endif

// ==================== V32HFmode (32 x half-precision floats) ====================
#ifdef __AVX512FP16
#include <float.h>
__attribute__((noinline))
_Float16 test_v32hfmode_blend() {
    _Float16 src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 1.5f);
        src2[i] = (_Float16)(100.0f - i * 1.5f);
    }
    
    __m512h v1 = _mm512_loadu_ph(src1);
    __m512h v2 = _mm512_loadu_ph(src2);
    
    // Create mask with first half ones, second half zeros
    __mmask32 mask = 0x0000FFFF;
    
    // This should generate vblendmps for half-precision
    __m512h result = _mm512_mask_blend_ph(mask, v1, v2);
    
    _Float16 out[32];
    _mm512_storeu_ph(out, result);
    
    _Float16 sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += out[i];
    }
    return sum;
}
#endif

// ==================== V32BFmode (32 x brain float) ====================
#ifdef __AVX512BF16
__attribute__((noinline))
float test_v32bfmode_blend() {
    // BF16 is typically handled through conversion from float
    float src1_f[32], src2_f[32];
    for (int i = 0; i < 32; i++) {
        src1_f[i] = i * 2.0f;
        src2_f[i] = 100.0f - i * 2.0f;
    }
    
    // Convert to BF16
    __m512bh v1 = (__m512bh)_mm512_cvtneps_pbh(_mm512_loadu_ps(src1_f));
    __m512bh v2 = (__m512bh)_mm512_cvtneps_pbh(_mm512_loadu_ps(src2_f));
    
    // Create alternating mask
    __mmask32 mask = 0xAAAAAAAA;
    
    // Blend BF16 vectors - this should use appropriate blending
    __m512bh result = _mm512_mask_blend_epi16(mask, 
        (__m512i)v1, (__m512i)v2);
    
    // Convert back to float for checksum
    __m512 result_f = _mm512_cvtpbh_ps((__m128bh)_mm512_castsi512_si128(result));
    
    float out[16];  // Only first 16 elements for simplicity
    _mm512_storeu_ps(out, result_f);
    
    float sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += out[i];
    }
    return sum;
}
#endif

// ==================== V16SImode (16 x 32-bit integers) ====================
#ifdef __AVX512F__
__attribute__((noinline))
uint64_t test_v16simode_blend() {
    int32_t src1[16], src2[16];
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1000;
        src2[i] = -i * 1000;
    }
    
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    // Mask with every other element set
    __mmask16 mask = 0xAAAA;  // 1010101010101010
    
    // This should generate vblendmd
    __m512i result = _mm512_mask_blend_epi32(mask, v1, v2);
    
    int32_t out[16];
    _mm512_storeu_si512((__m512i*)out, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 16; i++) {
        checksum += (uint32_t)out[i];
    }
    return checksum;
}
#endif

// ==================== V8DImode (8 x 64-bit integers) ====================
#ifdef __AVX512F__
__attribute__((noinline))
uint64_t test_v8dimode_blend() {
    int64_t src1[8], src2[8];
    for (int i = 0; i < 8; i++) {
        src1[i] = 0x1000 * i;
        src2[i] = -0x1000 * i;
    }
    
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    // Mask: select first 4 from v1, last 4 from v2
    __mmask8 mask = 0xF0;  // 11110000
    
    // This should generate vblendmq
    __m512i result = _mm512_mask_blend_epi64(mask, v1, v2);
    
    int64_t out[8];
    _mm512_storeu_si512((__m512i*)out, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 8; i++) {
        checksum += (uint64_t)out[i];
    }
    return checksum;
}
#endif

// ==================== V8DFmode (8 x double-precision floats) ====================
#ifdef __AVX512F__
__attribute__((noinline))
double test_v8dfmode_blend() {
    double src1[8], src2[8];
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.25;
        src2[i] = 100.0 - i * 1.25;
    }
    
    __m512d v1 = _mm512_loadu_pd(src1);
    __m512d v2 = _mm512_loadu_pd(src2);
    
    // Create mask using comparison
    __m512d cmp = _mm512_set1_pd(50.0);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, cmp, _CMP_LT_OQ);
    
    // This should generate vblendmpd
    __m512d result = _mm512_mask_blend_pd(mask, v1, v2);
    
    double out[8];
    _mm512_storeu_pd(out, result);
    
    double sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += out[i];
    }
    return sum;
}
#endif

// ==================== V16SFmode (16 x single-precision floats) ====================
#ifdef __AVX512F__
__attribute__((noinline))
float test_v16sfmode_blend() {
    float src1[16], src2[16];
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.5f;
        src2[i] = 50.0f - i * 0.5f;
    }
    
    __m512 v1 = _mm512_loadu_ps(src1);
    __m512 v2 = _mm512_loadu_ps(src2);
    
    // Create mask using comparison
    __m512 cmp = _mm512_set1_ps(10.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, cmp, _CMP_GT_OQ);
    
    // This should generate vblendmps
    __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
    
    float out[16];
    _mm512_storeu_ps(out, result);
    
    float sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += out[i];
    }
    return sum;
}
#endif

// ==================== Main Driver ====================
int main() {
    uint64_t total_checksum = 0;
    
    printf("Testing AVX-512 blend operations...\n");
    
#ifdef __AVX512BW__
    printf("Testing V64QImode (8-bit integers)...\n");
    total_checksum += test_v64qimode_blend();
    
    printf("Testing V32HImode (16-bit integers)...\n");
    total_checksum += test_v32himode_blend();
#endif
    
#ifdef __AVX512FP16__
    printf("Testing V32HFmode (half-precision floats)...\n");
    total_checksum += (uint64_t)test_v32hfmode_blend();
#endif
    
#ifdef __AVX512BF16__
    printf("Testing V32BFmode (brain floats)...\n");
    total_checksum += (uint64_t)test_v32bfmode_blend();
#endif
    
#ifdef __AVX512F__
    printf("Testing V16SImode (32-bit integers)...\n");
    total_checksum += test_v16simode_blend();
    
    printf("Testing V8DImode (64-bit integers)...\n");
    total_checksum += test_v8dimode_blend();
    
    printf("Testing V8DFmode (double-precision floats)...\n");
    total_checksum += (uint64_t)test_v8dfmode_blend();
    
    printf("Testing V16SFmode (single-precision floats)...\n");
    total_checksum += (uint64_t)test_v16sfmode_blend();
#endif
    
    printf("Total checksum: %lu\n", total_checksum);
    
    // Return non-zero to indicate success (prevents optimization)
    return total_checksum != 0 ? 0 : 1;
}
