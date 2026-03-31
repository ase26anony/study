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
uint64_t checksum_512i(const __m512i* vec, size_t count) {
    uint64_t sum = 0;
    const uint8_t* data = (const uint8_t*)vec;
    for (size_t i = 0; i < count * 64; i++) {
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

int main() {
    // Initialize source arrays with distinct patterns
    alignas(64) uint8_t src8_a[64], src8_b[64];
    alignas(64) uint16_t src16_a[32], src16_b[32];
    alignas(64) int32_t src32_a[16], src32_b[16];
    alignas(64) int64_t src64_a[8], src64_b[8];
    alignas(64) float srcf_a[16], srcf_b[16];
    alignas(64) double srcd_a[8], srcd_b[8];
    
    // Initialize with predictable patterns
    for (int i = 0; i < 64; i++) {
        src8_a[i] = i;
        src8_b[i] = 64 - i;
    }
    for (int i = 0; i < 32; i++) {
        src16_a[i] = i * 2;
        src16_b[i] = 1000 - i * 3;
    }
    for (int i = 0; i < 16; i++) {
        src32_a[i] = i * 100;
        src32_b[i] = -i * 50;
    }
    for (int i = 0; i < 8; i++) {
        src64_a[i] = i * 1000LL;
        src64_b[i] = -i * 500LL;
    }
    for (int i = 0; i < 16; i++) {
        srcf_a[i] = i * 1.5f;
        srcf_b[i] = 10.0f - i * 0.5f;
    }
    for (int i = 0; i < 8; i++) {
        srcd_a[i] = i * 2.5;
        srcd_b[i] = 20.0 - i * 1.0;
    }
    
    // Destination arrays
    alignas(64) uint8_t dst8[64];
    alignas(64) uint16_t dst16[32];
    alignas(64) int32_t dst32[16];
    alignas(64) int64_t dst64[8];
    alignas(64) float dstf[16];
    alignas(64) double dstd[8];
    
    uint64_t total_checksum = 0;
    
#ifdef __AVX512BW__
    // ==================== V64QImode (64x char) ====================
    {
        __m512i vec_a = _mm512_load_si512((const __m512i*)src8_a);
        __m512i vec_b = _mm512_load_si512((const __m512i*)src8_b);
        
        // Constant mask blend
        __m512i result = _mm512_mask_blend_epi8(
            0xAAAAAAAAAAAAAAAAULL,  // Alternating pattern
            vec_a, vec_b);
        
        // Loop-based blend with varying masks
        __m512i accum = _mm512_setzero_si512();
        for (int i = 0; i < 4; i++) {
            __mmask64 mask = (i % 2) ? 0xFFFFFFFFFFFFFFFFULL : 0xAAAAAAAAAAAAAAAAULL;
            accum = _mm512_mask_blend_epi8(mask, accum, result);
            
            // Multi-stage pipeline
            __m512i temp = _mm512_add_epi8(accum, _mm512_set1_epi8(1));
            accum = _mm512_mask_blend_epi8(0x5555555555555555ULL, accum, temp);
        }
        
        _mm512_store_si512((__m512i*)dst8, accum);
        
        // Force materialization with inline assembly
        asm volatile("" : "+v"(accum) : : "memory");
        
        total_checksum += checksum_512i(&accum, 1);
    }
    
    // ==================== V32HImode (32x short) ====================
    {
        __m512i vec_a = _mm512_load_si512((const __m512i*)src16_a);
        __m512i vec_b = _mm512_load_si512((const __m512i*)src16_b);
        
        // Constant mask blend
        __m512i result = _mm512_mask_blend_epi16(
            0xAAAAAAAA,  // Alternating pattern
            vec_a, vec_b);
        
        // Loop-based blend with varying masks
        __m512i accum = _mm512_setzero_si512();
        for (int i = 0; i < 8; i++) {
            __mmask32 mask = (i % 3) ? 0xFFFFFFFF : 0xAAAAAAAA;
            accum = _mm512_mask_blend_epi16(mask, accum, result);
            
            // Multi-stage pipeline
            __m512i temp = _mm512_add_epi16(accum, _mm512_set1_epi16(i));
            accum = _mm512_mask_blend_epi16(0x55555555, accum, temp);
        }
        
        _mm512_store_si512((__m512i*)dst16, accum);
        
        // Force materialization
        asm volatile("" : "+v"(accum) : : "memory");
        
        total_checksum += checksum_512i(&accum, 1);
    }
#endif // __AVX512BW__

#ifdef __AVX512F__
    // ==================== V16SImode (16x int) ====================
    {
        __m512i vec_a = _mm512_load_si512((const __m512i*)src32_a);
        __m512i vec_b = _mm512_load_si512((const __m512i*)src32_b);
        
        // Constant mask blend
        __m512i result = _mm512_mask_blend_epi32(
            0xAAAA,  // Alternating pattern
            vec_a, vec_b);
        
        // Loop-based blend with varying masks
        __m512i accum = _mm512_setzero_si512();
        for (int i = 0; i < 16; i++) {
            __mmask16 mask = (i % 4) ? 0xFFFF : 0xAAAA;
            accum = _mm512_mask_blend_epi32(mask, accum, result);
            
            // Multi-stage pipeline
            __m512i temp = _mm512_add_epi32(accum, _mm512_set1_epi32(i));
            accum = _mm512_mask_blend_epi32(0x5555, accum, temp);
        }
        
        _mm512_store_si512((__m512i*)dst32, accum);
        
        // Force materialization
        asm volatile("" : "+v"(accum) : : "memory");
        
        total_checksum += checksum_512i(&accum, 1);
    }
    
    // ==================== V8DImode (8x long) ====================
    {
        __m512i vec_a = _mm512_load_si512((const __m512i*)src64_a);
        __m512i vec_b = _mm512_load_si512((const __m512i*)src64_b);
        
        // Constant mask blend
        __m512i result = _mm512_mask_blend_epi64(
            0xAA,  // Alternating pattern
            vec_a, vec_b);
        
        // Loop-based blend with varying masks
        __m512i accum = _mm512_setzero_si512();
        for (int i = 0; i < 8; i++) {
            __mmask8 mask = (i % 2) ? 0xFF : 0xAA;
            accum = _mm512_mask_blend_epi64(mask, accum, result);
            
            // Multi-stage pipeline
            __m512i temp = _mm512_add_epi64(accum, _mm512_set1_epi64(i));
            accum = _mm512_mask_blend_epi64(0x55, accum, temp);
        }
        
        _mm512_store_si512((__m512i*)dst64, accum);
        
        // Force materialization
        asm volatile("" : "+v"(accum) : : "memory");
        
        total_checksum += checksum_512i(&accum, 1);
    }
    
    // ==================== V16SFmode (16x float) ====================
    {
        __m512 vec_a = _mm512_load_ps(srcf_a);
        __m512 vec_b = _mm512_load_ps(srcf_b);
        
        // Constant mask blend
        __m512 result = _mm512_mask_blend_ps(
            0xAAAA,  // Alternating pattern
            vec_a, vec_b);
        
        // Loop-based blend with varying masks
        __m512 accum = _mm512_setzero_ps();
        for (int i = 0; i < 16; i++) {
            __mmask16 mask = (i % 5) ? 0xFFFF : 0xAAAA;
            accum = _mm512_mask_blend_ps(mask, accum, result);
            
            // Multi-stage pipeline
            __m512 temp = _mm512_add_ps(accum, _mm512_set1_ps(i * 0.1f));
            accum = _mm512_mask_blend_ps(0x5555, accum, temp);
        }
        
        _mm512_store_ps(dstf, accum);
        
        // Force materialization
        asm volatile("" : "+v"(accum) : : "memory");
        
        total_checksum += (uint64_t)checksum_512f(&accum, 1);
    }
    
    // ==================== V8DFmode (8x double) ====================
    {
        __m512d vec_a = _mm512_load_pd(srcd_a);
        __m512d vec_b = _mm512_load_pd(srcd_b);
        
        // Constant mask blend
        __m512d result = _mm512_mask_blend_pd(
            0xAA,  // Alternating pattern
            vec_a, vec_b);
        
        // Loop-based blend with varying masks
        __m512d accum = _mm512_setzero_pd();
        for (int i = 0; i < 8; i++) {
            __mmask8 mask = (i % 3) ? 0xFF : 0xAA;
            accum = _mm512_mask_blend_pd(mask, accum, result);
            
            // Multi-stage pipeline
            __m512d temp = _mm512_add_pd(accum, _mm512_set1_pd(i * 0.01));
            accum = _mm512_mask_blend_pd(0x55, accum, temp);
        }
        
        _mm512_store_pd(dstd, accum);
        
        // Force materialization
        asm volatile("" : "+v"(accum) : : "memory");
        
        total_checksum += (uint64_t)checksum_512d(&accum, 1);
    }
#endif // __AVX512F__

#ifdef __AVX512BF16__
    // ==================== V32BFmode (32x brain-float) ====================
    {
        // Initialize brain-float data (using uint16_t as storage)
        alignas(64) uint16_t srcbf_a[32], srcbf_b[32];
        for (int i = 0; i < 32; i++) {
            srcbf_a[i] = i * 0x100;
            srcbf_b[i] = 0x7C00 - i * 0x80;  // Some bfloat16 patterns
        }
        
        __m512bh vec_a, vec_b;
        
        // Load using appropriate casting
        vec_a = _mm512_castsi512_pbh(_mm512_load_si512((const __m512i*)srcbf_a));
        vec_b = _mm512_castsi512_pbh(_mm512_load_si512((const __m512i*)srcbf_b));
        
        // Constant mask blend
        __m512bh result = _mm512_mask_blend_epi16(
            0xAAAAAAAA,  // Alternating pattern
            vec_a, vec_b);
        
        // Loop-based blend with varying masks
        __m512bh accum = _mm512_castsi512_pbh(_mm512_setzero_si512());
        for (int i = 0; i < 8; i++) {
            __mmask32 mask = (i % 3) ? 0xFFFFFFFF : 0xAAAAAAAA;
            accum = _mm512_mask_blend_epi16(mask, accum, result);
            
            // Multi-stage pipeline - need to convert to float for arithmetic
            __m512 accum_f = _mm512_cvtpbh_ps(accum);
            __m512 temp_f = _mm512_add_ps(accum_f, _mm512_set1_ps(i * 0.1f));
            __m512bh temp = _mm512_cvtne2ps_pbh(temp_f, temp_f);
            accum = _mm512_mask_blend_epi16(0x55555555, accum, temp);
        }
        
        // Store result
        _mm512_store_si512((__m512i*)srcbf_a, _mm512_castpbh_si512(accum));
        
        // Force materialization
        asm volatile("" : "+v"(accum) : : "memory");
        
        // Simple checksum for bfloat16
        uint64_t bf_sum = 0;
        for (int i = 0; i < 32; i++) {
            bf_sum += srcbf_a[i];
        }
        total_checksum += bf_sum;
    }
#endif // __AVX512BF16__

#ifdef __AVX512FP16__
    // ==================== V32HFmode (32x half-float) ====================
    {
        // Initialize half-float data
        alignas(64) uint16_t srchf_a[32], srchf_b[32];
        for (int i = 0; i < 32; i++) {
            srchf_a[i] = i * 0x0400;  // Simple half-float pattern
            srchf_b[i] = 0x3C00 - i * 0x0200;  // 1.0, 0.875, 0.75, etc.
        }
        
        __m512h vec_a, vec_b;
        
        // Load using appropriate casting
        vec_a = _mm512_castsi512_ph(_mm512_load_si512((const __m512i*)srchf_a));
        vec_b = _mm512_castsi512_ph(_mm512_load_si512((const __m512i*)srchf_b));
        
        // Constant mask blend
        __m512h result = _mm512_mask_blend_epi16(
            0xAAAAAAAA,  // Alternating pattern
            vec_a, vec_b);
        
        // Loop-based blend with varying masks
        __m512h accum = _mm512_castsi512_ph(_mm512_setzero_si512());
        for (int i = 0; i < 8; i++) {
            __mmask32 mask = (i % 3) ? 0xFFFFFFFF : 0xAAAAAAAA;
            accum = _mm512_mask_blend_epi16(mask, accum, result);
            
            // Multi-stage pipeline
            __m512h temp = _mm512_add_ph(accum, _mm512_set1_ph(_cvtsh_u16(i * 0.1f)));
            accum = _mm512_mask_blend_epi16(0x55555555, accum, temp);
        }
        
        // Store result
        _mm512_store_si512((__m512i*)srchf_a, _mm512_castph_si512(accum));
        
        // Force materialization
        asm volatile("" : "+v"(accum) : : "memory");
        
        // Simple checksum for half-float
        uint64_t hf_sum = 0;
        for (int i = 0; i < 32; i++) {
            hf_sum += srchf_a[i];
        }
        total_checksum += hf_sum;
    }
#endif // __AVX512FP16__

    // Print final checksum to prevent optimization
    printf("Total checksum: %lu\n", total_checksum);
    
    return 0;
}
