#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

// Helper function to print results (for debugging)
void print_512i_u8(__m512i v) {
    uint8_t arr[64];
    _mm512_storeu_si512((void*)arr, v);
    for (int i = 0; i < 64; i++) {
        printf("%02x ", arr[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");
}

// ==================== V64QImode (64 x 8-bit integers) ====================
#ifdef __AVX512BW__
__attribute__((noinline))
uint64_t test_v64qimode_blend() {
    // Create two arrays with distinct patterns
    uint8_t src1[64], src2[64];
    for (int i = 0; i < 64; i++) {
        src1[i] = i;           // 0, 1, 2, ...
        src2[i] = 0xFF - i;    // 0xFF, 0xFE, ...
    }
    
    __m512i a = _mm512_loadu_si512((const void*)src1);
    __m512i b = _mm512_loadu_si512((const void*)src2);
    
    // Create alternating mask: 0xAA...AA (10101010 pattern)
    __mmask64 mask = 0xAAAAAAAAAAAAAAAA;
    
    // This should generate vblendmb
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Force usage of result
    uint8_t out[64];
    _mm512_storeu_si512((void*)out, result);
    
    // Compute checksum
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
    
    __m512i a = _mm512_loadu_si512((const void*)src1);
    __m512i b = _mm512_loadu_si512((const void*)src2);
    
    // Create checkerboard mask: 0x5555... (01010101 pattern)
    __mmask32 mask = 0x55555555;
    
    // This should generate vblendmw
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    uint16_t out[32];
    _mm512_storeu_si512((void*)out, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += out[i];
    }
    return checksum;
}
#endif

// ==================== V32HFmode (32 x half-precision floats) ====================
#ifdef __AVX512FP16__
#include <float.h>
__attribute__((noinline))
_Float16 test_v32hfmode_blend() {
    _Float16 src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 1.5f);
        src2[i] = (_Float16)(100.0f - i * 1.5f);
    }
    
    __m512h a = _mm512_loadu_ph((const void*)src1);
    __m512h b = _mm512_loadu_ph((const void*)src2);
    
    // Create mask with first half ones, second half zeros
    __mmask32 mask = 0x0000FFFF;
    
    // This should generate vblendmps for half-precision
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    _Float16 out[32];
    _mm512_storeu_ph((void*)out, result);
    
    _Float16 sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += out[i];
    }
    return sum;
}
#endif

// ==================== V32BFmode (32 x brain float) ====================
#ifdef __AVX512BF16__
#ifdef __AVX512FP16__
__attribute__((noinline))
uint32_t test_v32bfmode_blend() {
    // BF16 is typically handled through conversion from float
    float src1_f[32], src2_f[32];
    for (int i = 0; i < 32; i++) {
        src1_f[i] = i * 2.0f;
        src2_f[i] = 100.0f - i * 2.0f;
    }
    
    // Load as float and convert to BF16
    __m512 a_f = _mm512_loadu_ps(src1_f);
    __m512 b_f = _mm512_loadu_ps(src2_f);
    
    __m512bh a = (__m512bh)_mm512_cvtneps_pbh(a_f);
    __m512bh b = (__m512bh)_mm512_cvtneps_pbh(b_f);
    
    // Create alternating mask
    __mmask32 mask = 0xAAAAAAAA;
    
    // Blend BF16 values - may use vblendmps on BF16 elements
    __m512bh result = _mm512_mask_blend_epi32(mask, 
        (__m512i)a, (__m512i)b);
    
    // Convert back for verification
    __m512 result_f = _mm512_cvtpbh_ps((__m128bh)_mm512_castsi512_si128(result));
    
    float out[32];
    _mm512_storeu_ps(out, result_f);
    
    uint32_t checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += *(uint32_t*)&out[i];
    }
    return checksum;
}
#endif
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
    
    __m512i a = _mm512_loadu_si512((const void*)src1);
    __m512i b = _mm512_loadu_si512((const void*)src2);
    
    // Mask with every other element set
    __mmask16 mask = 0xAAAA;  // 1010101010101010
    
    // This should generate vblendmd
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    int32_t out[16];
    _mm512_storeu_si512((void*)out, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 16; i++) {
        checksum += (uint64_t)out[i];
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
        src1[i] = i * 10000LL;
        src2[i] = -i * 10000LL;
    }
    
    __m512i a = _mm512_loadu_si512((const void*)src1);
    __m512i b = _mm512_loadu_si512((const void*)src2);
    
    // Mask: select first 4 from a, last 4 from b
    __mmask8 mask = 0x0F;  // 00001111
    
    // This should generate vblendmq
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    int64_t out[8];
    _mm512_storeu_si512((void*)out, result);
    
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
        src1[i] = i * 1.5;
        src2[i] = 100.0 - i * 1.5;
    }
    
    __m512d a = _mm512_loadu_pd(src1);
    __m512d b = _mm512_loadu_pd(src2);
    
    // Mask: select where i % 2 == 0
    __mmask8 mask = 0x55;  // 01010101
    
    // This should generate vblendmpd
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    double out[8];
    _mm512_storeu_pd(out, result);
    
    double sum = 0.0;
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
    
    __m512 a = _mm512_loadu_ps(src1);
    __m512 b = _mm512_loadu_ps(src2);
    
    // Mask: select first 8 from a, last 8 from b
    __mmask16 mask = 0x00FF;  // 0000000011111111
    
    // This should generate vblendmps
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    float out[16];
    _mm512_storeu_ps(out, result);
    
    float sum = 0.0f;
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
    printf("Testing V64QImode (64x8-bit)...\n");
    total_checksum += test_v64qimode_blend();
    
    printf("Testing V32HImode (32x16-bit)...\n");
    total_checksum += test_v32himode_blend();
#endif
    
#ifdef __AVX512FP16__
    printf("Testing V32HFmode (32xhalf)...\n");
    total_checksum += (uint64_t)test_v32hfmode_blend();
#endif
    
#ifdef __AVX512BF16__
#ifdef __AVX512FP16__
    printf("Testing V32BFmode (32xbfloat16)...\n");
    total_checksum += test_v32bfmode_blend();
#endif
#endif
    
#ifdef __AVX512F__
    printf("Testing V16SImode (16x32-bit)...\n");
    total_checksum += test_v16simode_blend();
    
    printf("Testing V8DImode (8x64-bit)...\n");
    total_checksum += test_v8dimode_blend();
    
    printf("Testing V8DFmode (8xdouble)...\n");
    total_checksum += (uint64_t)test_v8dfmode_blend();
    
    printf("Testing V16SFmode (16xfloat)...\n");
    total_checksum += (uint64_t)test_v16sfmode_blend();
#endif
    
    printf("Total checksum: %lu\n", total_checksum);
    
    // Return non-zero to indicate success (prevents optimization)
    return total_checksum != 0 ? 0 : 1;
}
