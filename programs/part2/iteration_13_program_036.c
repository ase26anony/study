#include <immintrin.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

// Initialize arrays with distinct patterns
void init_arrays(void) {
    // Arrays will be initialized in main to avoid optimization issues
}

// Simple checksum function to prevent optimization
uint64_t compute_checksum(const void* data, size_t size) {
    uint64_t sum = 0;
    const uint8_t* ptr = (const uint8_t*)data;
    for (size_t i = 0; i < size; i++) {
        sum += ptr[i];
    }
    return sum;
}

int main(void) {
    // Initialize source arrays with distinct patterns
    alignas(64) int8_t   src8_a[64],   src8_b[64],   dst8[64];
    alignas(64) int16_t  src16_a[32],  src16_b[32],  dst16[32];
    alignas(64) int32_t  src32_a[16],  src32_b[16],  dst32[16];
    alignas(64) int64_t  src64_a[8],   src64_b[8],   dst64[8];
    alignas(64) float    srcf_a[16],   srcf_b[16],   dstf[16];
    alignas(64) double   srcd_a[8],    srcd_b[8],    dstd[8];
    
    // Initialize with predictable patterns
    for (int i = 0; i < 64; i++) {
        src8_a[i] = i;
        src8_b[i] = 64 - i;
        if (i < 64) dst8[i] = 0;
    }
    for (int i = 0; i < 32; i++) {
        src16_a[i] = i * 2;
        src16_b[i] = 1000 - i * 3;
        dst16[i] = 0;
    }
    for (int i = 0; i < 16; i++) {
        src32_a[i] = i * 100;
        src32_b[i] = 5000 - i * 200;
        dst32[i] = 0;
    }
    for (int i = 0; i < 8; i++) {
        src64_a[i] = i * 1000LL;
        src64_b[i] = 1000000LL - i * 5000LL;
        dst64[i] = 0;
    }
    for (int i = 0; i < 16; i++) {
        srcf_a[i] = i * 1.5f;
        srcf_b[i] = 100.0f - i * 2.5f;
        dstf[i] = 0.0f;
    }
    for (int i = 0; i < 8; i++) {
        srcd_a[i] = i * 3.14159;
        srcd_b[i] = 200.0 - i * 6.28318;
        dstd[i] = 0.0;
    }
    
    uint64_t total_checksum = 0;
    
#ifdef __AVX512F__
    printf("AVX512F blend tests\n");
    
    // V16SImode (16x int32) - E_V16SImode
    {
        __m512i va = _mm512_load_si512((const __m512i*)src32_a);
        __m512i vb = _mm512_load_si512((const __m512i*)src32_b);
        
        // Constant mask pattern
        __mmask16 mask_const = 0xAAAA;  // Alternating bits
        
        // Loop with varying masks to prevent optimization
        for (int iter = 0; iter < 4; iter++) {
            __mmask16 mask = (iter % 2) ? mask_const : ~mask_const;
            __m512i result = _mm512_mask_blend_epi32(mask, va, vb);
            
            // Multi-stage pipeline: blend -> arithmetic -> blend
            __m512i temp = _mm512_add_epi32(result, _mm512_set1_epi32(iter));
            result = _mm512_mask_blend_epi32(0x5555, temp, va);  // Different mask
            
            // Force materialization with inline assembly
            asm volatile("" : "+v"(result) : : "memory");
            
            _mm512_store_si512((__m512i*)dst32, result);
        }
        
        total_checksum += compute_checksum(dst32, sizeof(dst32));
    }
    
    // V8DImode (8x int64) - E_V8DImode
    {
        __m512i va = _mm512_load_si512((const __m512i*)src64_a);
        __m512i vb = _mm512_load_si512((const __m512i*)src64_b);
        
        __mmask8 mask_const = 0xAA;  // Alternating bits
        
        for (int iter = 0; iter < 3; iter++) {
            __mmask8 mask = (iter % 3 == 0) ? mask_const : (0xFF >> iter);
            __m512i result = _mm512_mask_blend_epi64(mask, va, vb);
            
            // Chain operations
            __m512i shifted = _mm512_slli_epi64(result, 1);
            result = _mm512_mask_blend_epi64(0x55, shifted, vb);
            
            asm volatile("" : "+v"(result) : : "memory");
            
            _mm512_store_si512((__m512i*)dst64, result);
        }
        
        total_checksum += compute_checksum(dst64, sizeof(dst64));
    }
    
    // V16SFmode (16x float) - E_V16SFmode
    {
        __m512 va = _mm512_load_ps(srcf_a);
        __m512 vb = _mm512_load_ps(srcf_b);
        
        __mmask16 mask_const = 0xCCCC;  // Checkerboard pattern
        
        for (int iter = 0; iter < 4; iter++) {
            __mmask16 mask = (iter & 1) ? mask_const : (mask_const ^ 0xFFFF);
            __m512 result = _mm512_mask_blend_ps(mask, va, vb);
            
            // Multi-stage processing
            __m512 scaled = _mm512_mul_ps(result, _mm512_set1_ps(1.1f));
            result = _mm512_mask_blend_ps(0x3333, scaled, va);
            
            asm volatile("" : "+v"(result) : : "memory");
            
            _mm512_store_ps(dstf, result);
        }
        
        total_checksum += compute_checksum(dstf, sizeof(dstf));
    }
    
    // V8DFmode (8x double) - E_V8DFmode
    {
        __m512d va = _mm512_load_pd(srcd_a);
        __m512d vb = _mm512_load_pd(srcd_b);
        
        __mmask8 mask_const = 0x99;  // Pattern
        
        for (int iter = 0; iter < 3; iter++) {
            __mmask8 mask = (iter % 2) ? mask_const : ~mask_const;
            __m512d result = _mm512_mask_blend_pd(mask, va, vb);
            
            // Pipeline with arithmetic
            __m512d adjusted = _mm512_add_pd(result, _mm512_set1_pd(0.5));
            result = _mm512_mask_blend_pd(0x66, adjusted, vb);
            
            asm volatile("" : "+v"(result) : : "memory");
            
            _mm512_store_pd(dstd, result);
        }
        
        total_checksum += compute_checksum(dstd, sizeof(dstd));
    }
#endif  // __AVX512F__

#ifdef __AVX512BW__
    printf("AVX512BW blend tests\n");
    
    // V64QImode (64x int8) - E_V64QImode
    {
        __m512i va = _mm512_load_si512((const __m512i*)src8_a);
        __m512i vb = _mm512_load_si512((const __m512i*)src8_b);
        
        __mmask64 mask_const = 0xAAAAAAAAAAAAAAAAULL;  // Alternating bytes
        
        for (int iter = 0; iter < 4; iter++) {
            __mmask64 mask = (iter % 3) ? mask_const : (mask_const ^ 0xFFFFFFFFFFFFFFFFULL);
            __m512i result = _mm512_mask_blend_epi8(mask, va, vb);
            
            // Multi-stage pipeline
            __m512i added = _mm512_add_epi8(result, _mm512_set1_epi8(iter));
            result = _mm512_mask_blend_epi8(0x5555555555555555ULL, added, vb);
            
            asm volatile("" : "+v"(result) : : "memory");
            
            _mm512_store_si512((__m512i*)dst8, result);
        }
        
        total_checksum += compute_checksum(dst8, sizeof(dst8));
    }
    
    // V32HImode (32x int16) - E_V32HImode
    {
        __m512i va = _mm512_load_si512((const __m512i*)src16_a);
        __m512i vb = _mm512_load_si512((const __m512i*)src16_b);
        
        __mmask32 mask_const = 0xAAAAAAAA;  // Alternating words
        
        for (int iter = 0; iter < 4; iter++) {
            __mmask32 mask = (iter & 1) ? mask_const : (mask_const ^ 0xFFFFFFFF);
            __m512i result = _mm512_mask_blend_epi16(mask, va, vb);
            
            // Chain operations
            __m512i shifted = _mm512_slli_epi16(result, 1);
            result = _mm512_mask_blend_epi16(0x55555555, shifted, va);
            
            asm volatile("" : "+v"(result) : : "memory");
            
            _mm512_store_si512((__m512i*)dst16, result);
        }
        
        total_checksum += compute_checksum(dst16, sizeof(dst16));
    }
    
#ifdef __AVX512FP16__
    // V32HFmode (32x half-float) - E_V32HFmode
    {
        // Use __m512i for storage, cast to __m512h
        alignas(64) uint16_t hf_a[32], hf_b[32], hf_dst[32];
        for (int i = 0; i < 32; i++) {
            hf_a[i] = i * 0x0400;  // Simple half-float pattern
            hf_b[i] = 0x3C00 - i * 0x0200;  // Another pattern
        }
        
        __m512i va_i = _mm512_load_si512((const __m512i*)hf_a);
        __m512i vb_i = _mm512_load_si512((const __m512i*)hf_b);
        
        // Cast to half-precision vectors
        __m512h va = _mm512_castsi512_ph(va_i);
        __m512h vb = _mm512_castsi512_ph(vb_i);
        
        __mmask32 mask_const = 0xCCCCCCCC;  // Checkerboard
        
        for (int iter = 0; iter < 3; iter++) {
            __mmask32 mask = (iter % 2) ? mask_const : ~mask_const;
            
            // Use appropriate intrinsic - note: _mm512_mask_blend_ph may not exist
            // Use integer blend as fallback for half precision
            __m512h result_h = _mm512_castsi512_ph(
                _mm512_mask_blend_epi16(mask, 
                    _mm512_castph_si512(va),
                    _mm512_castph_si512(vb)));
            
            // Cast back for storage
            __m512i result_i = _mm512_castph_si512(result_h);
            asm volatile("" : "+v"(result_i) : : "memory");
            
            _mm512_store_si512((__m512i*)hf_dst, result_i);
        }
        
        total_checksum += compute_checksum(hf_dst, sizeof(hf_dst));
    }
#endif  // __AVX512FP16__
#endif  // __AVX512BW__

#ifdef __AVX512BF16__
    printf("AVX512BF16 blend tests\n");
    
    // V32BFmode (32x brain-float) - E_V32BFmode
    {
        // Brain float data
        alignas(64) uint16_t bf_a[32], bf_b[32], bf_dst[32];
        for (int i = 0; i < 32; i++) {
            bf_a[i] = i * 0x0200;  // Simple bfloat16 pattern
            bf_b[i] = 0x4000 - i * 0x0100;
        }
        
        __m512i va_i = _mm512_load_si512((const __m512i*)bf_a);
        __m512i vb_i = _mm512_load_si512((const __m512i*)bf_b);
        
        // Cast to brain-float vectors if supported
        #ifdef __AVX512BF16__
        __m512bh va = _mm512_castsi512_pbh(va_i);
        __m512bh vb = _mm512_castsi512_pbh(vb_i);
        
        __mmask32 mask_const = 0xF0F0F0F0;  // Pattern
        
        for (int iter = 0; iter < 3; iter++) {
            __mmask32 mask = (iter % 3 == 0) ? mask_const : (mask_const ^ 0xFFFFFFFF);
            
            // Use integer blend for brain float (similar approach to half-float)
            __m512bh result_bh = _mm512_castsi512_pbh(
                _mm512_mask_blend_epi16(mask,
                    _mm512_castpbh_si512(va),
                    _mm512_castpbh_si512(vb)));
            
            __m512i result_i = _mm512_castpbh_si512(result_bh);
            asm volatile("" : "+v"(result_i) : : "memory");
            
            _mm512_store_si512((__m512i*)bf_dst, result_i);
        }
        #endif
        
        total_checksum += compute_checksum(bf_dst, sizeof(bf_dst));
    }
#endif  // __AVX512BF16__

    printf("Total checksum: %lu\n", total_checksum);
    return 0;
}
