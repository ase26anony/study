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
        if (i < 32) {
            src16_a[i] = i * 2;
            src16_b[i] = 64 - i * 2;
        }
        if (i < 16) {
            src32_a[i] = i * 4;
            src32_b[i] = 256 - i * 4;
            srcf_a[i] = i * 0.5f;
            srcf_b[i] = 8.0f - i * 0.5f;
        }
        if (i < 8) {
            src64_a[i] = i * 8;
            src64_b[i] = 512 - i * 8;
            srcd_a[i] = i * 0.25;
            srcd_b[i] = 2.0 - i * 0.25;
        }
    }
    
    uint64_t total_checksum = 0;
    
#ifdef __AVX512F__
    // ==================== AVX-512F Cases ====================
    
    // E_V16SImode - 16x int32 blend
    {
        __m512i va = _mm512_load_si512((__m512i*)src32_a);
        __m512i vb = _mm512_load_si512((__m512i*)src32_b);
        
        // Constant mask blend
        __mmask16 mask_const = 0xAAAA; // Alternating bits
        __m512i vc = _mm512_mask_blend_epi32(mask_const, va, vb);
        
        // Loop-based blend with varying mask
        __m512i accum = _mm512_setzero_si512();
        for (int i = 0; i < 4; i++) {
            __mmask16 mask = (i % 2) ? 0xFFFF : 0xAAAA;
            __m512i temp = _mm512_mask_blend_epi32(mask, va, vb);
            accum = _mm512_add_epi32(accum, temp);
            
            // Multi-stage pipeline
            __m512i shifted = _mm512_slli_epi32(temp, 1);
            accum = _mm512_mask_blend_epi32(0x5555, accum, shifted);
        }
        
        _mm512_store_si512((__m512i*)dst32, vc);
        total_checksum += compute_checksum(dst32, sizeof(dst32));
        
        // Force materialization with inline assembly
        __m512i asm_result;
        asm volatile (
            "vmovdqa64 %1, %0\n\t"
            : "=v"(asm_result)
            : "v"(accum)
            : "memory"
        );
        (void)asm_result;
    }
    
    // E_V8DImode - 8x int64 blend
    {
        __m512i va = _mm512_load_si512((__m512i*)src64_a);
        __m512i vb = _mm512_load_si512((__m512i*)src64_b);
        
        __mmask8 mask_const = 0xAA; // Alternating bits
        __m512i vc = _mm512_mask_blend_epi64(mask_const, va, vb);
        
        // Multi-stage processing
        __m512i temp = vc;
        for (int i = 0; i < 3; i++) {
            __mmask8 mask = (0xFF << i) & 0xFF;
            temp = _mm512_mask_blend_epi64(mask, temp, vb);
            temp = _mm512_add_epi64(temp, _mm512_set1_epi64(1));
        }
        
        _mm512_store_si512((__m512i*)dst64, vc);
        total_checksum += compute_checksum(dst64, sizeof(dst64));
    }
    
    // E_V16SFmode - 16x float blend
    {
        __m512 va = _mm512_load_ps(srcf_a);
        __m512 vb = _mm512_load_ps(srcf_b);
        
        __mmask16 mask_const = 0x5555; // Alternating bits (different pattern)
        __m512 vc = _mm512_mask_blend_ps(mask_const, va, vb);
        
        // Loop with data-dependent mask
        __m512 accum = _mm512_setzero_ps();
        for (int i = 0; i < 16; i++) {
            __mmask16 mask = (srcf_a[i] > srcf_b[i]) ? 0xFFFF : 0xAAAA;
            __m512 temp = _mm512_mask_blend_ps(mask, va, vb);
            accum = _mm512_add_ps(accum, temp);
        }
        
        _mm512_store_ps(dstf, vc);
        total_checksum += compute_checksum(dstf, sizeof(dstf));
    }
    
    // E_V8DFmode - 8x double blend
    {
        __m512d va = _mm512_load_pd(srcd_a);
        __m512d vb = _mm512_load_pd(srcd_b);
        
        __mmask8 mask_const = 0x55; // Alternating bits
        __m512d vc = _mm512_mask_blend_pd(mask_const, va, vb);
        
        // Multi-stage pipeline
        __m512d temp1 = _mm512_mul_pd(vc, _mm512_set1_pd(2.0));
        __m512d temp2 = _mm512_mask_blend_pd(0xAA, temp1, vb);
        __m512d result = _mm512_add_pd(temp2, _mm512_set1_pd(1.0));
        
        _mm512_store_pd(dstd, result);
        total_checksum += compute_checksum(dstd, sizeof(dstd));
    }
#endif // __AVX512F__

