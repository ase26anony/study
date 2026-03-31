#include <immintrin.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

// Initialize arrays with distinct patterns
void init_arrays(void) {
    // Arrays will be initialized in main to avoid optimization
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

int main(void) {
    // Initialize source arrays with distinct patterns
    alignas(64) uint8_t src8_a[64], src8_b[64];
    alignas(64) uint16_t src16_a[32], src16_b[32];
    alignas(64) int32_t src32_a[16], src32_b[16];
    alignas(64) int64_t src64_a[8], src64_b[8];
    alignas(64) float srcf_a[16], srcf_b[16];
    alignas(64) double srcd_a[8], srcd_b[8];
    
    // Initialize with patterns
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
        srcf_a[i] = i * 1.5f;
        srcf_b[i] = i * 2.5f;
    }
    for (int i = 0; i < 8; i++) {
        src64_a[i] = i * 1000LL;
        src64_b[i] = -i * 500LL;
        srcd_a[i] = i * 3.14159;
        srcd_b[i] = i * 2.71828;
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
        __m512i va = _mm512_load_si512((const __m512i*)src8_a);
        __m512i vb = _mm512_load_si512((const __m512i*)src8_b);
        
        // Constant mask blend
        __mmask64 mask64_const = 0xAAAAAAAAAAAAAAAAULL;
        __m512i vc = _mm512_mask_blend_epi8(mask64_const, va, vb);
        
        // Loop-based blend with varying masks
        __m512i vresult = _mm512_setzero_si512();
        for (int i = 0; i < 4; i++) {
            __mmask64 mask = (i % 2) ? 0xFFFFFFFFFFFFFFFFULL : 0x5555555555555555ULL;
            vresult = _mm512_mask_blend_epi8(mask, vc, vb);
            
            // Multi-stage pipeline
            __m512i temp = _mm512_add_epi8(vresult, _mm512_set1_epi8(1));
            vresult = _mm512_mask_blend_epi8(mask64_const, vresult, temp);
        }
        
        _mm512_store_si512((__m512i*)dst8, vresult);
        
        // Force materialization with inline assembly
        asm volatile ("" : "+v"(vresult) : : "memory");
        
        total_checksum += checksum_512i(&vresult, 1);
    }
    
    // ==================== V32HImode (32x short) ====================
    {
        __m512i va = _mm512_load_si512((const __m512i*)src16_a);
        __m512i vb = _mm512_load_si512((const __m512i*)src16_b);
        
        __mmask32 mask32_const = 0xAAAAAAAA;
        __m512i vc = _mm512_mask_blend_epi16(mask32_const, va, vb);
        
        // Multi-stage processing
        __m512i vresult = vc;
        for (int i = 0; i < 3; i++) {
            __mmask32 mask = (i % 3) ? 0xFFFFFFFF : 0x55555555;
            __m512i temp = _mm512_add_epi16(vresult, _mm512_set1_epi16(i));
            vresult = _mm512_mask_blend_epi16(mask, vresult, temp);
        }
        
        _mm512_store_si512((__m512i*)dst16, vresult);
        asm volatile ("" : "+v"(vresult) : : "memory");
        
        total_checksum += checksum_512i(&vresult, 1);
    }
    
#ifdef __AVX512FP16__
    // ==================== V32HFmode (32x half-float) ====================
    {
        // Use __m512i with casting for half-precision
        __m512i va_hf = _mm512_load_si512((const __m512i*)src16_a);
        __m512i vb_hf = _mm512_load_si512((const __m512i*)src16_b);
        
        __mmask32 mask32_const = 0xAAAAAAAA;
        
        // Cast to half-precision if supported
        #ifdef __AVX512FP16__
        __m512h vh_a = _mm512_castsi512_ph(va_hf);
        __m512h vh_b = _mm512_castsi512_ph(vb_hf);
        __m512h vh_result = _mm512_mask_blend_ph(mask32_const, vh_a, vh_b);
        
        // Loop with varying masks
        for (int i = 0; i < 2; i++) {
            __mmask32 mask = (i % 2) ? 0xFFFFFFFF : 0x55555555;
            vh_result = _mm512_mask_blend_ph(mask, vh_result, vh_b);
        }
        
        __m512i vresult = _mm512_castph_si512(vh_result);
        #else
        // Fallback: use integer blend
        __m512i vresult = _mm512_mask_blend_epi16(mask32_const, va_hf, vb_hf);
        #endif
        
        _mm512_store_si512((__m512i*)dst16, vresult);
        asm volatile ("" : "+v"(vresult) : : "memory");
        
        total_checksum += checksum_512i(&vresult, 1);
    }
#endif // __AVX512FP16__
#endif // __AVX512BW__

#ifdef __AVX512F__
    // ==================== V16SImode (16x int) ====================
    {
        __m512i va = _mm512_load_si512((const __m512i*)src32_a);
        __m512i vb = _mm512_load_si512((const __m512i*)src32_b);
        
        __mmask16 mask16_const = 0xAAAA;
        __m512i vc = _mm512_mask_blend_epi32(mask16_const, va, vb);
        
        // Pipeline with arithmetic
        __m512i vresult = vc;
        for (int i = 0; i < 4; i++) {
            __mmask16 mask = (i % 2) ? 0xFFFF : 0x5555;
            __m512i temp = _mm512_add_epi32(vresult, _mm512_set1_epi32(i * 10));
            vresult = _mm512_mask_blend_epi32(mask, vresult, temp);
        }
        
        _mm512_store_si512((__m512i*)dst32, vresult);
        asm volatile ("" : "+v"(vresult) : : "memory");
        
        total_checksum += checksum_512i(&vresult, 1);
    }
    
    // ==================== V8DImode (8x long) ====================
    {
        __m512i va = _mm512_load_si512((const __m512i*)src64_a);
        __m512i vb = _mm512_load_si512((const __m512i*)src64_b);
        
        __mmask8 mask8_const = 0xAA;
        __m512i vc = _mm512_mask_blend_epi64(mask8_const, va, vb);
        
        // Multi-stage processing
        __m512i vresult = vc;
        for (int i = 0; i < 3; i++) {
            __mmask8 mask = (i % 3) ? 0xFF : 0x55;
            __m512i temp = _mm512_add_epi64(vresult, _mm512_set1_epi64(i * 100LL));
            vresult = _mm512_mask_blend_epi64(mask, vresult, temp);
        }
        
        _mm512_store_si512((__m512i*)dst64, vresult);
        asm volatile ("" : "+v"(vresult) : : "memory");
        
        total_checksum += checksum_512i(&vresult, 1);
    }
    
    // ==================== V16SFmode (16x float) ====================
    {
        __m512 va = _mm512_load_ps(srcf_a);
        __m512 vb = _mm512_load_ps(srcf_b);
        
        __mmask16 mask16_const = 0xAAAA;
        __m512 vc = _mm512_mask_blend_ps(mask16_const, va, vb);
        
        // Computational kernel with blends
        __m512 vresult = vc;
        for (int i = 0; i < 5; i++) {
            __mmask16 mask = (i % 2) ? 0xFFFF : 0x5555;
            __m512 temp = _mm512_add_ps(vresult, _mm512_set1_ps(i * 0.5f));
            vresult = _mm512_mask_blend_ps(mask, vresult, temp);
            
            // Additional blend in pipeline
            temp = _mm512_mul_ps(vresult, _mm512_set1_ps(1.1f));
            vresult = _mm512_mask_blend_ps(mask16_const, vresult, temp);
        }
        
        _mm512_store_ps(dstf, vresult);
        asm volatile ("" : "+v"(vresult) : : "memory");
        
        total_checksum += (uint64_t)checksum_512f(&vresult, 1);
    }
    
    // ==================== V8DFmode (8x double) ====================
    {
        __m512d va = _mm512_load_pd(srcd_a);
        __m512d vb = _mm512_load_pd(srcd_b);
        
        __mmask8 mask8_const = 0xAA;
        __m512d vc = _mm512_mask_blend_pd(mask8_const, va, vb);
        
        // Filter-like pipeline
        __m512d vresult = vc;
        for (int i = 0; i < 4; i++) {
            __mmask8 mask = (i % 3) ? 0xFF : 0x55;
            __m512d temp = _mm512_add_pd(vresult, _mm512_set1_pd(i * 0.25));
            vresult = _mm512_mask_blend_pd(mask, vresult, temp);
            
            // Second blend stage
            temp = _mm512_mul_pd(vresult, _mm512_set1_pd(1.05));
            vresult = _mm512_mask_blend_pd(mask8_const, vresult, temp);
        }
        
        _mm512_store_pd(dstd, vresult);
        asm volatile ("" : "+v"(vresult) : : "memory");
        
        total_checksum += (uint64_t)checksum_512d(&vresult, 1);
    }
#endif // __AVX512F__

#ifdef __AVX512BF16__
    // ==================== V32BFmode (32x brain-float) ====================
    {
        // Use integer arrays as source for BF16
        alignas(64) uint16_t srcbf_a[32], srcbf_b[32];
        for (int i = 0; i < 32; i++) {
            srcbf_a[i] = i * 100;
            srcbf_b[i] = 2000 - i * 50;
        }
        
        __m512i va_bf = _mm512_load_si512((const __m512i*)srcbf_a);
        __m512i vb_bf = _mm512_load_si512((const __m512i*)srcbf_b);
        
        __mmask32 mask32_const = 0xAAAAAAAA;
        
        #ifdef __AVX512BF16__
        // Cast to brain-float if supported
        __m512bh vbf_a = _mm512_castsi512_pbh(va_bf);
        __m512bh vbf_b = _mm512_castsi512_pbh(vb_bf);
        
        // Use appropriate blend intrinsic for BF16
        __m512bh vbf_result = _mm512_mask_blend_epi16(mask32_const, vbf_a, vbf_b);
        
        // Loop with data-dependent masks
        for (int i = 0; i < 3; i++) {
            __mmask32 mask = (i % 2) ? 0xFFFFFFFF : 0x55555555;
            vbf_result = _mm512_mask_blend_epi16(mask, vbf_result, vbf_b);
        }
        
        __m512i vresult = _mm512_castpbh_si512(vbf_result);
        #else
        // Fallback to integer blend
        __m512i vresult = _mm512_mask_blend_epi16(mask32_const, va_bf, vb_bf);
        #endif
        
        alignas(64) uint16_t dstbf[32];
        _mm512_store_si512((__m512i*)dstbf, vresult);
        asm volatile ("" : "+v"(vresult) : : "memory");
        
        total_checksum += checksum_512i(&vresult, 1);
    }
#endif // __AVX512BF16__

    // Final output to prevent optimization
    printf("Total checksum: %lu\n", total_checksum);
    
    // Use results to prevent dead code elimination
    volatile uint8_t sink8 = dst8[0];
    volatile uint16_t sink16 = dst16[0];
    volatile int32_t sink32 = dst32[0];
    volatile int64_t sink64 = dst64[0];
    volatile float sinkf = dstf[0];
    volatile double sinkd = dstd[0];
    
    (void)sink8; (void)sink16; (void)sink32;
    (void)sink64; (void)sinkf; (void)sinkd;
    
    return 0;
}
