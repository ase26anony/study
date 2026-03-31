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

// ==================== V64QImode (64 x 8-bit integers) ====================
#ifdef __AVX512BW__
__attribute__((noinline))
uint64_t test_v64qimode_blend() {
    // Initialize arrays with distinct patterns
    uint8_t src1[64], src2[64];
    for (int i = 0; i < 64; i++) {
        src1[i] = i;           // 0, 1, 2, ...
        src2[i] = 0xFF - i;    // 0xFF, 0xFE, 0xFD, ...
    }
    
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    // Create alternating mask: 0xAA... (10101010 pattern)
    __mmask64 mask = 0xAAAAAAAAAAAAAAAA;
    
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
        src2[i] = 0xFFFF - i * 100;
    }
    
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    // Create checkerboard mask: 0x5555... (01010101 pattern)
    __mmask32 mask = 0x55555555;
    
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
    
    // Create alternating mask
    __mmask32 mask = 0xAAAAAAAA;
    
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
    
    __m512bh v1 = _mm512_loadu_si512((const __m512bh*)src1);
    __m512bh v2 = _mm512_loadu_si512((const __m512bh*)src2);
    
    // Create mask for blending
    __mmask32 mask = 0xCCCCCCCC;
    
    // Blend bfloat16 values - using integer blend since specific BF16 blend might not exist
    __m512bh result = _mm512_mask_blend_epi16(mask, 
        (__m512i)v1, (__m512i)v2);
    
    __bfloat16 out[32];
    _mm512_storeu_si512((__m512i*)out, (__m512i)result);
    
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
    
    // Create mask: lower 8 elements from v1, upper 8 from v2
    __mmask16 mask = 0xFF00;
    
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
        src1[i] = 1LL << (i * 2);
        src2[i] = -(1LL << (i * 2));
    }
    
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    // Create mask: alternating pattern
    __mmask8 mask = 0xAA;  // 10101010
    
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
    
    // Create mask: first 4 from v1, last 4 from v2
    __mmask8 mask = 0xF0;  // 11110000
    
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
    
    // Create alternating mask
    __mmask16 mask = 0xAAAA;  // 1010101010101010
    
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
    float total_float = 0.0f;
    double total_double = 0.0;
    
    printf("Testing AVX-512 blend operations...\n");
    
#ifdef __AVX512BW__
    printf("V64QImode blend: ");
    uint64_t r1 = test_v64qimode_blend();
    printf("checksum = %lu\n", r1);
    total_checksum += r1;
    
    printf("V32HImode blend: ");
    uint64_t r2 = test_v32himode_blend();
    printf("checksum = %lu\n", r2);
    total_checksum += r2;
#else
    printf("AVX512BW not available, skipping V64QI and V32HI modes\n");
#endif

#ifdef __AVX512FP16__
    printf("V32HFmode blend: ");
    float r3 = test_v32hfmode_blend();
    printf("sum = %f\n", r3);
    total_float += r3;
#else
    printf("AVX512FP16 not available, skipping V32HF mode\n");
#endif

#ifdef __AVX512BF16__
    printf("V32BFmode blend: ");
    float r4 = test_v32bfmode_blend();
    printf("sum = %f\n", r4);
    total_float += r4;
#else
    printf("AVX512BF16 not available, skipping V32BF mode\n");
#endif

#ifdef __AVX512F__
    printf("V16SImode blend: ");
    uint64_t r5 = test_v16simode_blend();
    printf("checksum = %lu\n", r5);
    total_checksum += r5;
    
    printf("V8DImode blend: ");
    uint64_t r6 = test_v8dimode_blend();
    printf("checksum = %lu\n", r6);
    total_checksum += r6;
    
    printf("V8DFmode blend: ");
    double r7 = test_v8dfmode_blend();
    printf("sum = %f\n", r7);
    total_double += r7;
    
    printf("V16SFmode blend: ");
    float r8 = test_v16sfmode_blend();
    printf("sum = %f\n", r8);
    total_float += r8;
#else
    printf("AVX512F not available, skipping V16SI, V8DI, V8DF, V16SF modes\n");
#endif
    
    printf("\nFinal aggregates:\n");
    printf("Integer checksum total: %lu\n", total_checksum);
    printf("Float sum total: %f\n", total_float);
    printf("Double sum total: %f\n", total_double);
    
    // Return non-zero if any test failed (simplified check)
    return (total_checksum == 0 && total_float == 0.0f && total_double == 0.0) ? 1 : 0;
}
