#include <immintrin.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

// Initialize arrays with distinct patterns
#define ARRAY_SIZE 1024

// Integer arrays
static uint8_t  src8_a[ARRAY_SIZE]  __attribute__((aligned(64)));
static uint8_t  src8_b[ARRAY_SIZE]  __attribute__((aligned(64)));
static uint8_t  dst8[ARRAY_SIZE]    __attribute__((aligned(64)));

static uint16_t src16_a[ARRAY_SIZE] __attribute__((aligned(64)));
static uint16_t src16_b[ARRAY_SIZE] __attribute__((aligned(64)));
static uint16_t dst16[ARRAY_SIZE]   __attribute__((aligned(64)));

static uint32_t src32_a[ARRAY_SIZE] __attribute__((aligned(64)));
static uint32_t src32_b[ARRAY_SIZE] __attribute__((aligned(64)));
static uint32_t dst32[ARRAY_SIZE]   __attribute__((aligned(64)));

static uint64_t src64_a[ARRAY_SIZE] __attribute__((aligned(64)));
static uint64_t src64_b[ARRAY_SIZE] __attribute__((aligned(64)));
static uint64_t dst64[ARRAY_SIZE]   __attribute__((aligned(64)));

// Floating-point arrays
static float    srcf_a[ARRAY_SIZE]  __attribute__((aligned(64)));
static float    srcf_b[ARRAY_SIZE]  __attribute__((aligned(64)));
static float    dstf[ARRAY_SIZE]    __attribute__((aligned(64)));

static double   srcd_a[ARRAY_SIZE]  __attribute__((aligned(64)));
static double   srcd_b[ARRAY_SIZE]  __attribute__((aligned(64)));
static double   dstd[ARRAY_SIZE]    __attribute__((aligned(64)));

// Half-precision arrays (stored as uint16_t)
static uint16_t srch_a[ARRAY_SIZE]  __attribute__((aligned(64)));
static uint16_t srch_b[ARRAY_SIZE]  __attribute__((aligned(64)));
static uint16_t dsth[ARRAY_SIZE]    __attribute__((aligned(64)));

// Brain-float arrays (stored as uint16_t)
static uint16_t srcb_a[ARRAY_SIZE]  __attribute__((aligned(64)));
static uint16_t srcb_b[ARRAY_SIZE]  __attribute__((aligned(64)));
static uint16_t dstb[ARRAY_SIZE]    __attribute__((aligned(64)));

// Initialize all arrays with distinct patterns
void init_arrays() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        // Integer patterns
        src8_a[i]  = i % 256;
        src8_b[i]  = (i + 128) % 256;
        src16_a[i] = i * 3;
        src16_b[i] = i * 5;
        src32_a[i] = i * 7;
        src32_b[i] = i * 11;
        src64_a[i] = i * 13ULL;
        src64_b[i] = i * 17ULL;
        
        // Floating-point patterns
        srcf_a[i] = i * 0.125f;
        srcf_b[i] = i * 0.25f;
        srcd_a[i] = i * 0.0625;
        srcd_b[i] = i * 0.03125;
        
        // Half-precision patterns (simple integer values)
        srch_a[i] = i % 1024;
        srch_b[i] = (i + 512) % 1024;
        
        // Brain-float patterns
        srcb_a[i] = (i * 2) % 1024;
        srcb_b[i] = (i * 3) % 1024;
    }
}

// Checksum functions
uint64_t checksum_8(const uint8_t* arr, int size) {
    uint64_t sum = 0;
    for (int i = 0; i < size; i++) sum += arr[i];
    return sum;
}

uint64_t checksum_16(const uint16_t* arr, int size) {
    uint64_t sum = 0;
    for (int i = 0; i < size; i++) sum += arr[i];
    return sum;
}

uint64_t checksum_32(const uint32_t* arr, int size) {
    uint64_t sum = 0;
    for (int i = 0; i < size; i++) sum += arr[i];
    return sum;
}

uint64_t checksum_64(const uint64_t* arr, int size) {
    uint64_t sum = 0;
    for (int i = 0; i < size; i++) sum += arr[i];
    return sum;
}

double checksum_f(const float* arr, int size) {
    double sum = 0.0;
    for (int i = 0; i < size; i++) sum += arr[i];
    return sum;
}

double checksum_d(const double* arr, int size) {
    double sum = 0.0;
    for (int i = 0; i < size; i++) sum += arr[i];
    return sum;
}

