/* avx512_blend_coverage.c
 * Test program to cover AVX-512 blend expansion in GCC's i386-expand.cc
 * Compile with: gcc -O3 -mavx512f -mavx512bw -mavx512fp16 -march=native -o test test.c
 */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __AVX512F__

/* Helper function to create alternating mask patterns */
static __mmask64 create_mask64(int pattern) {
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if ((pattern == 0 && (i % 2 == 0)) ||    /* alternating */
            (pattern == 1 && (i % 4 < 2)) ||     /* 2 on, 2 off */
            (pattern == 2)) {                    /* all ones */
            mask |= (1ULL << i);
        }
    }
    return mask;
}

static __mmask32 create_mask32(int pattern) {
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if ((pattern == 0 && (i % 2 == 0)) ||
            (pattern == 1 && (i % 4 < 2)) ||
            (pattern == 2)) {
            mask |= (1U << i);
        }
    }
    return mask;
}

static __mmask16 create_mask16(int pattern) {
    __mmask16 mask = 0;
    for (int i = 0; i < 16; i++) {
        if ((pattern == 0 && (i % 2 == 0)) ||
            (pattern == 1 && (i % 4 < 2)) ||
            (pattern == 2)) {
            mask |= (1 << i);
        }
    }
    return mask;
}

static __mmask8 create_mask8(int pattern) {
    __mmask8 mask = 0;
    for (int i = 0; i < 8; i++) {
        if ((pattern == 0 && (i % 2 == 0)) ||
            (pattern == 1 && (i % 4 < 2)) ||
            (pattern == 2)) {
            mask |= (1 << i);
        }
    }
    return mask;
}

/* V64QImode: 64 x 8-bit integers */
#ifdef __AVX512BW__
__attribute__((noinline))
uint64_t test_v64qimode_blend() {
    /* Initialize arrays with distinct patterns */
    uint8_t a[64], b[64];
    for (int i = 0; i < 64; i++) {
        a[i] = i;           /* 0, 1, 2, ... */
        b[i] = 255 - i;     /* 255, 254, 253, ... */
    }
    
    __m512i va = _mm512_loadu_si512((const __m512i*)a);
    __m512i vb = _mm512_loadu_si512((const __m512i*)b);
    
    /* Create mask using comparison */
    __m512i cmp_val = _mm512_set1_epi8(128);
    __mmask64 mask = _mm512_cmpgt_epi8_mask(va, cmp_val);
    
    /* Force blend operation - should generate vblendmb */
    __m512i result = _mm512_mask_blend_epi8(mask, va, vb);
    
    /* Process result to prevent elimination */
    uint8_t res_arr[64];
    _mm512_storeu_si512((__m512i*)res_arr, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += res_arr[i];
    }
    return sum;
}
#endif

/* V32HImode: 32 x 16-bit integers */
#ifdef __AVX512BW__
__attribute__((noinline))
uint64_t test_v32himode_blend() {
    uint16_t a[32], b[32];
    for (int i = 0; i < 32; i++) {
        a[i] = i * 100;
        b[i] = 65535 - i * 100;
    }
    
    __m512i va = _mm512_loadu_si512((const __m512i*)a);
    __m512i vb = _mm512_loadu_si512((const __m512i*)b);
    
    /* Create mask - should generate vblendmw */
    __mmask32 mask = create_mask32(0);  /* alternating pattern */
    __m512i result = _mm512_mask_blend_epi16(mask, va, vb);
    
    uint16_t res_arr[32];
    _mm512_storeu_si512((__m512i*)res_arr, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += res_arr[i];
    }
    return sum;
}
#endif

/* V32HFmode: 32 x half-precision floats */
#ifdef __AVX512FP16__
__attribute__((noinline))
float test_v32hfmode_blend() {
    /* Use _Float16 type for half precision */
    _Float16 a[32], b[32];
    for (int i = 0; i < 32; i++) {
        a[i] = (_Float16)(i * 0.5f);
        b[i] = (_Float16)(10.0f - i * 0.5f);
    }
    
    __m512h va = _mm512_loadu_ph(a);
    __m512h vb = _mm512_loadu_ph(b);
    
    /* Create mask using comparison - should generate vblendmps for half */
    __m512h cmp_val = _mm512_set1_ph(5.0f);
    __mmask32 mask = _mm512_cmp_ph_mask(va, cmp_val, _CMP_GT_OQ);
    
    /* Blend operation */
    __m512h result = _mm512_mask_blend_ph(mask, va, vb);
    
    _Float16 res_arr[32];
    _mm512_storeu_ph(res_arr, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)res_arr[i];
    }
    return sum;
}
#endif

/* V32BFmode: 32 x bfloat16 */
#ifdef __AVX512BF16__
#ifdef __AVX512FP16__
__attribute__((noinline))
float test_v32bfmode_blend() {
    /* Use __m512bh for bfloat16 */
    __m512bh a, b;
    
    /* Initialize with float values and convert to bfloat16 */
    float fa[16], fb[16];
    for (int i = 0; i < 16; i++) {
        fa[i] = i * 0.7f;
        fb[i] = 15.0f - i * 0.7f;
    }
    
    /* Convert to bfloat16 - requires AVX512BF16 */
    a = _mm512_cvtneps_pbh(_mm512_loadu_ps(fa));
    b = _mm512_cvtneps_pbh(_mm512_loadu_ps(fb));
    
    /* Create mask - use comparison on original floats */
    __m512 cmp_val = _mm512_set1_ps(5.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(_mm512_loadu_ps(fa), cmp_val, _CMP_GT_OQ);
    
    /* Blend bfloat16 vectors - compiler should generate appropriate blend */
    __m512bh result = _mm512_mask_blend_epi32(mask, a, b);
    
    /* Convert back to float for verification */
    __m512 float_result = _mm512_cvtpbh_ps(result);
    
    float res_arr[16];
    _mm512_storeu_ps(res_arr, float_result);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += res_arr[i];
    }
    return sum;
}
#endif
#endif

