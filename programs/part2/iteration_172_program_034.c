/* AVX-512 Blend Coverage Test for i386-expand.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

/* Compile-time feature checks */
#ifdef __AVX512F__
#define HAS_AVX512F 1
#else
#define HAS_AVX512F 0
#endif

#ifdef __AVX512BW__
#define HAS_AVX512BW 1
#else
#define HAS_AVX512BW 0
#endif

#ifdef __AVX512FP16__
#define HAS_AVX512FP16 1
#else
#define HAS_AVX512FP16 0
#endif

#ifdef __AVX512BF16__
#define HAS_AVX512BF16 1
#else
#define HAS_AVX512BF16 0
#endif

/* Force generation of specific instructions with target attributes */
#ifdef __GNUC__
#define TARGET_AVX512F __attribute__((target("avx512f")))
#define TARGET_AVX512BW __attribute__((target("avx512bw")))
#define TARGET_AVX512FP16 __attribute__((target("avx512f,avx512bw,avx512fp16")))
#define TARGET_AVX512BF16 __attribute__((target("avx512f,avx512bw,avx512bf16")))
#else
#define TARGET_AVX512F
#define TARGET_AVX512BW
#define TARGET_AVX512FP16
#define TARGET_AVX512BF16
#endif

/* Test data initialization */
static void init_test_data(void) {
    /* Data will be initialized in main() */
}

/* V64QImode test - requires AVX512BW */
TARGET_AVX512BW
static uint64_t test_v64qimode(void) {
    uint64_t checksum = 0;
    
#if HAS_AVX512BW
    /* Create two different 512-bit vectors */
    __m512i a = _mm512_set1_epi8(0x11);
    __m512i b = _mm512_set1_epi8(0x22);
    
    /* Generate dynamic mask based on position */
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if (i % 3 == 0) {
            mask |= (1ULL << i);
        }
    }
    
    /* This should trigger gen_avx512bw_blendmv64qi */
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    /* Extract bytes to compute checksum */
    alignas(64) uint8_t out[64];
    _mm512_store_si512((__m512i*)out, result);
    
    for (int i = 0; i < 64; i++) {
        checksum += out[i];
    }
#endif
    
    return checksum;
}

/* V32HImode test - requires AVX512BW */
TARGET_AVX512BW
static uint64_t test_v32himode(void) {
    uint64_t checksum = 0;
    
#if HAS_AVX512BW
    __m512i a = _mm512_set1_epi16(0x1111);
    __m512i b = _mm512_set1_epi16(0x2222);
    
    /* Dynamic mask: select every other element */
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if (i % 2 == 0) {
            mask |= (1U << i);
        }
    }
    
    /* This should trigger gen_avx512bw_blendmv32hi */
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    alignas(64) uint16_t out[32];
    _mm512_store_si512((__m512i*)out, result);
    
    for (int i = 0; i < 32; i++) {
        checksum += out[i];
    }
#endif
    
    return checksum;
}

/* V32HFmode test - requires AVX512-FP16 */
TARGET_AVX512FP16
static float test_v32hfmode(void) {
    float checksum = 0.0f;
    
#if HAS_AVX512FP16
    /* Initialize with different patterns */
    __m512h a = _mm512_set1_ph(1.0f);
    __m512h b = _mm512_set1_ph(2.0f);
    
    /* Create dynamic mask */
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if (i % 4 == 0) {
            mask |= (1U << i);
        }
    }
    
    /* This should trigger gen_avx512bw_blendmv32hf */
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    alignas(64) _Float16 out[32];
    _mm512_store_ph(out, result);
    
    for (int i = 0; i < 32; i++) {
        checksum += (float)out[i];
    }
#endif
    
    return checksum;
}

/* V32BFmode test - requires AVX512-BF16 */
TARGET_AVX512BF16
static float test_v32bfmode(void) {
    float checksum = 0.0f;
    
#if HAS_AVX512BF16
    /* Initialize bfloat16 vectors */
    __m512bh a = _mm512_set1_epi16(0x3F80); /* bfloat16 1.0 */
    __m512bh b = _mm512_set1_epi16(0x4000); /* bfloat16 2.0 */
    
    /* Dynamic mask pattern */
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if (i % 5 == 0) {
            mask |= (1U << i);
        }
    }
    
    /* This should trigger gen_avx512bw_blendmv32bf */
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    
    alignas(64) __bf16 out[32];
    _mm512_store_ph((void*)out, result);
    
    for (int i = 0; i < 32; i++) {
        /* Simple accumulation - actual bfloat16 conversion would be more complex */
        checksum += (float)out[i];
    }
