#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __AVX512F__
// AVX-512F: 16xSI, 8xDI, 16xSF, 8xDF
void test_avx512f() {
    alignas(64) int32_t src1_si[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    alignas(64) int32_t src2_si[16] = {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};
    alignas(64) int32_t dst_si[16];
    
    alignas(64) int64_t src1_di[8] = {0,1,2,3,4,5,6,7};
    alignas(64) int64_t src2_di[8] = {7,6,5,4,3,2,1,0};
    alignas(64) int64_t dst_di[8];
    
    alignas(64) float src1_sf[16] = {0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
                                     8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f};
    alignas(64) float src2_sf[16] = {15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
                                     7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f};
    alignas(64) float dst_sf[16];
    
    alignas(64) double src1_df[8] = {0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0};
    alignas(64) double src2_df[8] = {7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0};
    alignas(64) double dst_df[8];
    
    // Multi-stage pipeline with varying masks
    for (int iter = 0; iter < 4; iter++) {
        __m512i v_si1 = _mm512_load_si512(src1_si);
        __m512i v_si2 = _mm512_load_si512(src2_si);
        
        // Constant mask blend (E_V16SImode)
        __mmask16 mask_si = (iter % 2) ? 0xAAAA : 0x5555;
        __m512i v_si_blend = _mm512_mask_blend_epi32(mask_si, v_si1, v_si2);
        
        // Loop-based blend with varying mask
        __m512i v_si_result = _mm512_setzero_si512();
        for (int i = 0; i < 16; i++) {
            __mmask16 dynamic_mask = (i % 3) ? 0xFFFF : 0x0000;
            __m512i v_temp = _mm512_mask_blend_epi32(dynamic_mask, v_si_blend, v_si1);
            // Force compiler to keep the operation
            asm volatile("" : "+v"(v_temp));
            v_si_result = _mm512_add_epi32(v_si_result, v_temp);
        }
        _mm512_store_si512(dst_si, v_si_result);
        
        // Double precision blend (E_V8DFmode)
        __m512d v_df1 = _mm512_load_pd(src1_df);
        __m512d v_df2 = _mm512_load_pd(src2_df);
        __mmask8 mask_df = 0xAA; // Alternating pattern
        __m512d v_df_blend = _mm512_mask_blend_pd(mask_df, v_df1, v_df2);
        
        // Multi-stage processing
        __m512d v_df_temp = _mm512_add_pd(v_df_blend, _mm512_set1_pd(1.0));
        __m512d v_df_result = _mm512_mask_blend_pd(0x55, v_df_temp, v_df1);
        _mm512_store_pd(dst_df, v_df_result);
        
        // Single precision blend (E_V16SFmode)
        __m512 v_sf1 = _mm512_load_ps(src1_sf);
        __m512 v_sf2 = _mm512_load_ps(src2_sf);
        __mmask16 mask_sf = 0xAAAA; // Alternating pattern
        __m512 v_sf_blend = _mm512_mask_blend_ps(mask_sf, v_sf1, v_sf2);
        
        // Chain blends for pipeline
        __m512 v_sf_temp = _mm512_add_ps(v_sf_blend, _mm512_set1_ps(0.5f));
        __m512 v_sf_result = _mm512_mask_blend_ps(0x5555, v_sf_temp, v_sf2);
        _mm512_store_ps(dst_sf, v_sf_result);
        
        // 64-bit integer blend (E_V8DImode)
        __m512i v_di1 = _mm512_load_si512(src1_di);
        __m512i v_di2 = _mm512_load_si512(src2_di);
        __mmask8 mask_di = (iter % 2) ? 0xFF : 0x00;
        __m512i v_di_blend = _mm512_mask_blend_epi64(mask_di, v_di1, v_di2);
        
        // Data-dependent blend
        __m512i v_di_shift = _mm512_slli_epi64(v_di_blend, 1);
        __m512i v_di_result = _mm512_mask_blend_epi64(0xAA, v_di_shift, v_di1);
        _mm512_store_si512(dst_di, v_di_result);
    }
    
    // Compute checksums
    int64_t checksum_si = 0;
    double checksum_df = 0.0;
    for (int i = 0; i < 16; i++) checksum_si += dst_si[i];
    for (int i = 0; i < 8; i++) checksum_df += dst_df[i];
    
    printf("AVX512F Checksums: SI=%ld, DF=%f\n", checksum_si, checksum_df);
}
#endif

#ifdef __AVX512BW__
// AVX-512BW: 64xQI, 32xHI
void test_avx512bw() {
    alignas(64) int8_t src1_qi[64];
    alignas(64) int8_t src2_qi[64];
    alignas(64) int8_t dst_qi[64];
    
    alignas(64) int16_t src1_hi[32];
    alignas(64) int16_t src2_hi[32];
    alignas(64) int16_t dst_hi[32];
    
    // Initialize with patterns
    for (int i = 0; i < 64; i++) {
        src1_qi[i] = i;
        src2_qi[i] = 63 - i;
    }
    for (int i = 0; i < 32; i++) {
        src1_hi[i] = i * 2;
        src2_hi[i] = 62 - i * 2;
    }
    
    // Multi-iteration pipeline
    for (int iter = 0; iter < 3; iter++) {
        __m512i v_qi1 = _mm512_load_si512(src1_qi);
        __m512i v_qi2 = _mm512_load_si512(src2_qi);
        
        // Byte blend with constant mask (E_V64QImode)
        __mmask64 mask_qi = 0xAAAAAAAAAAAAAAAAULL; // Alternating bytes
        __m512i v_qi_blend = _mm512_mask_blend_epi8(mask_qi, v_qi1, v_qi2);
        
        // Loop with varying masks
        __m512i v_qi_result = _mm512_setzero_si512();
        for (int i = 0; i < 8; i++) {
            __mmask64 dynamic_mask = (i % 2) ? 0xFFFFFFFFFFFFFFFFULL : 0x0000000000000000ULL;
            __m512i v_temp = _mm512_mask_blend_epi8(dynamic_mask, v_qi_blend, v_qi1);
            // Prevent optimization
            asm volatile("" : "+v"(v_temp));
            v_qi_result = _mm512_add_epi8(v_qi_result, v_temp);
        }
        _mm512_store_si512(dst_qi, v_qi_result);
        
        // Word blend (E_V32HImode)
        __m512i v_hi1 = _mm512_load_si512(src1_hi);
        __m512i v_hi2 = _mm512_load_si512(src2_hi);
        __mmask32 mask_hi = 0xAAAAAAAA; // Alternating words
        __m512i v_hi_blend = _mm512_mask_blend_epi16(mask_hi, v_hi1, v_hi2);
        
        // Multi-stage processing
        __m512i v_hi_shift = _mm512_slli_epi16(v_hi_blend, 1);
        __m512i v_hi_result = _mm512_mask_blend_epi16(0x55555555, v_hi_shift, v_hi1);
        _mm512_store_si512(dst_hi, v_hi_result);
    }
    
    // Compute checksums
    int32_t checksum_qi = 0;
    int32_t checksum_hi = 0;
    for (int i = 0; i < 64; i++) checksum_qi += dst_qi[i];
    for (int i = 0; i < 32; i++) checksum_hi += dst_hi[i];
    
    printf("AVX512BW Checksums: QI=%d, HI=%d\n", checksum_qi, checksum_hi);
}
#endif

#ifdef __AVX512BW__
// Half-precision float (HF) - E_V32HFmode
void test_avx512hf() {
    alignas(64) uint16_t src1_hf[32];
    alignas(64) uint16_t src2_hf[32];
    alignas(64) uint16_t dst_hf[32];
    
    // Initialize half-precision patterns
    for (int i = 0; i < 32; i++) {
        src1_hf[i] = 0x3C00 | (i & 0x7FF); // ~1.0 with variations
        src2_hf[i] = 0x4000 | ((31 - i) & 0x7FF); // ~2.0 with variations
    }
    
    __m512i v_hf1 = _mm512_load_si512(src1_hf);
    __m512i v_hf2 = _mm512_load_si512(src2_hf);
    
    // Use __m512h if available, otherwise cast
    #ifdef __AVX512FP16__
        __m512h v1 = _mm512_castsi512_ph(v_hf1);
        __m512h v2 = _mm512_castsi512_ph(v_hf2);
        __mmask32 mask_hf = 0xAAAAAAAA; // Alternating pattern
        __m512h v_blend = _mm512_mask_blend_ph(mask_hf, v1, v2);
        __m512i v_result = _mm512_castph_si512(v_blend);
    #else
        // Simulate blend using integer operations
        __mmask32 mask_hf = 0xAAAAAAAA;
        __m512i v_result = _mm512_mask_blend_epi16(mask_hf, v_hf1, v_hf2);
    #endif
    
    _mm512_store_si512(dst_hf, v_result);
    
    // Checksum
    uint32_t checksum_hf = 0;
    for (int i = 0; i < 32; i++) checksum_hf += dst_hf[i];
    printf("AVX512HF Checksum: %u\n", checksum_hf);
}
#endif

#ifdef __AVX512BF16__
// Brain float (BF16) - E_V32BFmode
void test_avx512bf16() {
    alignas(64) uint16_t src1_bf[32];
    alignas(64) uint16_t src2_bf[32];
    alignas(64) uint16_t dst_bf[32];
    
    // Initialize bfloat16 patterns
    for (int i = 0; i < 32; i++) {
        src1_bf[i] = 0x3F80 | (i & 0x7F); // ~1.0 with variations
        src2_bf[i] = 0x4000 | ((31 - i) & 0x7F); // ~2.0 with variations
    }
    
    __m512i v_bf1 = _mm512_load_si512(src1_bf);
    __m512i v_bf2 = _mm512_load_si512(src2_bf);
    
    // Use __m512bh if available
    #ifdef __AVX512BF16__
        __m512bh v1 = _mm512_castsi512_pbh(v_bf1);
        __m512bh v2 = _mm512_castsi512_pbh(v_bf2);
        __mmask32 mask_bf = 0x55555555; // Alternating pattern (different from HF)
        __m512bh v_blend = _mm512_mask_blend_epi16(mask_bf, v1, v2);
        __m512i v_result = _mm512_castpbh_si512(v_blend);
    #else
        // Fallback to integer blend
        __mmask32 mask_bf = 0x55555555;
        __m512i v_result = _mm512_mask_blend_epi16(mask_bf, v_bf1, v_bf2);
    #endif
    
    _mm512_store_si512(dst_bf, v_result);
    
    // Multi-stage pipeline with loop
    for (int i = 0; i < 4; i++) {
        __m512i v_temp = _mm512_load_si512(dst_bf);
        __mmask32 dynamic_mask = (i % 2) ? 0xFFFFFFFF : 0x00000000;
        __m512i v_new = _mm512_mask_blend_epi16(dynamic_mask, v_temp, v_bf1);
        // Force materialization
        asm volatile("" : "+v"(v_new));
        _mm512_store_si512(dst_bf, v_new);
    }
    
    // Checksum
    uint32_t checksum_bf = 0;
    for (int i = 0; i < 32; i++) checksum_bf += dst_bf[i];
    printf("AVX512BF16 Checksum: %u\n", checksum_bf);
}
#endif

int main() {
    printf("Testing AVX-512 Blend Expansions\n");
    printf("===============================\n");
    
    #ifdef __AVX512F__
    test_avx512f();
    #endif
    
    #ifdef __AVX512BW__
    test_avx512bw();
    test_avx512hf();
    #endif
    
    #ifdef __AVX512BF16__
    test_avx512bf16();
    #endif
    
    #if !defined(__AVX512F__) && !defined(__AVX512BW__) && !defined(__AVX512BF16__)
    printf("No AVX-512 extensions detected. Compile with -mavx512f -mavx512bw -mavx512bf16\n");
    #endif
    
    return 0;
}
