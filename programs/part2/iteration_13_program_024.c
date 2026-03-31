#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Constants for predictable patterns
#define PATTERN_A 0xAAAAAAAAAAAAAAAAULL
#define PATTERN_5 0x5555555555555555ULL
#define PATTERN_3 0x3333333333333333ULL
#define PATTERN_F 0xFFFFFFFFFFFFFFFFULL

// Initialize arrays with distinct patterns
void init_arrays(void) {
    // Arrays will be initialized in main to avoid static initialization issues
}

// Simple checksum function to prevent optimization
uint64_t checksum_512i(const __m512i* vec, size_t count) {
    uint64_t sum = 0;
    const uint64_t* data = (const uint64_t*)vec;
    for (size_t i = 0; i < count * 8; i++) {
        sum += data[i];
    }
    return sum;
}

float checksum_512f(const __m512* vec, size_t count) {
    float sum = 0.0f;
    const float* data = (const float*)vec;
    for (size_t i = 0; i < count * 16; i++) {
        sum += data[i];
    }
    return sum;
}

double checksum_512d(const __m512d* vec, size_t count) {
    double sum = 0.0;
    const double* data = (const double*)vec;
    for (size_t i = 0; i < count * 8; i++) {
        sum += data[i];
    }
    return sum;
}

int main(void) {
    uint64_t total_checksum = 0;
    
    // Initialize source arrays with predictable patterns
    alignas(64) uint8_t src8_a[64], src8_b[64];
    alignas(64) uint16_t src16_a[32], src16_b[32];
    alignas(64) uint32_t src32_a[16], src32_b[16];
    alignas(64) uint64_t src64_a[8], src64_b[8];
    alignas(64) float srcf_a[16], srcf_b[16];
    alignas(64) double srcd_a[8], srcd_b[8];
    
    for (int i = 0; i < 64; i++) {
        src8_a[i] = i;
        src8_b[i] = 64 - i;
        if (i < 32) {
            src16_a[i] = i * 2;
            src16_b[i] = 64 - i * 2;
        }
        if (i < 16) {
            src32_a[i] = i * 4;
            src32_b[i] = 64 - i * 4;
            srcf_a[i] = i * 0.5f;
            srcf_b[i] = 32.0f - i * 0.5f;
        }
        if (i < 8) {
            src64_a[i] = i * 8;
            src64_b[i] = 64 - i * 8;
            srcd_a[i] = i * 0.25;
            srcd_b[i] = 16.0 - i * 0.25;
        }
    }
    
#ifdef __AVX512BW__
    {
        // Test E_V64QImode (64x 8-bit integers)
        __m512i v64qi_a = _mm512_load_si512(src8_a);
        __m512i v64qi_b = _mm512_load_si512(src8_b);
        __m512i v64qi_result;
        
        // Multi-stage pipeline with varying masks
        for (int i = 0; i < 4; i++) {
            __mmask64 mask = (i % 2) ? PATTERN_A : PATTERN_5;
            v64qi_result = _mm512_mask_blend_epi8(mask, v64qi_a, v64qi_b);
            
            // Chain operations: blend -> arithmetic -> blend
            __m512i temp = _mm512_add_epi8(v64qi_result, _mm512_set1_epi8(1));
            __mmask64 mask2 = (i % 3) ? PATTERN_3 : PATTERN_F;
            v64qi_result = _mm512_mask_blend_epi8(mask2, v64qi_result, temp);
            
            // Force materialization with inline assembly
            asm volatile("" : "+v"(v64qi_result) : : "memory");
        }
        
        alignas(64) uint8_t dst64qi[64];
        _mm512_store_si512(dst64qi, v64qi_result);
        for (int i = 0; i < 64; i++) total_checksum += dst64qi[i];
        
        // Test E_V32HImode (32x 16-bit integers)
        __m512i v32hi_a = _mm512_load_si512(src16_a);
        __m512i v32hi_b = _mm512_load_si512(src16_b);
        __m512i v32hi_result;
        
        for (int i = 0; i < 4; i++) {
            __mmask32 mask = (i % 2) ? PATTERN_A : PATTERN_5;
            v32hi_result = _mm512_mask_blend_epi16(mask, v32hi_a, v32hi_b);
            
            // Data-dependent mask generation
            __mmask32 dynamic_mask = 0;
            for (int j = 0; j < 32; j++) {
                if ((i + j) % 3 == 0) dynamic_mask |= (1ULL << j);
            }
            __m512i temp = _mm512_slli_epi16(v32hi_result, 1);
            v32hi_result = _mm512_mask_blend_epi16(dynamic_mask, v32hi_result, temp);
            
            asm volatile("" : "+v"(v32hi_result) : : "memory");
        }
        
        alignas(64) uint16_t dst32hi[32];
        _mm512_store_si512(dst32hi, v32hi_result);
        for (int i = 0; i < 32; i++) total_checksum += dst32hi[i];
    }
#endif

#ifdef __AVX512F__
    {
        // Test E_V16SImode (16x 32-bit integers)
        __m512i v16si_a = _mm512_load_si512(src32_a);
        __m512i v16si_b = _mm512_load_si512(src32_b);
        __m512i v16si_result;
        
        for (int i = 0; i < 4; i++) {
            __mmask16 mask = (i % 2) ? PATTERN_A : PATTERN_5;
            v16si_result = _mm512_mask_blend_epi32(mask, v16si_a, v16si_b);
            
            // Multi-stage processing
            __m512i temp = _mm512_mullo_epi32(v16si_result, _mm512_set1_epi32(2));
            __mmask16 mask2 = (i % 4) ? PATTERN_3 : PATTERN_F;
            v16si_result = _mm512_mask_blend_epi32(mask2, v16si_result, temp);
            
            asm volatile("" : "+v"(v16si_result) : : "memory");
        }
        
        alignas(64) uint32_t dst16si[16];
        _mm512_store_si512(dst16si, v16si_result);
        for (int i = 0; i < 16; i++) total_checksum += dst16si[i];
        
        // Test E_V8DImode (8x 64-bit integers)
        __m512i v8di_a = _mm512_load_si512(src64_a);
        __m512i v8di_b = _mm512_load_si512(src64_b);
        __m512i v8di_result;
        
        for (int i = 0; i < 4; i++) {
            __mmask8 mask = (i % 2) ? 0xAA : 0x55;
            v8di_result = _mm512_mask_blend_epi64(mask, v8di_a, v8di_b);
            
            __m512i temp = _mm512_add_epi64(v8di_result, _mm512_set1_epi64(1));
            __mmask8 mask2 = (i % 3) ? 0x33 : 0xFF;
            v8di_result = _mm512_mask_blend_epi64(mask2, v8di_result, temp);
            
            asm volatile("" : "+v"(v8di_result) : : "memory");
        }
        
        alignas(64) uint64_t dst8di[8];
        _mm512_store_si512(dst8di, v8di_result);
        for (int i = 0; i < 8; i++) total_checksum += dst8di[i];
        
        // Test E_V16SFmode (16x single-precision floats)
        __m512 v16sf_a = _mm512_load_ps(srcf_a);
        __m512 v16sf_b = _mm512_load_ps(srcf_b);
        __m512 v16sf_result;
        
        for (int i = 0; i < 4; i++) {
            __mmask16 mask = (i % 2) ? PATTERN_A : PATTERN_5;
            v16sf_result = _mm512_mask_blend_ps(mask, v16sf_a, v16sf_b);
            
            __m512 temp = _mm512_mul_ps(v16sf_result, _mm512_set1_ps(1.5f));
            __mmask16 mask2 = (i % 4) ? PATTERN_3 : PATTERN_F;
            v16sf_result = _mm512_mask_blend_ps(mask2, v16sf_result, temp);
            
            asm volatile("" : "+v"(v16sf_result) : : "memory");
        }
        
        alignas(64) float dst16sf[16];
        _mm512_store_ps(dst16sf, v16sf_result);
        float sf_sum = 0;
        for (int i = 0; i < 16; i++) sf_sum += dst16sf[i];
        total_checksum += (uint64_t)sf_sum;
        
        // Test E_V8DFmode (8x double-precision floats)
        __m512d v8df_a = _mm512_load_pd(srcd_a);
        __m512d v8df_b = _mm512_load_pd(srcd_b);
        __m512d v8df_result;
        
        for (int i = 0; i < 4; i++) {
            __mmask8 mask = (i % 2) ? 0xAA : 0x55;
            v8df_result = _mm512_mask_blend_pd(mask, v8df_a, v8df_b);
            
            __m512d temp = _mm512_mul_pd(v8df_result, _mm512_set1_pd(1.25));
            __mmask8 mask2 = (i % 3) ? 0x33 : 0xFF;
            v8df_result = _mm512_mask_blend_pd(mask2, v8df_result, temp);
            
            asm volatile("" : "+v"(v8df_result) : : "memory");
        }
        
        alignas(64) double dst8df[8];
        _mm512_store_pd(dst8df, v8df_result);
        double df_sum = 0;
        for (int i = 0; i < 8; i++) df_sum += dst8df[i];
        total_checksum += (uint64_t)df_sum;
    }
#endif

#ifdef __AVX512BF16__
    {
        // For half-precision and brain-float, we need to use appropriate types
        // Initialize half-precision arrays
        alignas(64) uint16_t srchf_a[32], srchf_b[32];
        alignas(64) uint16_t srcbf_a[32], srcbf_b[32];
        
        for (int i = 0; i < 32; i++) {
            srchf_a[i] = i * 128;  // Simple pattern for half-float
            srchf_b[i] = 4096 - i * 128;
            srcbf_a[i] = i * 256;  // Simple pattern for brain-float
            srcbf_b[i] = 8192 - i * 256;
        }
        
        // Test E_V32HFmode (32x half-precision floats)
        // Use __m512i with casting for half-precision
        __m512i v32hf_a = _mm512_load_si512(srchf_a);
        __m512i v32hf_b = _mm512_load_si512(srchf_b);
        __m512i v32hf_result;
        
        for (int i = 0; i < 4; i++) {
            __mmask32 mask = (i % 2) ? PATTERN_A : PATTERN_5;
            // Cast to appropriate type for blend operation
            __m512h v32hf_a_cast = _mm512_castsi512_ph(v32hf_a);
            __m512h v32hf_b_cast = _mm512_castsi512_ph(v32hf_b);
            __m512h v32hf_result_cast;
            
            // Note: _mm512_mask_blend_ph doesn't exist, use epi16 equivalent
            // This should still trigger the V32HFmode case
            v32hf_result = _mm512_mask_blend_epi16(mask, v32hf_a, v32hf_b);
            
            asm volatile("" : "+v"(v32hf_result) : : "memory");
        }
        
        alignas(64) uint16_t dst32hf[32];
        _mm512_store_si512(dst32hf, v32hf_result);
        for (int i = 0; i < 32; i++) total_checksum += dst32hf[i];
        
        // Test E_V32BFmode (32x brain-float)
        __m512i v32bf_a = _mm512_load_si512(srcbf_a);
        __m512i v32bf_b = _mm512_load_si512(srcbf_b);
        __m512i v32bf_result;
        
        for (int i = 0; i < 4; i++) {
            __mmask32 mask = (i % 2) ? PATTERN_A : PATTERN_5;
            // Cast to brain-float type
            __m512bh v32bf_a_cast = _mm512_castsi512_pbh(v32bf_a);
            __m512bh v32bf_b_cast = _mm512_castsi512_pbh(v32bf_b);
            
            // Use epi16 blend for brain-float as well
            v32bf_result = _mm512_mask_blend_epi16(mask, v32bf_a, v32bf_b);
            
            asm volatile("" : "+v"(v32bf_result) : : "memory");
        }
        
        alignas(64) uint16_t dst32bf[32];
        _mm512_store_si512(dst32bf, v32bf_result);
        for (int i = 0; i < 32; i++) total_checksum += dst32bf[i];
    }
#endif

    printf("Total checksum: %lu\n", total_checksum);
    return 0;
}