#ifdef __AVX512BW__
    // ==================== AVX-512BW Cases ====================
    
    // E_V64QImode - 64x int8 blend
    {
        __m512i va = _mm512_load_si512((__m512i*)src8_a);
        __m512i vb = _mm512_load_si512((__m512i*)src8_b);
        
        __mmask64 mask_const = 0xAAAAAAAAAAAAAAAAULL; // Alternating bytes
        __m512i vc = _mm512_mask_blend_epi8(mask_const, va, vb);
        
        // Loop-based blend with varying patterns
        __m512i accum = _mm512_setzero_si512();
        for (int i = 0; i < 8; i++) {
            __mmask64 mask = (i % 3 == 0) ? 0xFFFFFFFFFFFFFFFFULL : 
                            (i % 3 == 1) ? 0xAAAAAAAAAAAAAAAAULL : 
                            0x5555555555555555ULL;
            __m512i temp = _mm512_mask_blend_epi8(mask, va, vb);
            accum = _mm512_add_epi8(accum, temp);
            
            // Additional blend in pipeline
            __m512i shifted = _mm512_slli_epi16(temp, 1);
            accum = _mm512_mask_blend_epi8(0xCCCCCCCCCCCCCCCCULL, accum, shifted);
        }
        
        _mm512_store_si512((__m512i*)dst8, vc);
        total_checksum += compute_checksum(dst8, sizeof(dst8));
    }
    
    // E_V32HImode - 32x int16 blend
    {
        __m512i va = _mm512_load_si512((__m512i*)src16_a);
        __m512i vb = _mm512_load_si512((__m512i*)src16_b);
        
        __mmask32 mask_const = 0xAAAAAAAA; // Alternating words
        __m512i vc = _mm512_mask_blend_epi16(mask_const, va, vb);
        
        // Multi-stage processing
        __m512i temp = vc;
        for (int i = 0; i < 4; i++) {
            __mmask32 mask = (0xFFFFFFFFUL >> (i * 8)) & 0xFFFFFFFFUL;
            temp = _mm512_mask_blend_epi16(mask, temp, vb);
            temp = _mm512_add_epi16(temp, _mm512_set1_epi16(1));
        }
        
        _mm512_store_si512((__m512i*)dst16, temp);
        total_checksum += compute_checksum(dst16, sizeof(dst16));
    }
    
    // E_V32HFmode - 32x half-float blend
    {
        // Use __m512i for storage, cast to __m512h for operations
        __m512i va_i = _mm512_load_si512((__m512i*)src16_a);
        __m512i vb_i = _mm512_load_si512((__m512i*)src16_b);
        
        // Convert to half-precision pattern (simulated)
        __m512h va, vb;
        #ifdef __AVX512FP16__
        // If compiler supports __m512h directly
        va = _mm512_castsi512_ph(va_i);
        vb = _mm512_castsi512_ph(vb_i);
        #else
        // Use integer representation as placeholder
        va = (__m512h)va_i;
        vb = (__m512h)vb_i;
        #endif
        
        __mmask32 mask_const = 0x55555555; // Different alternating pattern
        __m512h vc;
        
        // Use inline assembly to force blend operation
        asm volatile (
            "vmovdqu64 %1, %%zmm0\n\t"
            "vmovdqu64 %2, %%zmm1\n\t"
            "kmovd %3, %%k1\n\t"
            "vpblendmw %%zmm0, %%zmm1, %%zmm2 %{%%k1%}\n\t"
            "vmovdqu64 %%zmm2, %0\n\t"
            : "=m"(vc)
            : "m"(va), "m"(vb), "r"(mask_const)
            : "zmm0", "zmm1", "zmm2", "k1", "memory"
        );
        
        // Store result
        __m512i result_i;
        #ifdef __AVX512FP16__
        result_i = _mm512_castph_si512(vc);
        #else
        result_i = (__m512i)vc;
        #endif
        _mm512_store_si512((__m512i*)dst16, result_i);
        total_checksum += compute_checksum(dst16, sizeof(dst16));
    }
#endif // __AVX512BW__

#ifdef __AVX512BF16__
    // ==================== AVX-512BF16 Cases ====================
    
    // E_V32BFmode - 32x brain-float blend
    {
        // Use alternating pattern for brain-float data
        alignas(64) uint16_t bf_data_a[32], bf_data_b[32], bf_result[32];
        for (int i = 0; i < 32; i++) {
            bf_data_a[i] = i * 0x0101;
            bf_data_b[i] = 0x8080 - i * 0x0101;
        }
        
        __m512i va_i = _mm512_load_si512((__m512i*)bf_data_a);
        __m512i vb_i = _mm512_load_si512((__m512i*)bf_data_b);
        
        __m512bh va, vb;
        #ifdef __AVX512BF16__
        // Cast to brain-float type
        va = _mm512_castsi512_pbh(va_i);
        vb = _mm512_castsi512_pbh(vb_i);
        #else
        va = (__m512bh)va_i;
        vb = (__m512bh)vb_i;
        #endif
        
        __mmask32 mask_const = 0x33333333; // Checkerboard pattern
        
        // Force blend with inline assembly
        __m512bh vc;
        asm volatile (
            "vmovdqu64 %1, %%zmm0\n\t"
            "vmovdqu64 %2, %%zmm1\n\t"
            "kmovd %3, %%k1\n\t"
            "vpblendmw %%zmm0, %%zmm1, %%zmm2 %{%%k1%}\n\t"
            "vmovdqu64 %%zmm2, %0\n\t"
            : "=m"(vc)
            : "m"(va), "m"(vb), "r"(mask_const)
            : "zmm0", "zmm1", "zmm2", "k1", "memory"
        );
        
        // Convert back and store
        __m512i result_i;
        #ifdef __AVX512BF16__
        result_i = _mm512_castpbh_si512(vc);
        #else
        result_i = (__m512i)vc;
        #endif
        _mm512_store_si512((__m512i*)bf_result, result_i);
        total_checksum += compute_checksum(bf_result, sizeof(bf_result));
    }
#endif // __AVX512BF16__

    // Print final checksum to prevent optimization
    printf("Total checksum: %lu\n", total_checksum);
    
    return 0;
}
