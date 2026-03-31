#include <stdio.h>
#include <stdint.h>
#include <immintrin.h>
#include <string.h>

// Prevent inlining to ensure each function generates separate code
#define NOINLINE __attribute__((noinline, noipa))

// ==================== 64 x 8-bit integers (V64QImode) ====================
NOINLINE __m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    return _mm512_mask_blend_epi8(mask, a, b);
}

// ==================== 32 x 16-bit integers (V32HImode) ====================
NOINLINE __m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    return _mm512_mask_blend_epi16(mask, a, b);
}

// ==================== 16 x 32-bit integers (V16SImode) ====================
NOINLINE __m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    return _mm512_mask_blend_epi32(mask, a, b);
}

// ==================== 8 x 64-bit integers (V8DImode) ====================
NOINLINE __m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    return _mm512_mask_blend_epi64(mask, a, b);
}

// ==================== 16 x single-precision floats (V16SFmode) ====================
NOINLINE __m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    return _mm512_mask_blend_ps(mask, a, b);
}

// ==================== 8 x double-precision floats (V8DFmode) ====================
NOINLINE __m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    return _mm512_mask_blend_pd(mask, a, b);
}

#ifdef __AVX512FP16__
// ==================== 32 x half-precision floats (V32HFmode) ====================
NOINLINE __m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    return _mm512_mask_blend_ph(mask, a, b);
}
#endif

#ifdef __AVX512BF16__
// ==================== 32 x bfloat16 floats (V32BFmode) ====================
// Note: There's no direct _mm512_mask_blend for bfloat16, so we use integer blend
NOINLINE __m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    // Convert to epi16 for blending, then back
    __m512i a_int = _mm512_castps_si512(_mm512_castbh_ps(a));
    __m512i b_int = _mm512_castps_si512(_mm512_castbh_ps(b));
    __m512i result = _mm512_mask_blend_epi16(mask, a_int, b_int);
    return _mm512_castsi512_bh(result);
}
#endif

// Helper function to compute checksum
uint64_t compute_checksum(void* data, size_t size) {
    uint64_t sum = 0;
    uint8_t* ptr = (uint8_t*)data;
    for (size_t i = 0; i < size; i++) {
        sum += ptr[i];
    }
    return sum;
}

int main() {
    uint64_t total_checksum = 0;
    
    // Initialize data with patterns
    uint8_t data8_a[64], data8_b[64];
    uint16_t data16_a[32], data16_b[32];
    uint32_t data32_a[16], data32_b[16];
    uint64_t data64_a[8], data64_b[8];
    float dataf_a[16], dataf_b[16];
    double datad_a[8], datad_b[8];
    
    for (int i = 0; i < 64; i++) {
        data8_a[i] = i;
        data8_b[i] = i * 2;
    }
    for (int i = 0; i < 32; i++) {
        data16_a[i] = i;
        data16_b[i] = i * 3;
    }
    for (int i = 0; i < 16; i++) {
        data32_a[i] = i;
        data32_b[i] = i * 4;
    }
    for (int i = 0; i < 8; i++) {
        data64_a[i] = i;
        data64_b[i] = i * 5;
    }
    for (int i = 0; i < 16; i++) {
        dataf_a[i] = i * 0.5f;
        dataf_b[i] = i * 1.5f;
    }
    for (int i = 0; i < 8; i++) {
        datad_a[i] = i * 0.25;
        datad_b[i] = i * 0.75;
    }
    
    // Create alternating bit masks for different widths
    __mmask64 mask64 = 0xAAAAAAAAAAAAAAAA;  // Alternating bits for 64 elements
    __mmask32 mask32 = 0xAAAAAAAA;          // Alternating bits for 32 elements
    __mmask16 mask16 = 0xAAAA;              // Alternating bits for 16 elements
    __mmask8 mask8 = 0xAA;                  // Alternating bits for 8 elements
    
    // Perform blend operations for each type
    __m512i vec8_a = _mm512_loadu_si512(data8_a);
    __m512i vec8_b = _mm512_loadu_si512(data8_b);
    __m512i result8 = blend_v64qi(vec8_a, vec8_b, mask64);
    _mm512_storeu_si512(data8_a, result8);
    total_checksum += compute_checksum(data8_a, sizeof(data8_a));
    
    __m512i vec16_a = _mm512_loadu_si512(data16_a);
    __m512i vec16_b = _mm512_loadu_si512(data16_b);
    __m512i result16 = blend_v32hi(vec16_a, vec16_b, mask32);
    _mm512_storeu_si512(data16_a, result16);
    total_checksum += compute_checksum(data16_a, sizeof(data16_a));
    
    __m512i vec32_a = _mm512_loadu_si512(data32_a);
    __m512i vec32_b = _mm512_loadu_si512(data32_b);
    __m512i result32 = blend_v16si(vec32_a, vec32_b, mask16);
    _mm512_storeu_si512(data32_a, result32);
    total_checksum += compute_checksum(data32_a, sizeof(data32_a));
    
    __m512i vec64_a = _mm512_loadu_si512(data64_a);
    __m512i vec64_b = _mm512_loadu_si512(data64_b);
    __m512i result64 = blend_v8di(vec64_a, vec64_b, mask8);
    _mm512_storeu_si512(data64_a, result64);
    total_checksum += compute_checksum(data64_a, sizeof(data64_a));
    
    __m512 vecf_a = _mm512_loadu_ps(dataf_a);
    __m512 vecf_b = _mm512_loadu_ps(dataf_b);
    __m512 resultf = blend_v16sf(vecf_a, vecf_b, mask16);
    _mm512_storeu_ps(dataf_a, resultf);
    total_checksum += compute_checksum(dataf_a, sizeof(dataf_a));
    
    __m512d vecd_a = _mm512_loadu_pd(datad_a);
    __m512d vecd_b = _mm512_loadu_pd(datad_b);
    __m512d resultd = blend_v8df(vecd_a, vecd_b, mask8);
    _mm512_storeu_pd(datad_a, resultd);
    total_checksum += compute_checksum(datad_a, sizeof(datad_a));
    
#ifdef __AVX512FP16__
    // Half-precision floats
    _Float16 datah_a[32], datah_b[32];
    for (int i = 0; i < 32; i++) {
        datah_a[i] = i * 0.25f;
        datah_b[i] = i * 0.75f;
    }
    __m512h vech_a = _mm512_loadu_ph(datah_a);
    __m512h vech_b = _mm512_loadu_ph(datah_b);
    __m512h resulth = blend_v32hf(vech_a, vech_b, mask32);
    _mm512_storeu_ph(datah_a, resulth);
    total_checksum += compute_checksum(datah_a, sizeof(datah_a));
#endif
    
#ifdef __AVX512BF16__
    // Bfloat16 floats
    __bfloat16 datab_a[32], datab_b[32];
    for (int i = 0; i < 32; i++) {
        datab_a[i] = i * 0.125f;
        datab_b[i] = i * 0.375f;
    }
    __m512bh vecb_a = _mm512_loadu_bh(datab_a);
    __m512bh vecb_b = _mm512_loadu_bh(datab_b);
    __m512bh resultb = blend_v32bf(vecb_a, vecb_b, mask32);
    _mm512_storeu_bh(datab_a, resultb);
    total_checksum += compute_checksum(datab_a, sizeof(datab_a));
#endif
    
    // Print final checksum to ensure code isn't optimized away
    printf("Total checksum: %lu\n", total_checksum);
    
    return 0;
}
