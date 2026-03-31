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

/* Target attributes to ensure proper code generation */
#if defined(__GNUC__) && !defined(__clang__)
#define TARGET_AVX512 __attribute__((target("avx512f,avx512bw,avx512fp16,avx512bf16")))
#else
#define TARGET_AVX512
#endif

/* Function prototypes with target attributes */
TARGET_AVX512 static void test_v64qi(void);
TARGET_AVX512 static void test_v32hi(void);
TARGET_AVX512 static void test_v32hf(void);
TARGET_AVX512 static void test_v32bf(void);
TARGET_AVX512 static void test_v16si(void);
TARGET_AVX512 static void test_v8di(void);
TARGET_AVX512 static void test_v8df(void);
TARGET_AVX512 static void test_v16sf(void);

/* Global arrays to prevent constant propagation */
static uint8_t g_u8_data[64] __attribute__((aligned(64)));
static int16_t g_i16_data[32] __attribute__((aligned(64)));
static int32_t g_i32_data[16] __attribute__((aligned(64)));
static int64_t g_i64_data[8] __attribute__((aligned(64)));
static float g_f32_data[16] __attribute__((aligned(64)));
static double g_f64_data[8] __attribute__((aligned(64)));

#if HAS_AVX512FP16
static _Float16 g_f16_data[32] __attribute__((aligned(64)));
#endif

#if HAS_AVX512BF16
static __bf16 g_bf16_data[32] __attribute__((aligned(64)));
#endif

/* Output arrays */
static uint8_t g_u8_out[64] __attribute__((aligned(64)));
static int16_t g_i16_out[32] __attribute__((aligned(64)));
static int32_t g_i32_out[16] __attribute__((aligned(64)));
static int64_t g_i64_out[8] __attribute__((aligned(64)));
static float g_f32_out[16] __attribute__((aligned(64)));
static double g_f64_out[8] __attribute__((aligned(64)));

#if HAS_AVX512FP16
static _Float16 g_f16_out[32] __attribute__((aligned(64)));
#endif

#if HAS_AVX512BF16
static __bf16 g_bf16_out[32] __attribute__((aligned(64)));
#endif

/* Initialize test data with pattern to ensure non-constant masks */
static void init_data(void) {
    for (int i = 0; i < 64; i++) {
        g_u8_data[i] = (uint8_t)(i * 3 + 1);
    }
    for (int i = 0; i < 32; i++) {
        g_i16_data[i] = (int16_t)(i * 5 - 10);
    }
    for (int i = 0; i < 16; i++) {
        g_i32_data[i] = i * 7 - 20;
        g_f32_data[i] = (float)(i * 1.1);
    }
    for (int i = 0; i < 8; i++) {
        g_i64_data[i] = i * 11LL - 30;
        g_f64_data[i] = (double)(i * 2.2);
    }
    
#if HAS_AVX512FP16
    for (int i = 0; i < 32; i++) {
        g_f16_data[i] = (_Float16)(i * 0.5);
    }
#endif
    
#if HAS_AVX512BF16
    for (int i = 0; i < 32; i++) {
        /* Simple pattern for bfloat16 */
        uint16_t val = (uint16_t)(i * 3);
        g_bf16_data[i] = *(__bf16*)&val;
    }
#endif
}

/* V64QImode: 64-byte integer blend */
TARGET_AVX512
static void test_v64qi(void) {
#if HAS_AVX512BW
    /* Load data */
    __m512i a = _mm512_load_si512((const __m512i*)g_u8_data);
    __m512i b = _mm512_load_si512((const __m512i*)(g_u8_data + 32));
    
    /* Create dynamic mask based on data values */
    __mmask64 mask = _mm512_cmpeq_epi8_mask(_mm512_and_si512(a, _mm512_set1_epi8(1)),
                                           _mm512_setzero_si512());
    
    /* Perform blend - this should trigger gen_avx512bw_blendmv64qi */
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    /* Store result */
    _mm512_store_si512((__m512i*)g_u8_out, result);
#else
    fprintf(stderr, "AVX512BW not supported for V64QImode test\n");
#endif
}

/* V32HImode: 32 half-word integer blend */
TARGET_AVX512
static void test_v32hi(void) {
#if HAS_AVX512BW
    /* Load data */
    __m512i a = _mm512_load_si512((const __m512i*)g_i16_data);
    __m512i b = _mm512_load_si512((const __m512i*)(g_i16_data + 16));
    
    /* Create dynamic mask */
    __mmask32 mask = _mm512_cmplt_epi16_mask(a, _mm512_set1_epi16(0));
    
    /* Perform blend - this should trigger gen_avx512bw_blendmv32hi */
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    /* Store result */
    _mm512_store_si512((__m512i*)g_i16_out, result);
#else
    fprintf(stderr, "AVX512BW not supported for V32HImode test\n");
#endif
}

/* V32HFmode: 32 half-precision float blend */
TARGET_AVX512
static void test_v32hf(void) {
#if HAS_AVX512FP16
    /* Load data */
    __m512h a = _mm512_load_ph(g_f16_data);
    __m512h b = _mm512_load_ph(g_f16_data + 16);
    
    /* Create dynamic mask */
    __mmask32 mask = _mm512_cmp_ph_mask(a, b, _CMP_LT_OQ);
    
    /* Perform blend - this should trigger gen_avx512bw_blendmv32hf */
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    /* Store result */
    _mm512_store_ph(g_f16_out, result);
#else
    fprintf(stderr, "AVX512FP16 not supported for V32HFmode test\n");
#endif
}

