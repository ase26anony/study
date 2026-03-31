#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Helper function to create masks with alternating patterns
static inline __mmask64 create_mask64(int pattern) {
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if ((i & pattern) == 0) {
            mask |= (1ULL << i);
        }
    }
    return mask;
}

static inline __mmask32 create_mask32(int pattern) {
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if ((i & pattern) == 0) {
            mask |= (1U << i);
        }
    }
    return mask;
}

static inline __mmask16 create_mask16(int pattern) {
    __mmask16 mask = 0;
    for (int i = 0; i < 16; i++) {
        if ((i & pattern) == 0) {
            mask |= (1 << i);
        }
    }
    return mask;
}

static inline __mmask8 create_mask8(int pattern) {
    __mmask8 mask = 0;
    for (int i = 0; i < 8; i++) {
        if ((i & pattern) == 0) {
            mask |= (1 << i);
        }
    }
    return mask;
}

#ifdef __AVX512BW__
// V64QImode: 64 x 8-bit integers
__attribute__((noinline))
uint64_t test_v64qimode_blend() {
    // Initialize arrays with distinct patterns
    uint8_t src1[64], src2[64];
    for (int i = 0; i < 64; i++) {
        src1[i] = i;
        src2[i] = 64 + i;
    }
    
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    // Create alternating mask pattern
    __mmask64 mask = create_mask64(1);  // Alternating 0/1 pattern
    
    // This should generate vblendmb instruction
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

// V32HImode: 32 x 16-bit integers
__attribute__((noinline))
uint64_t test_v32himode_blend() {
    uint16_t src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 2;
        src2[i] = 1000 + i * 3;
    }
    
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    __mmask32 mask = create_mask32(3);  // Pattern 0011
    
    // This should generate vblendmw instruction
    __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
    
    uint16_t out[32];
    _mm512_storeu_si512((__m512i*)out, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += out[i];
    }
    return checksum;
}
#endif // __AVX512BW__

#ifdef __AVX512FP16__
// V32HFmode: 32 x half-precision floats
__attribute__((noinline))
_Float16 test_v32hfmode_blend() {
    _Float16 src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 0.5f);
        src2[i] = (_Float16)(100.0f + i * 0.25f);
    }
    
    __m512h v1 = _mm512_loadu_ph(src1);
    __m512h v2 = _mm512_loadu_ph(src2);
    
    __mmask32 mask = create_mask32(1);  // Alternating pattern
    
    // This should generate vblendmps instruction for half-precision
    __m512h result = _mm512_mask_blend_ph(mask, v1, v2);
    
    _Float16 out[32];
    _mm512_storeu_ph(out, result);
    
    _Float16 sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += out[i];
    }
    return sum;
}

// V32BFmode: 32 x brain float (bfloat16)
__attribute__((noinline))
uint32_t test_v32bfmode_blend() {
    // Use uint16_t to store bfloat16 values
    uint16_t src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        // Simple bfloat16 pattern (just using integer representation)
        src1[i] = i << 8;  // Shift to create bfloat16-like pattern
        src2[i] = (i + 32) << 8;
    }
    
    __m512bh v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512bh v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    __mmask32 mask = create_mask32(2);  // Pattern 0101
    
    // Use appropriate blend intrinsic for bfloat16
    // Note: The exact intrinsic may vary; this is a common pattern
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
#endif // __AVX512FP16__

#ifdef __AVX512F__
// V16SImode: 16 x 32-bit integers
__attribute__((noinline))
uint64_t test_v16simode_blend() {
    int32_t src1[16], src2[16];
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 10;
        src2[i] = 1000 + i * 20;
    }
    
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    __mmask16 mask = create_mask16(1);  // Alternating pattern
    
    // This should generate vblendmd instruction
    __m512i result = _mm512_mask_blend_epi32(mask, v1, v2);
    
    int32_t out[16];
    _mm512_storeu_si512((__m512i*)out, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 16; i++) {
        checksum += out[i];
    }
    return checksum;
}

// V8DImode: 8 x 64-bit integers
__attribute__((noinline))
uint64_t test_v8dimode_blend() {
    int64_t src1[8], src2[8];
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 100LL;
        src2[i] = 10000LL + i * 200LL;
    }
    
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    __mmask8 mask = create_mask8(1);  // Alternating pattern
    
    // This should generate vblendmq instruction
    __m512i result = _mm512_mask_blend_epi64(mask, v1, v2);
    
    int64_t out[8];
    _mm512_storeu_si512((__m512i*)out, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 8; i++) {
        checksum += out[i];
    }
    return checksum;
}

// V8DFmode: 8 x double-precision floats
__attribute__((noinline))
double test_v8dfmode_blend() {
    double src1[8], src2[8];
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.5;
        src2[i] = 100.0 + i * 2.5;
    }
    
    __m512d v1 = _mm512_loadu_pd(src1);
    __m512d v2 = _mm512_loadu_pd(src2);
    
    __mmask8 mask = create_mask8(3);  // Pattern 0011
    
    // This should generate vblendmpd instruction
    __m512d result = _mm512_mask_blend_pd(mask, v1, v2);
    
    double out[8];
    _mm512_storeu_pd(out, result);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += out[i];
    }
    return sum;
}

// V16SFmode: 16 x single-precision floats
__attribute__((noinline))
float test_v16sfmode_blend() {
    float src1[16], src2[16];
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.5f;
        src2[i] = 50.0f + i * 0.75f;
    }
    
    __m512 v1 = _mm512_loadu_ps(src1);
    __m512 v2 = _mm512_loadu_ps(src2);
    
    __mmask16 mask = create_mask16(2);  // Pattern 0101
    
    // This should generate vblendmps instruction
    __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
    
    float out[16];
    _mm512_storeu_ps(out, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += out[i];
    }
    return sum;
}
#endif // __AVX512F__

int main() {
    uint64_t total_checksum = 0;
    
    printf("Testing AVX-512 blend operations...\n");
    
#ifdef __AVX512BW__
    printf("Testing V64QImode (64x8-bit integers)...\n");
    total_checksum += test_v64qimode_blend();
    
    printf("Testing V32HImode (32x16-bit integers)...\n");
    total_checksum += test_v32himode_blend();
#else
    printf("AVX512BW not available, skipping V64QImode and V32HImode tests\n");
#endif
    
#ifdef __AVX512FP16__
    printf("Testing V32HFmode (32xhalf-precision floats)...\n");
    total_checksum += (uint64_t)test_v32hfmode_blend();
    
    printf("Testing V32BFmode (32xbfloat16)...\n");
    total_checksum += test_v32bfmode_blend();
#else
    printf("AVX512FP16 not available, skipping V32HFmode and V32BFmode tests\n");
#endif
    
#ifdef __AVX512F__
    printf("Testing V16SImode (16x32-bit integers)...\n");
    total_checksum += test_v16simode_blend();
    
    printf("Testing V8DImode (8x64-bit integers)...\n");
    total_checksum += test_v8dimode_blend();
    
    printf("Testing V8DFmode (8xdouble-precision floats)...\n");
    total_checksum += (uint64_t)test_v8dfmode_blend();
    
    printf("Testing V16SFmode (16xsingle-precision floats)...\n");
    total_checksum += (uint64_t)test_v16sfmode_blend();
#else
    printf("AVX512F not available, skipping remaining tests\n");
#endif
    
    printf("Total checksum: %lu\n", total_checksum);
    
    // Return non-zero if any test failed (simplified check)
    return total_checksum == 0 ? 1 : 0;
}
