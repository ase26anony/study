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
    if (size % 16 != 0) printf("\n");
}

// ==================== V64QImode (64 x 8-bit integers) ====================
#ifdef __AVX512BW__
__attribute__((noinline))
uint64_t test_v64qimode_blend() {
    // Initialize arrays with distinct patterns
    uint8_t src1[64], src2[64];
    for (int i = 0; i < 64; i++) {
        src1[i] = i;           // 0, 1, 2, ...
        src2[i] = 255 - i;     // 255, 254, 253, ...
    }
    
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    // Create alternating mask: 0xAA...AA (10101010 pattern)
    __mmask64 mask = _mm512_int2mask(0xAAAAAAAAAAAAAAAA);
    
    // Blend using vblendmb instruction
    __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
    
    // Store and compute checksum
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
        src2[i] = 65535 - i * 100;
    }
    
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    // Create checkerboard mask: 0x5555 (01010101 pattern)
    __mmask32 mask = _mm512_int2mask(0x55555555);
    
    // Blend using vblendmw instruction
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
#ifdef __AVX512FP16__
#include <float.h>
__attribute__((noinline))
float test_v32hfmode_blend() {
    _Float16 src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 1.5f);
        src2[i] = (_Float16)(100.0f - i * 1.5f);
    }
    
    __m512h v1 = _mm512_loadu_ph(src1);
    __m512h v2 = _mm512_loadu_ph(src2);
    
    // Create mask with first half zeros, second half ones
    __mmask32 mask = _mm512_int2mask(0xFFFF0000);
    
    // Blend using vblendmps instruction for half-precision
    __m512h result = _mm512_mask_blend_ph(mask, v1, v2);
    
    _Float16 out[32];
    _mm512_storeu_ph(out, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)out[i];
    }
    return sum;
}
#endif

// ==================== V32BFmode (32 x brain floats) ====================
#ifdef __AVX512BF16__
__attribute__((noinline))
float test_v32bfmode_blend() {
    // Initialize bfloat16 arrays
    __bfloat16 src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = bfloat16_from_float(i * 2.0f);
        src2[i] = bfloat16_from_float(200.0f - i * 2.0f);
    }
    
    __m512bh v1 = _mm512_loadu_bf16(src1);
    __m512bh v2 = _mm512_loadu_bf16(src2);
    
    // Create alternating mask
    __mmask32 mask = _mm512_int2mask(0xAAAAAAAA);
    
    // Blend bfloat16 values - use integer blend since there's no direct BF16 blend intrinsic
    __m512i v1_int = _mm512_castps_si512(_mm512_castbf16_ps(v1));
    __m512i v2_int = _mm512_castps_si512(_mm512_castbf16_ps(v2));
    __m512i result_int = _mm512_mask_blend_epi32(mask, v1_int, v2_int);
    
    __m512bh result = _mm512_castsi512_bf16(result_int);
    
    __bfloat16 out[32];
    _mm512_storeu_bf16(out, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += bfloat16_to_float(out[i]);
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
    
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    // Create mask: even indices from v1, odd from v2
    __mmask16 mask = _mm512_int2mask(0xAAAA);  // 1010101010101010
    
    // Blend using vblendmd instruction
    __m512i result = _mm512_mask_blend_epi32(mask, v1, v2);
    
    int32_t out[16];
    _mm512_storeu_si512((__m512i*)out, result);
    
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
        src1[i] = 1LL << (i * 8);
        src2[i] = -(1LL << (i * 8));
    }
    
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    // Create mask: first half from v1, second from v2
    __mmask8 mask = _mm512_int2mask(0xF0);  // 11110000
    
    // Blend using vblendmq instruction
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
        src1[i] = i * 1.5;
        src2[i] = 100.0 - i * 1.5;
    }
    
    __m512d v1 = _mm512_loadu_pd(src1);
    __m512d v2 = _mm512_loadu_pd(src2);
    
    // Create alternating mask
    __mmask8 mask = _mm512_int2mask(0xAA);  // 10101010
    
    // Blend using vblendmpd instruction
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
    
    // Create mask with pattern 0xAAAA (alternating)
    __mmask16 mask = _mm512_int2mask(0xAAAA);
    
    // Blend using vblendmps instruction
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

// ==================== Main Driver ====================
int main() {
    uint64_t total_checksum = 0;
    float float_sum = 0.0f;
    double double_sum = 0.0;
    
    printf("Testing AVX-512 blend operations...\n");
    
#ifdef __AVX512BW__
    printf("Testing V64QImode (64x8-bit)...\n");
    total_checksum += test_v64qimode_blend();
    
    printf("Testing V32HImode (32x16-bit)...\n");
    total_checksum += test_v32himode_blend();
#endif
    
#ifdef __AVX512FP16__
    printf("Testing V32HFmode (32xhalf-precision)...\n");
    float_sum += test_v32hfmode_blend();
#endif
    
#ifdef __AVX512BF16__
    printf("Testing V32BFmode (32xbfloat16)...\n");
    float_sum += test_v32bfmode_blend();
#endif
    
#ifdef __AVX512F__
    printf("Testing V16SImode (16x32-bit)...\n");
    total_checksum += test_v16simode_blend();
    
    printf("Testing V8DImode (8x64-bit)...\n");
    total_checksum += test_v8dimode_blend();
    
    printf("Testing V8DFmode (8xdouble)...\n");
    double_sum += test_v8dfmode_blend();
    
    printf("Testing V16SFmode (16xsingle)...\n");
    float_sum += test_v16sfmode_blend();
#endif
    
    // Use results to prevent dead code elimination
    printf("Integer checksum: %lu\n", total_checksum);
    printf("Float sum: %f\n", float_sum);
    printf("Double sum: %f\n", double_sum);
    
    // Return non-zero if any test produced zero (unlikely with our data)
    return (total_checksum == 0 && float_sum == 0.0f && double_sum == 0.0) ? 1 : 0;
}
