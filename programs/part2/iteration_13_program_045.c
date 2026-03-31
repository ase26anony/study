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
uint64_t checksum_512i(__m512i v) {
    union {
        __m512i vec;
        uint8_t bytes[64];
    } u = {v};
    
    uint64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += u.bytes[i];
    }
    return sum;
}

float checksum_512f(__m512 v) {
    union {
        __m512 vec;
        float floats[16];
    } u = {v};
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += u.floats[i];
    }
    return sum;
}

double checksum_512d(__m512d v) {
    union {
        __m512d vec;
        double doubles[8];
    } u = {v};
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += u.doubles[i];
    }
    return sum;
}

int main() {
    uint64_t total_checksum = 0;
    
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
        src16_b[i] = 1000 - i * 3;
    }
    for (int i = 0; i < 16; i++) {
        src32_a[i] = i * 100;
        src32_b[i] = 5000 - i * 200;
    }
    for (int i = 0; i < 8; i++) {
        src64_a[i] = i * 1000LL;
        src64_b[i] = 100000LL - i * 5000LL;
    }
    for (int i = 0; i < 16; i++) {
        srcf_a[i] = i * 1.5f;
        srcf_b[i] = 100.0f - i * 3.0f;
    }
    for (int i = 0; i < 8; i++) {
        srcd_a[i] = i * 2.5;
        srcd_b[i] = 200.0 - i * 6.0;
    }
    
#ifdef __AVX512BW__
    printf("Testing AVX512BW blend operations...\n");
    
    // Test case 1: V64QImode - 64x 8-bit integers
    {
        __m512i vec_a = _mm512_load_si512((__m512i*)src8_a);
        __m512i vec_b = _mm512_load_si512((__m512i*)src8_b);
        
        // Constant mask pattern
        __mmask64 mask64_const = 0xAAAAAAAAAAAAAAAAULL;
        
        // First blend with constant mask
        __m512i result = _mm512_mask_blend_epi8(mask64_const, vec_a, vec_b);
        
        // Loop-based blend with varying masks
        __m512i accum = _mm512_setzero_si512();
        for (int i = 0; i < 4; i++) {
            // Create data-dependent mask
            __mmask64 mask = (i % 2) ? 0xFFFFFFFFFFFFFFFFULL : 0x5555555555555555ULL;
            __m512i temp = _mm512_mask_blend_epi8(mask, vec_a, vec_b);
            
            // Multi-stage pipeline: blend -> add -> blend
            temp = _mm512_add_epi8(temp, _mm512_set1_epi8(i));
            accum = _mm512_mask_blend_epi8(mask64_const, accum, temp);
        }
        
        // Force materialization with inline assembly
        __m512i final_result;
        asm volatile (
            "vmovdqa64 %1, %0\n\t"
            : "=v"(final_result)
            : "v"(accum)
            : "memory"
        );
        
        total_checksum += checksum_512i(final_result);
    }
    
    // Test case 2: V32HImode - 32x 16-bit integers
    {
        __m512i vec_a = _mm512_load_si512((__m512i*)src16_a);
        __m512i vec_b = _mm512_load_si512((__m512i*)src16_b);
        
        __mmask32 mask32_const = 0xAAAAAAAA;
        
        // Multi-stage processing pipeline
        __m512i result = _mm512_mask_blend_epi16(mask32_const, vec_a, vec_b);
        
        for (int i = 0; i < 8; i++) {
            __mmask32 mask = (i % 3) ? 0xFFFFFFFF : 0x0000FFFF;
            __m512i temp = _mm512_mask_blend_epi16(mask, vec_a, vec_b);
            temp = _mm512_add_epi16(temp, _mm512_set1_epi16(i));
            result = _mm512_mask_blend_epi16(mask32_const, result, temp);
        }
        
        total_checksum += checksum_512i(result);
    }
#endif // __AVX512BW__

