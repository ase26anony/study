#include <immintrin.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

// Prevent inlining to ensure each function generates separate code
#define NOINLINE __attribute__((noinline, noipa))

// ========== 64 x 8-bit integers (V64QImode) ==========
NOINLINE __m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    return _mm512_mask_blend_epi8(mask, a, b);
}

// ========== 32 x 16-bit integers (V32HImode) ==========
NOINLINE __m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    return _mm512_mask_blend_epi16(mask, a, b);
}

// ========== 16 x 32-bit integers (V16SImode) ==========
NOINLINE __m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    return _mm512_mask_blend_epi32(mask, a, b);
}

// ========== 8 x 64-bit integers (V8DImode) ==========
NOINLINE __m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    return _mm512_mask_blend_epi64(mask, a, b);
}

// ========== 16 x single-precision floats (V16SFmode) ==========
NOINLINE __m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    return _mm512_mask_blend_ps(mask, a, b);
}

// ========== 8 x double-precision floats (V8DFmode) ==========
NOINLINE __m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    return _mm512_mask_blend_pd(mask, a, b);
}

#ifdef __AVX512FP16__
// ========== 32 x half-precision floats (V32HFmode) ==========
NOINLINE __m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    return _mm512_mask_blend_ph(mask, a, b);
}
#endif

#ifdef __AVX512BF16__
// ========== 32 x bfloat16 floats (V32BFmode) ==========
NOINLINE __m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    // Note: There's no direct _mm512_mask_blend_* intrinsic for bfloat16
    // We'll use a combination of operations to achieve blending
    __m512i ai = _mm512_castps_si512(_mm512_castbh_ps(a));
    __m512i bi = _mm512_castps_si512(_mm512_castbh_ps(b));
    __m512i result = _mm512_mask_blend_epi16(mask, ai, bi);
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
    
    // Initialize data arrays with patterns
    uint8_t data8_a[64], data8_b[64];
    uint16_t data16_a[32], data16_b[32];
    uint32_t data32_a[16], data32_b[16];
    uint64_t data64_a[8], data64_b[8];
    float dataf_a[16], dataf_b[16];
    double datad_a[8], datad_b[8];
    
    // Pattern initialization
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
    
    // Load data into vectors
    __m512i v8_a = _mm512_loadu_si512(data8_a);
    __m512i v8_b = _mm512_loadu_si512(data8_b);
    __m512i v16_a = _mm512_loadu_si512(data16_a);
    __m512i v16_b = _mm512_loadu_si512(data16_b);
    __m512i v32_a = _mm512_loadu_si512(data32_a);
    __m512i v32_b = _mm512_loadu_si512(data32_b);
    __m512i v64_a = _mm512_loadu_si512(data64_a);
    __m512i v64_b = _mm512_loadu_si512(data64_b);
    __m512 vf_a = _mm512_loadu_ps(dataf_a);
    __m512 vf_b = _mm512_loadu_ps(dataf_b);
    __m512d vd_a = _mm512_loadu_pd(datad_a);
    __m512d vd_b = _mm512_loadu_pd(datad_b);
    
    // Create masks with alternating patterns
    __mmask64 mask64 = 0xAAAAAAAAAAAAAAAA;  // Alternating bits
    __mmask32 mask32 = 0xAAAAAAAA;          // Alternating bits
    __mmask16 mask16 = 0xAAAA;              // Alternating bits
    __mmask8 mask8 = 0xAA;                  // Alternating bits
    
    // Perform blend operations
    __m512i res8 = blend_v64qi(v8_a, v8_b, mask64);
    __m512i res16 = blend_v32hi(v16_a, v16_b, mask32);
    __m512i res32 = blend_v16si(v32_a, v32_b, mask16);
    __m512i res64 = blend_v8di(v64_a, v64_b, mask8);
    __m512 resf = blend_v16sf(vf_a, vf_b, mask16);
    __m512d resd = blend_v8df(vd_a, vd_b, mask8);
    
    // Store results for checksum computation
    uint8_t result8[64];
    uint16_t result16[32];
    uint32_t result32[16];
    uint64_t result64[8];
    float resultf[16];
    double resultd[8];
    
    _mm512_storeu_si512(result8, res8);
    _mm512_storeu_si512(result16, res16);
    _mm512_storeu_si512(result32, res32);
    _mm512_storeu_si512(result64, res64);
    _mm512_storeu_ps(resultf, resf);
    _mm512_storeu_pd(resultd, resd);
    
    // Compute checksums
    total_checksum += compute_checksum(result8, sizeof(result8));
    total_checksum += compute_checksum(result16, sizeof(result16));
    total_checksum += compute_checksum(result32, sizeof(result32));
    total_checksum += compute_checksum(result64, sizeof(result64));
    total_checksum += compute_checksum(resultf, sizeof(resultf));
    total_checksum += compute_checksum(resultd, sizeof(resultd));
    
#ifdef __AVX512FP16__
    // Half-precision floats
    _Float16 datah_a[32], datah_b[32];
    for (int i = 0; i < 32; i++) {
        datah_a[i] = i * 0.25f;
        datah_b[i] = i * 0.75f;
    }
    __m512h vh_a = _mm512_loadu_ph(datah_a);
    __m512h vh_b = _mm512_loadu_ph(datah_b);
    __m512h resh = blend_v32hf(vh_a, vh_b, mask32);
    _Float16 resulth[32];
    _mm512_storeu_ph(resulth, resh);
    total_checksum += compute_checksum(resulth, sizeof(resulth));
#endif

#ifdef __AVX512BF16__
    // Bfloat16 floats
    __bfloat16 datab_a[32], datab_b[32];
    for (int i = 0; i < 32; i++) {
        datab_a[i] = i * 0.125f;
        datab_b[i] = i * 0.375f;
    }
    __m512bh vb_a = _mm512_loadu_bh(datab_a);
    __m512bh vb_b = _mm512_loadu_bh(datab_b);
    __m512bh resb = blend_v32bf(vb_a, vb_b, mask32);
    __bfloat16 resultb[32];
    _mm512_storeu_bh(resultb, resb);
    total_checksum += compute_checksum(resultb, sizeof(resultb));
#endif
    
    // Print final checksum
    printf("Total checksum: %lu\n", total_checksum);
    
    return 0;
}
