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
        else if ((i + 1) % 8 == 0) printf(" ");
    }
    printf("\n");
}

// ==================== V64QImode (64x 8-bit integers) ====================
#ifdef __AVX512BW__
__attribute__((noinline))
uint64_t test_v64qimode_blend() {
    // Initialize arrays with distinct patterns
    uint8_t src1[64], src2[64];
    for (int i = 0; i < 64; i++) {
        src1[i] = i;           // 0, 1, 2, ...
        src2[i] = 255 - i;     // 255, 254, 253, ...
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

// ==================== V32HImode (32x 16-bit integers) ====================
#ifdef __AVX512BW__
__attribute__((noinline))
uint64_t test_v32himode_blend() {
    int16_t src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 100;
        src2[i] = -i * 100;
    }
    
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    // Create mask with alternating groups of 4 bits
    __mmask32 mask = 0xAAAA5555;
    
    // This should generate vblendmw
    __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
    
    int16_t out[32];
    _mm512_storeu_si512((__m512i*)out, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += (uint16_t)out[i];
    }
    return checksum;
}
#endif

// ==================== V32HFmode (32x half-precision floats) ====================
#ifdef __AVX512FP16__
#include <float.h>
__attribute__((noinline))
_Float16 test_v32hfmode_blend() {
    _Float16 src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 1.5f);
        src2[i] = (_Float16)(i * 2.5f);
    }
    
    __m512h v1 = _mm512_loadu_ph(src1);
    __m512h v2 = _mm512_loadu_ph(src2);
    
    // Create checkerboard mask
    __mmask32 mask = 0x55555555;
    
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

// ==================== V32BFmode (32x brain floats) ====================
#ifdef __AVX512BF16__
__attribute__((noinline))
uint32_t test_v32bfmode_blend() {
    // Use __m512bh for bfloat16 vectors
    __m512bh v1, v2;
    
    // Initialize with patterns
    uint16_t src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = i << 8;      // Simple bfloat16 pattern
        src2[i] = (31 - i) << 8;
    }
    
    v1 = _mm512_loadu_si512((__m512i*)src1);
    v2 = _mm512_loadu_si512((__m512i*)src2);
    
    // Create mask
    __mmask32 mask = 0x33333333;
    
    // Blend bfloat16 vectors - may use multiple instructions
    // The compiler should generate appropriate blend for BF16
    __m512bh result = _mm512_mask_blend_epi16(mask, 
        (__m512i)v1, (__m512i)v2);
    
    uint16_t out[32];
    _mm512_storeu_si512((__m512i*)out, (__m512i)result);
    
    uint32_t checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += out[i];
    }
    return checksum;
}
#endif

// ==================== V16SImode (16x 32-bit integers) ====================
#ifdef __AVX512F__
__attribute__((noinline))
uint64_t test_v16simode_blend() {
    int32_t src1[16], src2[16];
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1000;
        src2[i] = i * 2000;
    }
    
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    // Create mask with pattern 0xAAAA (1010101010101010)
    __mmask16 mask = 0xAAAA;
    
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

// ==================== V8DImode (8x 64-bit integers) ====================
#ifdef __AVX512F__
__attribute__((noinline))
uint64_t test_v8dimode_blend() {
    int64_t src1[8], src2[8];
    for (int i = 0; i < 8; i++) {
        src1[i] = 0x1000 * i;
        src2[i] = 0x2000 * i;
    }
    
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    // Create mask: 0xAA (10101010)
    __mmask8 mask = 0xAA;
    
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

// ==================== V8DFmode (8x double-precision floats) ====================
#ifdef __AVX512F__
__attribute__((noinline))
double test_v8dfmode_blend() {
    double src1[8], src2[8];
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.1;
        src2[i] = i * 2.2;
    }
    
    __m512d v1 = _mm512_loadu_pd(src1);
    __m512d v2 = _mm512_loadu_pd(src2);
    
    // Create mask: 0x55 (01010101)
    __mmask8 mask = 0x55;
    
    // This should generate vblendmpd
    __m512d result = _mm512_mask_blend_pd(mask, v1, v2);
    
    double out[8];
    _mm512_storeu_pd(out, result);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += out[i];
    }
    return sum;
}
#endif

// ==================== V16SFmode (16x single-precision floats) ====================
#ifdef __AVX512F__
__attribute__((noinline))
float test_v16sfmode_blend() {
    float src1[16], src2[16];
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1.5f;
        src2[i] = i * 3.5f;
    }
    
    __m512 v1 = _mm512_loadu_ps(src1);
    __m512 v2 = _mm512_loadu_ps(src2);
    
    // Create mask with alternating pattern
    __mmask16 mask = 0xAAAA;
    
    // This should generate vblendmps
    __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
    
    float out[16];
    _mm512_storeu_ps(out, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += out[i];
    }
    return sum;
}
#endif

// ==================== Main driver ====================
int main() {
    uint64_t total_checksum = 0;
    
    printf("Testing AVX-512 blend operations...\n");
    
#ifdef __AVX512BW__
    printf("Testing V64QImode (64x int8)...\n");
    total_checksum += test_v64qimode_blend();
    
    printf("Testing V32HImode (32x int16)...\n");
    total_checksum += test_v32himode_blend();
#else
    printf("AVX512BW not available, skipping V64QImode and V32HImode\n");
#endif

#ifdef __AVX512FP16__
    printf("Testing V32HFmode (32x half-precision)...\n");
    // Convert half to integer for checksum
    total_checksum += (uint64_t)test_v32hfmode_blend();
#else
    printf("AVX512FP16 not available, skipping V32HFmode\n");
#endif

#ifdef __AVX512BF16__
    printf("Testing V32BFmode (32x bfloat16)...\n");
    total_checksum += test_v32bfmode_blend();
#else
    printf("AVX512BF16 not available, skipping V32BFmode\n");
#endif

#ifdef __AVX512F__
    printf("Testing V16SImode (16x int32)...\n");
    total_checksum += test_v16simode_blend();
    
    printf("Testing V8DImode (8x int64)...\n");
    total_checksum += test_v8dimode_blend();
    
    printf("Testing V8DFmode (8x double)...\n");
    total_checksum += (uint64_t)test_v8dfmode_blend();
    
    printf("Testing V16SFmode (16x float)...\n");
    total_checksum += (uint64_t)test_v16sfmode_blend();
#else
    printf("AVX512F not available, skipping V16SI/V8DI/V8DF/V16SF modes\n");
#endif
    
    printf("Total checksum: %lu\n", total_checksum);
    
    // Return non-zero if any test failed (simplified check)
    return total_checksum == 0 ? 1 : 0;
}