#ifdef __AVX512F__
    printf("Testing AVX512F blend operations...\n");
    
    // Test case 3: V16SImode - 16x 32-bit integers
    {
        __m512i vec_a = _mm512_load_si512((__m512i*)src32_a);
        __m512i vec_b = _mm512_load_si512((__m512i*)src32_b);
        
        __mmask16 mask16_const = 0xAAAA;
        
        __m512i result = _mm512_mask_blend_epi32(mask16_const, vec_a, vec_b);
        
        // Loop with varying masks
        for (int i = 0; i < 16; i++) {
            __mmask16 mask = (1 << (i % 16));
            __m512i temp = _mm512_mask_blend_epi32(mask, vec_a, vec_b);
            temp = _mm512_add_epi32(temp, _mm512_set1_epi32(i));
            result = _mm512_mask_blend_epi32(mask16_const, result, temp);
        }
        
        total_checksum += checksum_512i(result);
    }
    
    // Test case 4: V8DImode - 8x 64-bit integers
    {
        __m512i vec_a = _mm512_load_si512((__m512i*)src64_a);
        __m512i vec_b = _mm512_load_si512((__m512i*)src64_b);
        
        __mmask8 mask8_const = 0xAA;
        
        __m512i result = _mm512_mask_blend_epi64(mask8_const, vec_a, vec_b);
        
        for (int i = 0; i < 8; i++) {
            __mmask8 mask = (i % 2) ? 0xFF : 0x55;
            __m512i temp = _mm512_mask_blend_epi64(mask, vec_a, vec_b);
            temp = _mm512_add_epi64(temp, _mm512_set1_epi64(i));
            result = _mm512_mask_blend_epi64(mask8_const, result, temp);
        }
        
        total_checksum += checksum_512i(result);
    }
    
    // Test case 5: V16SFmode - 16x single-precision floats
    {
        __m512 vec_a = _mm512_load_ps(srcf_a);
        __m512 vec_b = _mm512_load_ps(srcf_b);
        
        __mmask16 mask16_const = 0xAAAA;
        
        __m512 result = _mm512_mask_blend_ps(mask16_const, vec_a, vec_b);
        
        // Multi-stage pipeline with arithmetic
        for (int i = 0; i < 4; i++) {
            __mmask16 mask = (i % 2) ? 0xFFFF : 0x5555;
            __m512 temp = _mm512_mask_blend_ps(mask, vec_a, vec_b);
            temp = _mm512_add_ps(temp, _mm512_set1_ps(i * 0.5f));
            result = _mm512_mask_blend_ps(mask16_const, result, temp);
        }
        
        total_checksum += (uint64_t)checksum_512f(result);
    }
    
    // Test case 6: V8DFmode - 8x double-precision floats
    {
        __m512d vec_a = _mm512_load_pd(srcd_a);
        __m512d vec_b = _mm512_load_pd(srcd_b);
        
        __mmask8 mask8_const = 0xAA;
        
        __m512d result = _mm512_mask_blend_pd(mask8_const, vec_a, vec_b);
        
        for (int i = 0; i < 4; i++) {
            __mmask8 mask = (i % 2) ? 0xFF : 0x55;
            __m512d temp = _mm512_mask_blend_pd(mask, vec_a, vec_b);
            temp = _mm512_add_pd(temp, _mm512_set1_pd(i * 0.25));
            result = _mm512_mask_blend_pd(mask8_const, result, temp);
        }
        
        total_checksum += (uint64_t)checksum_512d(result);
    }
#endif // __AVX512F__

