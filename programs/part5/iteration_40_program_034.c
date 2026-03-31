/* AVX-512 Vector Blend Coverage Test
 * Compile with: gcc -O3 -mavx512f -mavx512bw -mavx512fp16 -march=native -o avx512_blend_test avx512_blend_test.c
 */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifdef __AVX512F__

/* Helper function to print results for debugging */
static void print_hex(const void* data, size_t size) {
    const unsigned char* p = (const unsigned char*)data;
    for (size_t i = 0; i < size; i++) {
        printf("%02x", p[i]);
        if ((i + 1) % 16 == 0) printf("\n");
        else if ((i + 1) % 8 == 0) printf(" ");
    }
    printf("\n");
}

/* V64QImode: 64 x 8-bit integers */
#ifdef __AVX512BW__
__attribute__((noinline))
uint64_t test_v64qimode_blend() {
    __m512i a = _mm512_set1_epi8(0xAA);  /* 10101010 pattern */
    __m512i b = _mm512_set1_epi8(0x55);  /* 01010101 pattern */
    
    /* Create alternating mask: 0xAAAAAAAAAAAAAAAA */
    __mmask64 mask = 0xAAAAAAAAAAAAAAAAULL;
    
    /* This should generate vblendmb */
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    /* Force usage of result */
    volatile __m512i volatile_result = result;
    
    /* Compute checksum */
    alignas(64) uint8_t arr[64];
    _mm512_store_si512(arr, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += arr[i];
    }
    return sum;
}
#endif

/* V32HImode: 32 x 16-bit integers */
#ifdef __AVX512BW__
__attribute__((noinline))
uint64_t test_v32himode_blend() {
    __m512i a = _mm512_set1_epi16(0xAAAA);  /* 0xAAAA pattern */
    __m512i b = _mm512_set1_epi16(0x5555);  /* 0x5555 pattern */
    
    /* Create alternating mask */
    __mmask32 mask = 0xAAAAAAAA;
    
    /* This should generate vblendmw */
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    /* Force usage */
    volatile __m512i volatile_result = result;
    
    alignas(64) uint16_t arr[32];
    _mm512_store_si512(arr, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += arr[i];
    }
    return sum;
}
#endif

/* V32HFmode: 32 x half-precision floats */
#ifdef __AVX512FP16__
__attribute__((noinline))
float test_v32hfmode_blend() {
    __m512h a = _mm512_set1_ph(1.0f);   /* All 1.0 */
    __m512h b = _mm512_set1_ph(2.0f);   /* All 2.0 */
    
    /* Create mask with alternating bits */
    __mmask32 mask = 0xAAAAAAAA;
    
    /* This should generate vblendmps for half-precision */
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    /* Force usage */
    volatile __m512h volatile_result = result;
    
    /* Compute sum */
    alignas(64) _Float16 arr[32];
    _mm512_store_ph(arr, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)arr[i];
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
    __m512bh a = _mm512_set1_epi16(0x3F80);  /* bfloat16 1.0 */
    __m512bh b = _mm512_set1_epi16(0x4000);  /* bfloat16 2.0 */
    
    __mmask32 mask = 0xAAAAAAAA;
    
    /* Blend bfloat16 values - may use vblendmps with conversion */
    __m512bh result = _mm512_mask_blend_epi16(mask, 
        (__m512i)a, (__m512i)b);
    
    volatile __m512bh volatile_result = result;
    
    alignas(64) uint16_t arr[32];
    _mm512_store_si512(arr, (__m512i)result);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        /* Convert bfloat16 to float */
        uint32_t t = arr[i] << 16;
        float f;
        memcpy(&f, &t, sizeof(float));
        sum += f;
    }
    return sum;
}
#endif
#endif

/* V16SImode: 16 x 32-bit integers */
__attribute__((noinline))
uint64_t test_v16simode_blend() {
    __m512i a = _mm512_set1_epi32(0xAAAAAAAA);
    __m512i b = _mm512_set1_epi32(0x55555555);
    
    __mmask16 mask = 0xAAAA;  /* Alternating bits */
    
    /* This should generate vblendmd */
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    volatile __m512i volatile_result = result;
    
    alignas(64) uint32_t arr[16];
    _mm512_store_si512(arr, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += arr[i];
    }
    return sum;
}

/* V8DImode: 8 x 64-bit integers */
__attribute__((noinline))
uint64_t test_v8dimode_blend() {
    __m512i a = _mm512_set1_epi64(0xAAAAAAAAAAAAAAAAULL);
    __m512i b = _mm512_set1_epi64(0x5555555555555555ULL);
    
    __mmask8 mask = 0xAA;  /* Alternating bits */
    
    /* This should generate vblendmq */
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    volatile __m512i volatile_result = result;
    
    alignas(64) uint64_t arr[8];
    _mm512_store_si512(arr, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += arr[i];
    }
    return sum;
}

/* V8DFmode: 8 x double-precision floats */
__attribute__((noinline))
double test_v8dfmode_blend() {
    __m512d a = _mm512_set1_pd(1.0);
    __m512d b = _mm512_set1_pd(2.0);
    
    __mmask8 mask = 0xAA;  /* Alternating bits */
    
    /* This should generate vblendmpd */
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    volatile __m512d volatile_result = result;
    
    alignas(64) double arr[8];
    _mm512_store_pd(arr, result);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += arr[i];
    }
    return sum;
}

/* V16SFmode: 16 x single-precision floats */
__attribute__((noinline))
float test_v16sfmode_blend() {
    __m512 a = _mm512_set1_ps(1.0f);
    __m512 b = _mm512_set1_ps(2.0f);
    
    __mmask16 mask = 0xAAAA;  /* Alternating bits */
    
    /* This should generate vblendmps */
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    volatile __m512 volatile_result = result;
    
    alignas(64) float arr[16];
    _mm512_store_ps(arr, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Main test driver */
int main() {
    uint64_t checksum = 0;
    float float_sum = 0.0f;
    double double_sum = 0.0;
    
    printf("Testing AVX-512 vector blend operations...\n");
    
#ifdef __AVX512BW__
    printf("Testing V64QImode (64x8-bit)...\n");
    checksum += test_v64qimode_blend();
    
    printf("Testing V32HImode (32x16-bit)...\n");
    checksum += test_v32himode_blend();
#endif
    
#ifdef __AVX512FP16__
    printf("Testing V32HFmode (32xhalf-precision)...\n");
    float_sum += test_v32hfmode_blend();
#endif

#ifdef __AVX512BF16__
#ifdef __AVX512FP16__
    printf("Testing V32BFmode (32xbfloat16)...\n");
    float_sum += test_v32bfmode_blend();
#endif
#endif
    
    printf("Testing V16SImode (16x32-bit)...\n");
    checksum += test_v16simode_blend();
    
    printf("Testing V8DImode (8x64-bit)...\n");
    checksum += test_v8dimode_blend();
    
    printf("Testing V8DFmode (8xdouble)...\n");
    double_sum += test_v8dfmode_blend();
    
    printf("Testing V16SFmode (16xsingle)...\n");
    float_sum += test_v16sfmode_blend();
    
    /* Use results to prevent optimization */
    printf("Checksum: %lu\n", checksum);
    printf("Float sum: %f\n", float_sum);
    printf("Double sum: %f\n", double_sum);
    
    /* Simple validation - just ensure we didn't crash */
    printf("All blend tests completed successfully!\n");
    
    return 0;
}

#else
int main() {
    printf("AVX-512 not supported on this compiler/platform.\n");
    return 0;
}
#endif