/* V16SImode: 16 x 32-bit integers */
__attribute__((noinline))
uint64_t test_v16simode_blend() {
    int32_t a[16], b[16];
    for (int i = 0; i < 16; i++) {
        a[i] = i * 1000;
        b[i] = -i * 1000;
    }
    
    __m512i va = _mm512_loadu_si512((const __m512i*)a);
    __m512i vb = _mm512_loadu_si512((const __m512i*)b);
    
    /* Create mask - should generate vblendmd */
    __mmask16 mask = create_mask16(1);  /* 2 on, 2 off pattern */
    __m512i result = _mm512_mask_blend_epi32(mask, va, vb);
    
    int32_t res_arr[16];
    _mm512_storeu_si512((__m512i*)res_arr, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += (uint64_t)abs(res_arr[i]);
    }
    return sum;
}

/* V8DImode: 8 x 64-bit integers */
__attribute__((noinline))
uint64_t test_v8dimode_blend() {
    int64_t a[8], b[8];
    for (int i = 0; i < 8; i++) {
        a[i] = i * 10000LL;
        b[i] = -i * 10000LL;
    }
    
    __m512i va = _mm512_loadu_si512((const __m512i*)a);
    __m512i vb = _mm512_loadu_si512((const __m512i*)b);
    
    /* Create mask - should generate vblendmq */
    __mmask8 mask = create_mask8(0);  /* alternating pattern */
    __m512i result = _mm512_mask_blend_epi64(mask, va, vb);
    
    int64_t res_arr[8];
    _mm512_storeu_si512((__m512i*)res_arr, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (uint64_t)llabs(res_arr[i]);
    }
    return sum;
}

/* V8DFmode: 8 x double-precision floats */
__attribute__((noinline))
double test_v8dfmode_blend() {
    double a[8], b[8];
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.5;
        b[i] = 12.0 - i * 1.5;
    }
    
    __m512d va = _mm512_loadu_pd(a);
    __m512d vb = _mm512_loadu_pd(b);
    
    /* Create mask using comparison - should generate vblendmpd */
    __m512d cmp_val = _mm512_set1_pd(6.0);
    __mmask8 mask = _mm512_cmp_pd_mask(va, cmp_val, _CMP_GT_OQ);
    
    /* Blend operation */
    __m512d result = _mm512_mask_blend_pd(mask, va, vb);
    
    double res_arr[8];
    _mm512_storeu_pd(res_arr, result);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += res_arr[i];
    }
    return sum;
}

/* V16SFmode: 16 x single-precision floats */
__attribute__((noinline))
float test_v16sfmode_blend() {
    float a[16], b[16];
    for (int i = 0; i < 16; i++) {
        a[i] = i * 0.7f;
        b[i] = 15.0f - i * 0.7f;
    }
    
    __m512 va = _mm512_loadu_ps(a);
    __m512 vb = _mm512_loadu_ps(b);
    
    /* Create mask using comparison - should generate vblendmps */
    __m512 cmp_val = _mm512_set1_ps(5.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(va, cmp_val, _CMP_GT_OQ);
    
    /* Blend operation */
    __m512 result = _mm512_mask_blend_ps(mask, va, vb);
    
    float res_arr[16];
    _mm512_storeu_ps(res_arr, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += res_arr[i];
    }
    return sum;
}

#endif /* __AVX512F__ */

int main() {
    uint64_t total = 0;
    float float_total = 0.0f;
    double double_total = 0.0;
    
#ifdef __AVX512F__
    printf("Testing AVX-512 blend operations...\n");
    
    /* Test each vector mode */
    total += test_v16simode_blend();
    printf("V16SImode test completed\n");
    
    total += test_v8dimode_blend();
    printf("V8DImode test completed\n");
    
    double_total += test_v8dfmode_blend();
    printf("V8DFmode test completed\n");
    
    float_total += test_v16sfmode_blend();
    printf("V16SFmode test completed\n");
    
#ifdef __AVX512BW__
    total += test_v64qimode_blend();
    printf("V64QImode test completed\n");
    
    total += test_v32himode_blend();
    printf("V32HImode test completed\n");
#endif
    
#ifdef __AVX512FP16__
    float_total += test_v32hfmode_blend();
    printf("V32HFmode test completed\n");
    
#ifdef __AVX512BF16__
    float_total += test_v32bfmode_blend();
    printf("V32BFmode test completed\n");
#endif
#endif
    
    printf("\nResults:\n");
    printf("Integer total: %lu\n", total);
    printf("Float total: %f\n", float_total);
    printf("Double total: %f\n", double_total);
    
    /* Return non-zero if any test failed (simplified check) */
    if (total == 0 && float_total == 0.0f && double_total == 0.0) {
        return 1;  /* Likely all tests were optimized away */
    }
#else
    printf("AVX-512 not supported on this compiler/platform\n");
    return 1;
#endif
    
    return 0;
}