int main() {
    init_arrays();
    
    uint64_t total_checksum = 0;
    double total_fp_checksum = 0.0;
    
    // AVX-512BW: Byte and Word operations
#ifdef __AVX512BW__
    {
        // V64QImode: 64x 8-bit integers
        const int vecs = ARRAY_SIZE / 64;
        for (int v = 0; v < vecs; v++) {
            __m512i a = _mm512_load_si512((__m512i*)&src8_a[v * 64]);
            __m512i b = _mm512_load_si512((__m512i*)&src8_b[v * 64]);
            
            // Constant mask pattern
            __mmask64 mask_const = 0xAAAAAAAAAAAAAAAAULL;
            __m512i result = _mm512_mask_blend_epi8(mask_const, a, b);
            
            // Loop-based varying mask
            __mmask64 mask_var = 0;
            for (int i = 0; i < 64; i++) {
                if ((v * 64 + i) % 3 == 0) {
                    mask_var |= (1ULL << i);
                }
            }
            result = _mm512_mask_blend_epi8(mask_var, result, a);
            
            // Multi-stage pipeline
            __m512i temp = _mm512_add_epi8(result, _mm512_set1_epi8(1));
            result = _mm512_mask_blend_epi8(0x5555555555555555ULL, result, temp);
            
            _mm512_store_si512((__m512i*)&dst8[v * 64], result);
            
            // Force materialization with inline assembly
            asm volatile("" : "+v"(result) : : "memory");
        }
        
        // V32HImode: 32x 16-bit integers
        for (int v = 0; v < ARRAY_SIZE / 32; v++) {
            __m512i a = _mm512_load_si512((__m512i*)&src16_a[v * 32]);
            __m512i b = _mm512_load_si512((__m512i*)&src16_b[v * 32]);
            
            __mmask32 mask_const = 0xAAAAAAAA;
            __m512i result = _mm512_mask_blend_epi16(mask_const, a, b);
            
            // Data-dependent mask
            __mmask32 mask_var = 0;
            for (int i = 0; i < 32; i++) {
                if (src16_a[v * 32 + i] > src16_b[v * 32 + i]) {
                    mask_var |= (1U << i);
                }
            }
            result = _mm512_mask_blend_epi16(mask_var, result, b);
            
            // Chained operation
            __m512i scaled = _mm512_slli_epi16(result, 1);
            result = _mm512_mask_blend_epi16(0xCCCCCCCC, result, scaled);
            
            _mm512_store_si512((__m512i*)&dst16[v * 32], result);
        }
        
        total_checksum += checksum_8(dst8, ARRAY_SIZE);
        total_checksum += checksum_16(dst16, ARRAY_SIZE);
    }
#endif // __AVX512BW__
    
    // AVX-512F: Single/Double precision and 32/64-bit integers
#ifdef __AVX512F__
    {
        // V16SImode: 16x 32-bit integers
        const int vecs = ARRAY_SIZE / 16;
        for (int v = 0; v < vecs; v++) {
            __m512i a = _mm512_load_si512((__m512i*)&src32_a[v * 16]);
            __m512i b = _mm512_load_si512((__m512i*)&src32_b[v * 16]);
            
            __mmask16 mask_const = 0xAAAA;
            __m512i result = _mm512_mask_blend_epi32(mask_const, a, b);
            
            // Loop with varying mask
            for (int i = 0; i < 4; i++) {
                __mmask16 mask = (v % (i + 2)) ? 0xFFFF : 0x0000;
                result = _mm512_mask_blend_epi32(mask, result, a);
            }
            
            _mm512_store_si512((__m512i*)&dst32[v * 16], result);
        }
        
        // V8DImode: 8x 64-bit integers
        for (int v = 0; v < ARRAY_SIZE / 8; v++) {
            __m512i a = _mm512_load_si512((__m512i*)&src64_a[v * 8]);
            __m512i b = _mm512_load_si512((__m512i*)&src64_b[v * 8]);
            
            __mmask8 mask_const = 0xAA;
            __m512i result = _mm512_mask_blend_epi64(mask_const, a, b);
            
            // Multi-stage pipeline
            __m512i sum = _mm512_add_epi64(a, b);
            result = _mm512_mask_blend_epi64(0x55, result, sum);
            result = _mm512_mask_blend_epi64(0xF0, result, _mm512_slli_epi64(result, 1));
            
            _mm512_store_si512((__m512i*)&dst64[v * 8], result);
        }
        
        // V16SFmode: 16x single-precision floats
        for (int v = 0; v < ARRAY_SIZE / 16; v++) {
            __m512 a = _mm512_load_ps(&srcf_a[v * 16]);
            __m512 b = _mm512_load_ps(&srcf_b[v * 16]);
            
            __mmask16 mask_const = 0xAAAA;
            __m512 result = _mm512_mask_blend_ps(mask_const, a, b);
            
            // Data-dependent blending
            __m512 cmp = _mm512_cmp_ps_mask(a, b, _CMP_GT_OQ);
            result = _mm512_mask_blend_ps(cmp, result, a);
            
            // Arithmetic then blend
            __m512 scaled = _mm512_mul_ps(result, _mm512_set1_ps(2.0f));
            result = _mm512_mask_blend_ps(0x5555, result, scaled);
            
            _mm512_store_ps(&dstf[v * 16], result);
        }
        
        // V8DFmode: 8x double-precision floats
        for (int v = 0; v < ARRAY_SIZE / 8; v++) {
            __m512d a = _mm512_load_pd(&srcd_a[v * 8]);
            __m512d b = _mm512_load_pd(&srcd_b[v * 8]);
            
            __mmask8 mask_const = 0xAA;
            __m512d result = _mm512_mask_blend_pd(mask_const, a, b);
            
            // Loop-based varying operations
            for (int i = 0; i < 3; i++) {
                __m512d temp = _mm512_mul_pd(result, _mm512_set1_pd(1.5));
                __mmask8 mask = (1 << (i % 8)) - 1;
                result = _mm512_mask_blend_pd(mask, result, temp);
            }
            
            _mm512_store_pd(&dstd[v * 8], result);
        }
        
        total_checksum += checksum_32(dst32, ARRAY_SIZE);
        total_checksum += checksum_64(dst64, ARRAY_SIZE / 2);
        total_fp_checksum += checksum_f(dstf, ARRAY_SIZE);
        total_fp_checksum += checksum_d(dstd, ARRAY_SIZE);
    }
#endif // __AVX512F__
    
    // Half-precision (HF) operations
#ifdef __AVX512BW__
#ifdef __AVX512FP16__
    {
        // V32HFmode: 32x half-precision floats
        const int vecs = ARRAY_SIZE / 32;
        for (int v = 0; v < vecs; v++) {
            // Load as integers and cast to half-precision
            __m512i a_int = _mm512_load_si512((__m512i*)&srch_a[v * 32]);
            __m512i b_int = _mm512_load_si512((__m512i*)&srch_b[v * 32]);
            
            __m512h a = _mm512_castsi512_ph(a_int);
            __m512h b = _mm512_castsi512_ph(b_int);
            
            __mmask32 mask_const = 0xAAAAAAAA;
            __m512h result = _mm512_mask_blend_ph(mask_const, a, b);
            
            // Store back through integer type
            __m512i result_int = _mm512_castph_si512(result);
            _mm512_store_si512((__m512i*)&dsth[v * 32], result_int);
            
            // Force compiler to consider the blend
            asm volatile("" : "+v"(result) : : "memory");
        }
        
        total_checksum += checksum_16(dsth, ARRAY_SIZE);
    }
#endif // __AVX512FP16__
#endif // __AVX512BW__
    
    // Brain-float (BF16) operations
#ifdef __AVX512BF16__
    {
        // V32BFmode: 32x brain-float
        const int vecs = ARRAY_SIZE / 32;
        for (int v = 0; v < vecs; v++) {
            // Load as integers and cast to brain-float
            __m512i a_int = _mm512_load_si512((__m512i*)&srcb_a[v * 32]);
            __m512i b_int = _mm512_load_si512((__m512i*)&srcb_b[v * 32]);
            
            __m512bh a = _mm512_castsi512_pbh(a_int);
            __m512bh b = _mm512_castsi512_pbh(b_int);
            
            __mmask32 mask_const = 0xAAAAAAAA;
            __m512bh result = _mm512_mask_blend_epi16(mask_const, a, b);
            
            // Store back
            __m512i result_int = _mm512_castpbh_si512(result);
            _mm512_store_si512((__m512i*)&dstb[v * 32], result_int);
            
            // Prevent optimization
            asm volatile("" : "+v"(result) : : "memory");
        }
        
        total_checksum += checksum_16(dstb, ARRAY_SIZE);
    }
#endif // __AVX512BF16__
    
    // Print checksums to prevent dead code elimination
    printf("Integer checksum: %lu\n", total_checksum);
    printf("FP checksum: %f\n", total_fp_checksum);
    
    return 0;
}