#endif
    
    return checksum;
}

/* V16SImode test - requires AVX512F */
TARGET_AVX512F
static uint64_t test_v16simode(void) {
    uint64_t checksum = 0;
    
#if HAS_AVX512F
    __m512i a = _mm512_set1_epi32(0x11111111);
    __m512i b = _mm512_set1_epi32(0x22222222);
    
    /* Create mask with alternating pattern */
    __mmask16 mask = 0xAAAA; /* 1010101010101010 binary */
    
    /* This should trigger gen_avx512f_blendmv16si */
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    alignas(64) int32_t out[16];
    _mm512_store_si512((__m512i*)out, result);
    
    for (int i = 0; i < 16; i++) {
        checksum += (uint64_t)out[i];
    }
#endif
    
    return checksum;
}

/* V8DImode test - requires AVX512F */
TARGET_AVX512F
static uint64_t test_v8dimode(void) {
    uint64_t checksum = 0;
    
#if HAS_AVX512F
    __m512i a = _mm512_set1_epi64(0x1111111111111111ULL);
    __m512i b = _mm512_set1_epi64(0x2222222222222222ULL);
    
    /* Dynamic mask based on prime pattern */
    __mmask8 mask = 0;
    for (int i = 0; i < 8; i++) {
        if (i == 0 || i == 1 || i == 2 || i == 3 || i == 5 || i == 7) {
            mask |= (1U << i);
        }
    }
    
    /* This should trigger gen_avx512f_blendmv8di */
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    alignas(64) int64_t out[8];
    _mm512_store_si512((__m512i*)out, result);
    
    for (int i = 0; i < 8; i++) {
        checksum += (uint64_t)out[i];
    }
#endif
    
    return checksum;
}

/* V8DFmode test - requires AVX512F */
TARGET_AVX512F
static double test_v8dfmode(void) {
    double checksum = 0.0;
    
#if HAS_AVX512F
    __m512d a = _mm512_set1_pd(1.0);
    __m512d b = _mm512_set1_pd(2.0);
    
    /* Mask: select first 4 elements from a, last 4 from b */
    __mmask8 mask = 0x0F; /* 00001111 binary */
    
    /* This should trigger gen_avx512f_blendmv8df */
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    alignas(64) double out[8];
    _mm512_store_pd(out, result);
    
    for (int i = 0; i < 8; i++) {
        checksum += out[i];
    }
#endif
    
    return checksum;
}

/* V16SFmode test - requires AVX512F */
TARGET_AVX512F
static float test_v16sfmode(void) {
    float checksum = 0.0f;
    
#if HAS_AVX512F
    __m512 a = _mm512_set1_ps(1.0f);
    __m512 b = _mm512_set1_ps(2.0f);
    
    /* Create checkerboard mask */
    __mmask16 mask = 0x5555; /* 0101010101010101 binary */
    
    /* This should trigger gen_avx512f_blendmv16sf */
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    alignas(64) float out[16];
    _mm512_store_ps(out, result);
    
    for (int i = 0; i < 16; i++) {
        checksum += out[i];
    }
#endif
    
    return checksum;
}

/* Main test driver */
int main(void) {
    uint64_t total_checksum = 0;
    
    printf("AVX-512 Blend Coverage Test\n");
    printf("===========================\n");
    
    /* Test each vector mode */
#if HAS_AVX512BW
    printf("Testing V64QImode...\n");
    total_checksum += test_v64qimode();
    
    printf("Testing V32HImode...\n");
    total_checksum += test_v32himode();
#endif
    
#if HAS_AVX512FP16
    printf("Testing V32HFmode...\n");
    total_checksum += (uint64_t)test_v32hfmode();
#endif
    
#if HAS_AVX512BF16
    printf("Testing V32BFmode...\n");
    total_checksum += (uint64_t)test_v32bfmode();
#endif
    
#if HAS_AVX512F
    printf("Testing V16SImode...\n");
    total_checksum += test_v16simode();
    
    printf("Testing V8DImode...\n");
    total_checksum += test_v8dimode();
    
    printf("Testing V8DFmode...\n");
    total_checksum += (uint64_t)test_v8dfmode();
    
    printf("Testing V16SFmode...\n");
    total_checksum += (uint64_t)test_v16sfmode();
#endif
    
    printf("\nTotal checksum: %llu\n", (unsigned long long)total_checksum);
    printf("Test completed.\n");
    
    return 0;
}