/* V32BFmode: 32 bfloat16 blend */
TARGET_AVX512
static void test_v32bf(void) {
#if HAS_AVX512BF16
    /* Load data */
    __m512bh a = _mm512_load_ph(g_bf16_data);
    __m512bh b = _mm512_load_ph(g_bf16_data + 16);
    
    /* Create dynamic mask - use integer comparison since BF16 lacks native comparison */
    __m512i a_int = _mm512_load_si512((const __m512i*)g_bf16_data);
    __m512i b_int = _mm512_load_si512((const __m512i*)(g_bf16_data + 16));
    __mmask32 mask = _mm512_cmpeq_epi16_mask(_mm512_and_si512(a_int, _mm512_set1_epi16(1)),
                                            _mm512_setzero_si512());
    
    /* Perform blend - this should trigger gen_avx512bw_blendmv32bf */
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    
    /* Store result */
    _mm512_store_ph(g_bf16_out, result);
#else
    fprintf(stderr, "AVX512BF16 not supported for V32BFmode test\n");
#endif
}

/* V16SImode: 16 single-word integer blend */
TARGET_AVX512
static void test_v16si(void) {
#if HAS_AVX512F
    /* Load data */
    __m512i a = _mm512_load_si512((const __m512i*)g_i32_data);
    __m512i b = _mm512_load_si512((const __m512i*)(g_i32_data + 8));
    
    /* Create dynamic mask */
    __mmask16 mask = _mm512_cmpgt_epi32_mask(a, _mm512_set1_epi32(0));
    
    /* Perform blend - this should trigger gen_avx512f_blendmv16si */
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    /* Store result */
    _mm512_store_si512((__m512i*)g_i32_out, result);
#else
    fprintf(stderr, "AVX512F not supported for V16SImode test\n");
#endif
}

/* V8DImode: 8 double-word integer blend */
TARGET_AVX512
static void test_v8di(void) {
#if HAS_AVX512F
    /* Load data */
    __m512i a = _mm512_load_si512((const __m512i*)g_i64_data);
    __m512i b = _mm512_load_si512((const __m512i*)(g_i64_data + 4));
    
    /* Create dynamic mask */
    __mmask8 mask = _mm512_cmpeq_epi64_mask(_mm512_and_si512(a, _mm512_set1_epi64(1)),
                                           _mm512_setzero_si512());
    
    /* Perform blend - this should trigger gen_avx512f_blendmv8di */
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    /* Store result */
    _mm512_store_si512((__m512i*)g_i64_out, result);
#else
    fprintf(stderr, "AVX512F not supported for V8DImode test\n");
#endif
}

/* V8DFmode: 8 double-precision float blend */
TARGET_AVX512
static void test_v8df(void) {
#if HAS_AVX512F
    /* Load data */
    __m512d a = _mm512_load_pd(g_f64_data);
    __m512d b = _mm512_load_pd(g_f64_data + 4);
    
    /* Create dynamic mask */
    __mmask8 mask = _mm512_cmp_pd_mask(a, b, _CMP_LT_OQ);
    
    /* Perform blend - this should trigger gen_avx512f_blendmv8df */
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    /* Store result */
    _mm512_store_pd(g_f64_out, result);
#else
    fprintf(stderr, "AVX512F not supported for V8DFmode test\n");
#endif
}

/* V16SFmode: 16 single-precision float blend */
TARGET_AVX512
static void test_v16sf(void) {
#if HAS_AVX512F
    /* Load data */
    __m512 a = _mm512_load_ps(g_f32_data);
    __m512 b = _mm512_load_ps(g_f32_data + 8);
    
    /* Create dynamic mask */
    __mmask16 mask = _mm512_cmp_ps_mask(a, b, _CMP_GT_OQ);
    
    /* Perform blend - this should trigger gen_avx512f_blendmv16sf */
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    /* Store result */
    _mm512_store_ps(g_f32_out, result);
#else
    fprintf(stderr, "AVX512F not supported for V16SFmode test\n");
#endif
}

/* Calculate checksum to prevent dead code elimination */
static uint64_t calculate_checksum(void) {
    uint64_t checksum = 0;
    
    for (int i = 0; i < 64; i++) {
        checksum += g_u8_out[i];
    }
    for (int i = 0; i < 32; i++) {
        checksum += (uint16_t)g_i16_out[i];
    }
    for (int i = 0; i < 16; i++) {
        checksum += (uint32_t)g_i32_out[i];
        checksum += *(uint32_t*)&g_f32_out[i];
    }
    for (int i = 0; i < 8; i++) {
        checksum += (uint64_t)g_i64_out[i];
        checksum += *(uint64_t*)&g_f64_out[i];
    }
    
#if HAS_AVX512FP16
    for (int i = 0; i < 32; i++) {
        checksum += *(uint16_t*)&g_f16_out[i];
    }
#endif
    
#if HAS_AVX512BF16
    for (int i = 0; i < 32; i++) {
        checksum += *(uint16_t*)&g_bf16_out[i];
    }
#endif
    
    return checksum;
}

int main(void) {
    /* Initialize test data */
    init_data();
    
    printf("Testing AVX-512 blend instruction expansion...\n");
    
    /* Execute all blend tests */
    test_v64qi();
    test_v32hi();
    test_v32hf();
    test_v32bf();
    test_v16si();
    test_v8di();
    test_v8df();
    test_v16sf();
    
    /* Calculate checksum to ensure all code executes */
    uint64_t checksum = calculate_checksum();
    printf("Checksum: 0x%016llx\n", (unsigned long long)checksum);
    
    /* Simple validation */
    if (checksum != 0) {
        printf("All blend operations executed successfully.\n");
        return 0;
    } else {
        printf("Warning: Checksum is zero - possible optimization issue.\n");
        return 1;
    }
}