#ifdef __AVX512BF16__
    printf("Testing AVX512BF16 blend operations...\n");
    
    // Test case 7: V32BFmode - 32x brain-float (bfloat16)
    {
        // Initialize bfloat16 arrays
        alignas(64) uint16_t srcbf_a[32], srcbf_b[32];
        for (int i = 0; i < 32; i++) {
            srcbf_a[i] = i * 0x100;  // Simple bfloat16 pattern
            srcbf_b[i] = 0x4000 - i * 0x80;
        }
        
        __m512bh vec_a = _mm512_load_si512((__m512i*)srcbf_a);
        __m512bh vec_b = _mm512_load_si512((__m512i*)srcbf_b);
        
        __mmask32 mask32_const = 0xAAAAAAAA;
        
        // Use appropriate blend intrinsic for bfloat16
        __m512bh result = _mm512_mask_blend_epi16(mask32_const, 
            (__m512i)vec_a, (__m512i)vec_b);
        
        // Cast back to bfloat16 vector type
        result = (__m512bh)result;
        
        // Loop-based processing
        for (int i = 0; i < 4; i++) {
            __mmask32 mask = (i % 2) ? 0xFFFFFFFF : 0x55555555;
            __m512bh temp = (__m512bh)_mm512_mask_blend_epi16(mask, 
                (__m512i)vec_a, (__m512i)vec_b);
            
            // Force materialization
            asm volatile (
                "vmovdqa64 %1, %0\n\t"
                : "=v"(temp)
                : "v"(temp)
                : "memory"
            );
            
            result = (__m512bh)_mm512_mask_blend_epi16(mask32_const, 
                (__m512i)result, (__m512i)temp);
        }
        
        // Store and checksum
        alignas(64) uint16_t bf_result[32];
        _mm512_store_si512((__m512i*)bf_result, (__m512i)result);
        
        uint64_t bf_sum = 0;
        for (int i = 0; i < 32; i++) {
            bf_sum += bf_result[i];
        }
        total_checksum += bf_sum;
    }
#endif // __AVX512BF16__

    // For half-precision (HF), we need to check for F16C support
#if defined(__AVX512BW__) && defined(__F16C__)
    printf("Testing half-precision blend operations...\n");
    
    // Test case 8: V32HFmode - 32x half-precision floats
    {
        // Initialize half-precision arrays
        alignas(64) uint16_t srchf_a[32], srchf_b[32];
        for (int i = 0; i < 32; i++) {
            srchf_a[i] = i * 0x0400;  // Simple half-precision pattern
            srchf_b[i] = 0x3C00 - i * 0x0200;  // 1.0 - i*0.0078
        }
        
        // Load as integers and cast to half-precision
        __m512i vec_a_int = _mm512_load_si512((__m512i*)srchf_a);
        __m512i vec_b_int = _mm512_load_si512((__m512i*)srchf_b);
        
        __m512h vec_a = _mm512_castsi512_ph(vec_a_int);
        __m512h vec_b = _mm512_castsi512_ph(vec_b_int);
        
        __mmask32 mask32_const = 0xAAAAAAAA;
        
        // Blend using integer blend since there's no direct half-precision blend
        __m512i result_int = _mm512_mask_blend_epi16(mask32_const, vec_a_int, vec_b_int);
        __m512h result = _mm512_castsi512_ph(result_int);
        
        // Multi-stage processing
        for (int i = 0; i < 4; i++) {
            __mmask32 mask = (i % 3) ? 0xFFFFFFFF : 0x0000FFFF;
            __m512i temp_int = _mm512_mask_blend_epi16(mask, vec_a_int, vec_b_int);
            
            // Force compiler to consider the half-precision type
            asm volatile (
                "vmovdqa64 %1, %0\n\t"
                : "=v"(temp_int)
                : "v"(temp_int)
                : "memory"
            );
            
            result_int = _mm512_mask_blend_epi16(mask32_const, result_int, temp_int);
        }
        
        result = _mm512_castsi512_ph(result_int);
        
        // Store and checksum
        alignas(64) uint16_t hf_result[32];
        _mm512_store_si512((__m512i*)hf_result, result_int);
        
        uint64_t hf_sum = 0;
        for (int i = 0; i < 32; i++) {
            hf_sum += hf_result[i];
        }
        total_checksum += hf_sum;
    }
#endif // __AVX512BW__ && __F16C__

    printf("Final checksum: %lu\n", total_checksum);
    return 0;
}
