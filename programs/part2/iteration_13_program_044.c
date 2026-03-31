#include <immintrin.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

// Initialize arrays with distinct patterns
void init_arrays() {
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

int main() {
    printf("AVX-512 Blend Expansion Test\n");
    
    // Initialize source arrays with distinct patterns
    alignas(64) uint8_t src8_a[64], src8_b[64];
    alignas(64) uint16_t src16_a[32], src16_b[32];
    alignas(64) int32_t src32_a[16], src32_b[16];
    alignas(64) int64_t src64_a[8], src64_b[8];
    alignas(64) float srcf_a[16], srcf_b[16];
    alignas(64) double srcd_a[8], srcd_b[8];
    
    for (int i = 0; i < 64; i++) {
        src8_a[i] = i;
        src8_b[i] = 64 - i;
    }
    for (int i = 0; i < 32; i++) {
        src16_a[i] = i * 2;
        src16_b[i] = 64 - i * 2;
    }
    for (int i = 0; i < 16; i++) {
        src32_a[i] = i * 4;
        src32_b[i] = 64 - i * 4;
    }
    for (int i = 0; i < 8; i++) {
        src64_a[i] = i * 8;
        src64_b[i] = 64 - i * 8;
    }
    for (int i = 0; i < 16; i++) {
        srcf_a[i] = i * 0.5f;
        srcf_b[i] = 8.0f - i * 0.5f;
    }
    for (int i = 0; i < 8; i++) {
        srcd_a[i] = i * 0.25;
        srcd_b[i] = 2.0 - i * 0.25;
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
    printf("Testing AVX512BW blends (V64QI, V32HI modes)...\n");
    
    // V64QImode blend - E_V64QImode case
    {
        __m512i vec_a = _mm512_loadu_si512((const __m512i*)src8_a);
        __m512i vec_b = _mm512_loadu_si512((const __m512i*)src8_b);
        
        // Multi-stage processing pipeline
        __m512i result = vec_a;
        for (int i = 0; i < 4; i++) {
            // Varying mask based on loop index
            __mmask64 mask = (i % 3) ? 0xAAAAAAAAAAAAAAAAULL : 0x5555555555555555ULL;
            
            // Chain blend operations
            __m512i temp = _mm512_mask_blend_epi8(mask, result, vec_b);
            
            // Additional arithmetic to prevent optimization
            temp = _mm512_add_epi8(temp, _mm512_set1_epi8(1));
            
            // Second blend in the pipeline
            __mmask64 mask2 = (i % 2) ? 0xCCCCCCCCCCCCCCCCULL : 0x3333333333333333ULL;
            result = _mm512_mask_blend_epi8(mask2, temp, vec_a);
        }
        
        _mm512_storeu_si512((__m512i*)dst8, result);
        
        // Force materialization with inline assembly
        asm volatile ("" : : "v"(result) : "memory");
        
        total_checksum += checksum_512i(&result, 1);
    }
    
    // V32HImode blend - E_V32HImode case
    {
        __m512i vec_a = _mm512_loadu_si512((const __m512i*)src16_a);
        __m512i vec_b = _mm512_loadu_si512((const __m512i*)src16_b);
        
        __m512i result = vec_a;
        for (int i = 0; i < 4; i++) {
            __mmask32 mask = (i % 3) ? 0xAAAAAAAA : 0x55555555;
            
            // Explicit blend with 16-bit elements
            __m512i temp = _mm512_mask_blend_epi16(mask, result, vec_b);
            
            // Additional processing
            temp = _mm512_add_epi16(temp, _mm512_set1_epi16(1));
            
            // Second blend with different pattern
            __mmask32 mask2 = (i % 2) ? 0xCCCCCCCC : 0x33333333;
            result = _mm512_mask_blend_epi16(mask2, temp, vec_a);
        }
        
        _mm512_storeu_si512((__m512i*)dst16, result);
        asm volatile ("" : : "v"(result) : "memory");
        
        total_checksum += checksum_512i(&result, 1);
    }
#endif // __AVX512BW__

#ifdef __AVX512F__
    printf("Testing AVX512F blends (V16SI, V8DI, V16SF, V8DF modes)...\n");
    
    // V16SImode blend - E_V16SImode case
    {
        __m512i vec_a = _mm512_loadu_si512((const __m512i*)src32_a);
        __m512i vec_b = _mm512_loadu_si512((const __m512i*)src32_b);
        
        __m512i result = vec_a;
        for (int i = 0; i < 4; i++) {
            __mmask16 mask = (i % 3) ? 0xAAAA : 0x5555;
            
            // 32-bit integer blend
            __m512i temp = _mm512_mask_blend_epi32(mask, result, vec_b);
            
            // Arithmetic operation
            temp = _mm512_add_epi32(temp, _mm512_set1_epi32(1));
            
            // Second blend
            __mmask16 mask2 = (i % 2) ? 0xCCCC : 0x3333;
            result = _mm512_mask_blend_epi32(mask2, temp, vec_a);
        }
        
        _mm512_storeu_si512((__m512i*)dst32, result);
        asm volatile ("" : : "v"(result) : "memory");
        
        total_checksum += checksum_512i(&result, 1);
    }
    
    // V8DImode blend - E_V8DImode case
    {
        __m512i vec_a = _mm512_loadu_si512((const __m512i*)src64_a);
        __m512i vec_b = _mm512_loadu_si512((const __m512i*)src64_b);
        
        __m512i result = vec_a;
        for (int i = 0; i < 4; i++) {
            __mmask8 mask = (i % 3) ? 0xAA : 0x55;
            
            // 64-bit integer blend
            __m512i temp = _mm512_mask_blend_epi64(mask, result, vec_b);
            
            // Arithmetic
            temp = _mm512_add_epi64(temp, _mm512_set1_epi64(1));
            
            // Second blend
            __mmask8 mask2 = (i % 2) ? 0xCC : 0x33;
            result = _mm512_mask_blend_epi64(mask2, temp, vec_a);
        }
        
        _mm512_storeu_si512((__m512i*)dst64, result);
        asm volatile ("" : : "v"(result) : "memory");
        
        total_checksum += checksum_512i(&result, 1);
    }
    
    // V16SFmode blend - E_V16SFmode case
    {
        __m512 vec_a = _mm512_loadu_ps(srcf_a);
        __m512 vec_b = _mm512_loadu_ps(srcf_b);
        
        __m512 result = vec_a;
        for (int i = 0; i < 4; i++) {
            __mmask16 mask = (i % 3) ? 0xAAAA : 0x5555;
            
            // Single-precision float blend
            __m512 temp = _mm512_mask_blend_ps(mask, result, vec_b);
            
            // Additional computation
            temp = _mm512_add_ps(temp, _mm512_set1_ps(1.0f));
            
            // Second blend
            __mmask16 mask2 = (i % 2) ? 0xCCCC : 0x3333;
            result = _mm512_mask_blend_ps(mask2, temp, vec_a);
        }
        
        _mm512_storeu_ps(dstf, result);
        asm volatile ("" : : "v"(result) : "memory");
        
        total_checksum += (uint64_t)checksum_512f(&result, 1);
    }
    
    // V8DFmode blend - E_V8DFmode case
    {
        __m512d vec_a = _mm512_loadu_pd(srcd_a);
        __m512d vec_b = _mm512_loadu_pd(srcd_b);
        
        __m512d result = vec_a;
        for (int i = 0; i < 4; i++) {
            __mmask8 mask = (i % 3) ? 0xAA : 0x55;
            
            // Double-precision blend
            __m512d temp = _mm512_mask_blend_pd(mask, result, vec_b);
            
            // Computation
            temp = _mm512_add_pd(temp, _mm512_set1_pd(1.0));
            
            // Second blend
            __mmask8 mask2 = (i % 2) ? 0xCC : 0x33;
            result = _mm512_mask_blend_pd(mask2, temp, vec_a);
        }
        
        _mm512_storeu_pd(dstd, result);
        asm volatile ("" : : "v"(result) : "memory");
        
        total_checksum += (uint64_t)checksum_512d(&result, 1);
    }
#endif // __AVX512F__

#ifdef __AVX512BF16__
    printf("Testing AVX512BF16 blends (V32HF, V32BF modes)...\n");
    
    // For half-precision (HF) and brain-float (BF) modes
    // We need to use appropriate types if available
    
    // V32HFmode blend - E_V32HFmode case
    {
        // Initialize half-precision data
        alignas(64) uint16_t hf_data_a[32], hf_data_b[32];
        for (int i = 0; i < 32; i++) {
            hf_data_a[i] = i * 256;  // Simple pattern
            hf_data_b[i] = 8192 - i * 256;
        }
        
        __m512i vec_a = _mm512_loadu_si512((const __m512i*)hf_data_a);
        __m512i vec_b = _mm512_loadu_si512((const __m512i*)hf_data_b);
        
        // Cast to half-precision if supported
        #ifdef __AVX512FP16__
        __m512h hvec_a = _mm512_castsi512_ph(vec_a);
        __m512h hvec_b = _mm512_castsi512_ph(vec_b);
        __m512h hresult = hvec_a;
        
        for (int i = 0; i < 4; i++) {
            __mmask32 mask = (i % 3) ? 0xAAAAAAAA : 0x55555555;
            
            // Half-precision blend would use _mm512_mask_blend_ph if available
            // For now, we'll use the integer version which should still trigger the mode
            __m512i temp = _mm512_mask_blend_epi16(mask, 
                _mm512_castph_si512(hresult), 
                _mm512_castph_si512(hvec_b));
            
            hresult = _mm512_castsi512_ph(temp);
        }
        
        _mm512_storeu_si512((__m512i*)hf_data_a, _mm512_castph_si512(hresult));
        asm volatile ("" : : "v"(hresult) : "memory");
        #else
        // Fallback: use integer operations that should still trigger V32HFmode
        __m512i result = vec_a;
        for (int i = 0; i < 4; i++) {
            __mmask32 mask = (i % 3) ? 0xAAAAAAAA : 0x55555555;
            result = _mm512_mask_blend_epi16(mask, result, vec_b);
        }
        _mm512_storeu_si512((__m512i*)hf_data_a, result);
        asm volatile ("" : : "v"(result) : "memory");
        #endif
        
        total_checksum += checksum_512i(&vec_a, 1);
    }
    
    // V32BFmode blend - E_V32BFmode case
    {
        // Brain-float data
        alignas(64) uint16_t bf_data_a[32], bf_data_b[32];
        for (int i = 0; i < 32; i++) {
            bf_data_a[i] = i * 128;
            bf_data_b[i] = 4096 - i * 128;
        }
        
        __m512i vec_a = _mm512_loadu_si512((const __m512i*)bf_data_a);
        __m512i vec_b = _mm512_loadu_si512((const __m512i*)bf_data_b);
        
        #ifdef __AVX512BF16__
        // Use brain-float specific operations if available
        __m512bh bfvec_a = _mm512_castsi512_pbh(vec_a);
        __m512bh bfvec_b = _mm512_castsi512_pbh(vec_b);
        
        // Multi-stage processing
        __m512i result = vec_a;
        for (int i = 0; i < 4; i++) {
            __mmask32 mask = (i % 3) ? 0xAAAAAAAA : 0x55555555;
            
            // Brain-float blend through integer operations
            __m512i temp = _mm512_mask_blend_epi16(mask, result, vec_b);
            
            // Chain operations
            __mmask32 mask2 = (i % 2) ? 0xCCCCCCCC : 0x33333333;
            result = _mm512_mask_blend_epi16(mask2, temp, vec_a);
        }
        
        _mm512_storeu_si512((__m512i*)bf_data_a, result);
        asm volatile ("" : : "v"(result) : "memory");
        #else
        // Fallback for non-BF16 targets
        __m512i result = vec_a;
        for (int i = 0; i < 4; i++) {
            __mmask32 mask = (i % 3) ? 0xAAAAAAAA : 0x55555555;
            result = _mm512_mask_blend_epi16(mask, result, vec_b);
        }
        _mm512_storeu_si512((__m512i*)bf_data_a, result);
        asm volatile ("" : : "v"(result) : "memory");
        #endif
        
        total_checksum += checksum_512i(&vec_a, 1);
    }
#endif // __AVX512BF16__

    printf("Total checksum: %lu\n", total_checksum);
    printf("Test completed successfully.\n");
    
    return 0;
}
