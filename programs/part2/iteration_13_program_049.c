#include <immintrin.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

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

// Float arrays
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
        src8_a[i]  = i & 0xFF;
        src8_b[i]  = ~src8_a[i];
        src16_a[i] = i * 3;
        src16_b[i] = i * 7;
        src32_a[i] = i * 11;
        src32_b[i] = i * 13;
        src64_a[i] = (uint64_t)i * 17;
        src64_b[i] = (uint64_t)i * 19;
        
        // Float patterns
        srcf_a[i] = i * 1.5f;
        srcf_b[i] = i * 2.5f;
        srcd_a[i] = i * 3.14159;
        srcd_b[i] = i * 2.71828;
        
        // Half-precision patterns (store as raw bits)
        srch_a[i] = (uint16_t)(i * 5);
        srch_b[i] = (uint16_t)(i * 9);
        
        // Brain-float patterns (store as raw bits)
        srcb_a[i] = (uint16_t)(i * 15);
        srcb_b[i] = (uint16_t)(i * 21);
    }
}

// Simple checksum functions
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
    for ( int i = 0; i < size; i++) sum += arr[i];
    return sum;
}

int main() {
    init_arrays();
    
    uint64_t total_checksum = 0;
    
#ifdef __AVX512BW__
    // ==================== V64QImode (64x char) ====================
    {
        const __mmask64 const_mask1 = 0xAAAAAAAAAAAAAAAAULL;
        const __mmask64 const_mask2 = 0x5555555555555555ULL;
        
        for (int i = 0; i < ARRAY_SIZE; i += 64) {
            __m512i va = _mm512_load_si512((__m512i*)&src8_a[i]);
            __m512i vb = _mm512_load_si512((__m512i*)&src8_b[i]);
            
            // Multi-stage blend pipeline
            __m512i v1 = _mm512_mask_blend_epi8(const_mask1, va, vb);
            
            // Loop-based varying mask
            __mmask64 dynamic_mask = 0;
            for (int j = 0; j < 64; j++) {
                if ((i + j) % 3 == 0) {
                    dynamic_mask |= (1ULL << j);
                }
            }
            __m512i v2 = _mm512_mask_blend_epi8(dynamic_mask, v1, va);
            
            // Third blend with different mask
            __m512i v3 = _mm512_mask_blend_epi8(const_mask2, v2, vb);
            
            _mm512_store_si512((__m512i*)&dst8[i], v3);
            
            // Force materialization with inline assembly
            asm volatile("" : "+v"(va), "+v"(vb), "+v"(v1), "+v"(v2), "+v"(v3) : : "memory");
        }
        
        total_checksum += checksum_8(dst8, ARRAY_SIZE);
    }
    
    // ==================== V32HImode (32x short) ====================
    {
        const __mmask32 const_mask1 = 0xAAAAAAAA;
        const __mmask32 const_mask2 = 0x55555555;
        
        for (int i = 0; i < ARRAY_SIZE; i += 32) {
            __m512i va = _mm512_load_si512((__m512i*)&src16_a[i]);
            __m512i vb = _mm512_load_si512((__m512i*)&src16_b[i]);
            
            // Multi-stage processing
            __m512i v1 = _mm512_mask_blend_epi16(const_mask1, va, vb);
            
            // Data-dependent mask
            __mmask32 dynamic_mask = 0;
            for (int j = 0; j < 32; j++) {
                if (src16_a[i + j] > src16_b[i + j]) {
                    dynamic_mask |= (1U << j);
                }
            }
            __m512i v2 = _mm512_mask_blend_epi16(dynamic_mask, v1, va);
            
            __m512i v3 = _mm512_mask_blend_epi16(const_mask2, v2, vb);
            
            _mm512_store_si512((__m512i*)&dst16[i], v3);
            
            asm volatile("" : "+v"(va), "+v"(vb), "+v"(v1), "+v"(v2), "+v"(v3) : : "memory");
        }
        
        total_checksum += checksum_16(dst16, ARRAY_SIZE);
    }
#endif // __AVX512BW__

#ifdef __AVX512F__
    // ==================== V16SImode (16x int) ====================
    {
        const __mmask16 const_mask1 = 0xAAAA;
        const __mmask16 const_mask2 = 0x5555;
        
        for (int i = 0; i < ARRAY_SIZE; i += 16) {
            __m512i va = _mm512_load_si512((__m512i*)&src32_a[i]);
            __m512i vb = _mm512_load_si512((__m512i*)&src32_b[i]);
            
            // Chained blend operations
            __m512i v1 = _mm512_mask_blend_epi32(const_mask1, va, vb);
            
            __mmask16 dynamic_mask = 0;
            for (int j = 0; j < 16; j++) {
                if ((i + j) % 5 == 0) {
                    dynamic_mask |= (1U << j);
                }
            }
            __m512i v2 = _mm512_mask_blend_epi32(dynamic_mask, v1, va);
            
            __m512i v3 = _mm512_mask_blend_epi32(const_mask2, v2, vb);
            
            _mm512_store_si512((__m512i*)&dst32[i], v3);
            
            asm volatile("" : "+v"(va), "+v"(vb), "+v"(v1), "+v"(v2), "+v"(v3) : : "memory");
        }
        
        total_checksum += checksum_32(dst32, ARRAY_SIZE);
    }
    
    // ==================== V8DImode (8x long) ====================
    {
        const __mmask8 const_mask1 = 0xAA;
        const __mmask8 const_mask2 = 0x55;
        
        for (int i = 0; i < ARRAY_SIZE; i += 8) {
            __m512i va = _mm512_load_si512((__m512i*)&src64_a[i]);
            __m512i vb = _mm512_load_si512((__m512i*)&src64_b[i]);
            
            __m512i v1 = _mm512_mask_blend_epi64(const_mask1, va, vb);
            
            __mmask8 dynamic_mask = 0;
            for (int j = 0; j < 8; j++) {
                if (src64_a[i + j] % 2 == 0) {
                    dynamic_mask |= (1U << j);
                }
            }
            __m512i v2 = _mm512_mask_blend_epi64(dynamic_mask, v1, va);
            
            __m512i v3 = _mm512_mask_blend_epi64(const_mask2, v2, vb);
            
            _mm512_store_si512((__m512i*)&dst64[i], v3);
            
            asm volatile("" : "+v"(va), "+v"(vb), "+v"(v1), "+v"(v2), "+v"(v3) : : "memory");
        }
        
        total_checksum += checksum_64(dst64, ARRAY_SIZE);
    }
    
    // ==================== V16SFmode (16x float) ====================
    {
        const __mmask16 const_mask1 = 0xAAAA;
        const __mmask16 const_mask2 = 0x5555;
        
        for (int i = 0; i < ARRAY_SIZE; i += 16) {
            __m512 va = _mm512_load_ps(&srcf_a[i]);
            __m512 vb = _mm512_load_ps(&srcf_b[i]);
            
            // Multi-stage float blend pipeline
            __m512 v1 = _mm512_mask_blend_ps(const_mask1, va, vb);
            
            __mmask16 dynamic_mask = 0;
            for (int j = 0; j < 16; j++) {
                if (srcf_a[i + j] < srcf_b[i + j]) {
                    dynamic_mask |= (1U << j);
                }
            }
            __m512 v2 = _mm512_mask_blend_ps(dynamic_mask, v1, va);
            
            // Additional arithmetic operation before final blend
            __m512 v2_scaled = _mm512_mul_ps(v2, _mm512_set1_ps(1.5f));
            __m512 v3 = _mm512_mask_blend_ps(const_mask2, v2_scaled, vb);
            
            _mm512_store_ps(&dstf[i], v3);
            
            asm volatile("" : "+v"(va), "+v"(vb), "+v"(v1), "+v"(v2), "+v"(v3) : : "memory");
        }
        
        total_checksum += (uint64_t)checksum_f(dstf, ARRAY_SIZE);
    }
    
    // ==================== V8DFmode (8x double) ====================
    {
        const __mmask8 const_mask1 = 0xAA;
        const __mmask8 const_mask2 = 0x55;
        
        for (int i = 0; i < ARRAY_SIZE; i += 8) {
            __m512d va = _mm512_load_pd(&srcd_a[i]);
            __m512d vb = _mm512_load_pd(&srcd_b[i]);
            
            __m512d v1 = _mm512_mask_blend_pd(const_mask1, va, vb);
            
            __mmask8 dynamic_mask = 0;
            for (int j = 0; j < 8; j++) {
                if ((i + j) % 4 == 0) {
                    dynamic_mask |= (1U << j);
                }
            }
            __m512d v2 = _mm512_mask_blend_pd(dynamic_mask, v1, va);
            
            // Arithmetic operation in the pipeline
            __m512d v2_scaled = _mm512_mul_pd(v2, _mm512_set1_pd(2.0));
            __m512d v3 = _mm512_mask_blend_pd(const_mask2, v2_scaled, vb);
            
            _mm512_store_pd(&dstd[i], v3);
            
            asm volatile("" : "+v"(va), "+v"(vb), "+v"(v1), "+v"(v2), "+v"(v3) : : "memory");
        }
        
        total_checksum += (uint64_t)checksum_d(dstd, ARRAY_SIZE);
    }
#endif // __AVX512F__

#ifdef __AVX512BW__
    // ==================== V32HFmode (32x half-float) ====================
    {
        const __mmask32 const_mask1 = 0xAAAAAAAA;
        const __mmask32 const_mask2 = 0x55555555;
        
        for (int i = 0; i < ARRAY_SIZE; i += 32) {
            // Load as integers, cast to half-precision
            __m512i va_int = _mm512_load_si512((__m512i*)&srch_a[i]);
            __m512i vb_int = _mm512_load_si512((__m512i*)&srch_b[i]);
            
#if defined(__AVX512FP16__) && defined(__AVX512BW__)
            // Use native half-precision if available
            __m512h va = _mm512_castsi512_ph(va_int);
            __m512h vb = _mm512_castsi512_ph(vb_int);
            
            __m512h v1 = _mm512_mask_blend_ph(const_mask1, va, vb);
            
            __mmask32 dynamic_mask = 0;
            for (int j = 0; j < 32; j++) {
                if ((i + j) % 7 == 0) {
                    dynamic_mask |= (1U << j);
                }
            }
            __m512h v2 = _mm512_mask_blend_ph(dynamic_mask, v1, va);
            
            __m512h v3 = _mm512_mask_blend_ph(const_mask2, v2, vb);
            
            _mm512_store_si512((__m512i*)&dsth[i], _mm512_castph_si512(v3));
            
            asm volatile("" : "+v"(va), "+v"(vb), "+v"(v1), "+v"(v2), "+v"(v3) : : "memory");
#else
            // Fallback: use integer blend with appropriate casting
            __m512i v1 = _mm512_mask_blend_epi16(const_mask1, va_int, vb_int);
            
            __mmask32 dynamic_mask = 0;
            for (int j = 0; j < 32; j++) {
                if ((i + j) % 7 == 0) {
                    dynamic_mask |= (1U << j);
                }
            }
            __m512i v2 = _mm512_mask_blend_epi16(dynamic_mask, v1, va_int);
            
            __m512i v3 = _mm512_mask_blend_epi16(const_mask2, v2, vb_int);
            
            _mm512_store_si512((__m512i*)&dsth[i], v3);
            
            asm volatile("" : "+v"(va_int), "+v"(vb_int), "+v"(v1), "+v"(v2), "+v"(v3) : : "memory");
#endif
        }
        
        total_checksum += checksum_16(dsth, ARRAY_SIZE);
    }
#endif // __AVX512BW__

#ifdef __AVX512BF16__
    // ==================== V32BFmode (32x brain-float) ====================
    {
        const __mmask32 const_mask1 = 0xAAAAAAAA;
        const __mmask32 const_mask2 = 0x55555555;
        
        for (int i = 0; i < ARRAY_SIZE; i += 32) {
            // Load as integers
            __m512i va_int = _mm512_load_si512((__m512i*)&srcb_a[i]);
            __m512i vb_int = _mm512_load_si512((__m512i*)&srcb_b[i]);
            
#if defined(__AVX512BF16__)
            // Cast to brain-float
            __m512bh va = _mm512_castsi512_pbh(va_int);
            __m512bh vb = _mm512_castsi512_pbh(vb_int);
            
            // Use appropriate blend intrinsic for brain-float
            __m512bh v1 = _mm512_mask_blend_epi16(const_mask1, va, vb);
            
            __mmask32 dynamic_mask = 0;
            for (int j = 0; j < 32; j++) {
                if ((i + j) % 11 == 0) {
                    dynamic_mask |= (1U << j);
                }
            }
            __m512bh v2 = _mm512_mask_blend_epi16(dynamic_mask, v1, va);
            
            __m512bh v3 = _mm512_mask_blend_epi16(const_mask2, v2, vb);
            
            _mm512_store_si512((__m512i*)&dstb[i], _mm512_castpbh_si512(v3));
            
            asm volatile("" : "+v"(va), "+v"(vb), "+v"(v1), "+v"(v2), "+v"(v3) : : "memory");
#else
            // Fallback: use integer blend
            __m512i v1 = _mm512_mask_blend_epi16(const_mask1, va_int, vb_int);
            
            __mmask32 dynamic_mask = 0;
            for (int j = 0; j < 32; j++) {
                if ((i + j) % 11 == 0) {
                    dynamic_mask |= (1U << j);
                }
            }
            __m512i v2 = _mm512_mask_blend_epi16(dynamic_mask, v1, va_int);
            
            __m512i v3 = _mm512_mask_blend_epi16(const_mask2, v2, vb_int);
            
            _mm512_store_si512((__m512i*)&dstb[i], v3);
            
            asm volatile("" : "+v"(va_int), "+v"(vb_int), "+v"(v1), "+v"(v2), "+v"(v3) : : "memory");
#endif
        }
        
        total_checksum += checksum_16(dstb, ARRAY_SIZE);
    }
#endif // __AVX512BF16__

    printf("Total checksum: %lu\n", total_checksum);
    return 0;
}
