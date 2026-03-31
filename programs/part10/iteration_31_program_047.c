/* test_avx512_blend.c - Coverage for i386-expand.cc lines 4303-4326 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

/* Alignment helper */
#define ALIGN_64 __attribute__((aligned(64)))

/* Prevent optimization */
static volatile int g_volatile_counter = 0;

#ifdef __AVX512F__

/* V16SF - 16 single-precision floats */
static float test_v16sf_blend(void) {
    ALIGN_64 float src1[16] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                                9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    ALIGN_64 float src2[16] = {100.0f, 200.0f, 300.0f, 400.0f, 500.0f, 600.0f, 700.0f, 800.0f,
                                900.0f, 1000.0f, 1100.0f, 1200.0f, 1300.0f, 1400.0f, 1500.0f, 1600.0f};
    ALIGN_64 volatile float dst[16];
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Create mask using comparison */
    __mmask16 mask = _mm512_cmp_ps_mask(v1, _mm512_set1_ps(8.0f), _CMP_LT_OQ);
    
    /* Blend with mask - should trigger gen_avx512f_blendmv16sf */
    __m512 blended = _mm512_mask_blend_ps(mask, v2, v1);
    
    /* Use in arithmetic operation */
    __m512 scaled = _mm512_mul_ps(blended, _mm512_set1_ps(2.0f));
    
    /* Blend with arithmetic result */
    __m512 final = _mm512_mask_blend_ps(mask ^ 0xFFFF, scaled, blended);
    
    _mm512_store_ps((void*)dst, final);
    
    /* Compute reduction */
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    
    /* Force side effect */
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    return sum;
}

/* V8DF - 8 double-precision floats */
static double test_v8df_blend(void) {
    ALIGN_64 double src1[8] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    ALIGN_64 double src2[8] = {100.0, 200.0, 300.0, 400.0, 500.0, 600.0, 700.0, 800.0};
    ALIGN_64 volatile double dst[8];
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Create mask - blend odd elements */
    __mmask8 mask = 0xAA; /* 0b10101010 */
    
    /* Blend with mask - should trigger gen_avx512f_blendmv8df */
    __m512d blended = _mm512_mask_blend_pd(mask, v2, v1);
    
    /* Blend with broadcast scalar */
    __m512d broadcast = _mm512_set1_pd(42.0);
    __m512d final = _mm512_mask_blend_pd(mask, blended, broadcast);
    
    _mm512_store_pd((void*)dst, final);
    
    /* Loop with volatile dependency */
    double sum = 0.0;
    for (int i = g_volatile_counter & 7; i < 8; i++) {
        sum += dst[i];
    }
    
    return sum;
}

/* V16SI - 16 32-bit integers */
static int32_t test_v16si_blend(void) {
    ALIGN_64 int32_t src1[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    ALIGN_64 int32_t src2[16] = {100, 200, 300, 400, 500, 600, 700, 800, 
                                  900, 1000, 1100, 1200, 1300, 1400, 1500, 1600};
    ALIGN_64 volatile int32_t dst[16];
    
    __m512i v1 = _mm512_load_epi32(src1);
    __m512i v2 = _mm512_load_epi32(src2);
    
    /* Create mask using comparison */
    __mmask16 mask = _mm512_cmp_epi32_mask(v1, _mm512_set1_epi32(5), _MM_CMPINT_LT);
    
    /* Blend with mask - should trigger gen_avx512f_blendmv16si */
    __m512i blended = _mm512_mask_blend_epi32(mask, v2, v1);
    
    /* Blend with arithmetic result */
    __m512i added = _mm512_add_epi32(blended, _mm512_set1_epi32(10));
    __m512i final = _mm512_mask_blend_epi32(mask ^ 0xFFFF, added, blended);
    
    _mm512_store_epi32((void*)dst, final);
    
    int32_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    
    return sum;
}

/* V8DI - 8 64-bit integers */
static int64_t test_v8di_blend(void) {
    ALIGN_64 int64_t src1[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    ALIGN_64 int64_t src2[8] = {100, 200, 300, 400, 500, 600, 700, 800};
    ALIGN_64 volatile int64_t dst[8];
    
    __m512i v1 = _mm512_load_epi64(src1);
    __m512i v2 = _mm512_load_epi64(src2);
    
    /* Patterned mask */
    __mmask8 mask = 0x55; /* 0b01010101 */
    
    /* Blend with mask - should trigger gen_avx512f_blendmv8di */
    __m512i blended = _mm512_mask_blend_epi64(mask, v2, v1);
    
    /* Blend with shifted version */
    __m512i shifted = _mm512_slli_epi64(blended, 1);
    __m512i final = _mm512_mask_blend_epi64(mask, blended, shifted);
    
    _mm512_store_epi64((void*)dst, final);
    
    int64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    
    return sum;
}

#endif /* __AVX512F__ */

#ifdef __AVX512BW__

/* V64QI - 64 8-bit integers */
static int8_t test_v64qi_blend(void) {
    ALIGN_64 int8_t src1[64];
    ALIGN_64 int8_t src2[64];
    ALIGN_64 volatile int8_t dst[64];
    
    /* Initialize with patterns */
    for (int i = 0; i < 64; i++) {
        src1[i] = i;
        src2[i] = 100 + i;
    }
    
    __m512i v1 = _mm512_load_si512(src1);
    __m512i v2 = _mm512_load_si512(src2);
    
    /* Create mask: blend where src1[i] < 32 */
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if (src1[i] < 32) {
            mask |= (1ULL << i);
        }
    }
    
    /* Blend with mask - should trigger gen_avx512bw_blendmv64qi */
    __m512i blended = _mm512_mask_blend_epi8(mask, v2, v1);
    
    /* Additional blend with broadcast */
    __m512i broadcast = _mm512_set1_epi8(0xFF);
    __m512i final = _mm512_mask_blend_epi8(mask, blended, broadcast);
    
    _mm512_store_si512((void*)dst, final);
    
    int8_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += dst[i];
    }
    
    return sum;
}

/* V32HI - 32 16-bit integers */
static int16_t test_v32hi_blend(void) {
    ALIGN_64 int16_t src1[32];
    ALIGN_64 int16_t src2[32];
    ALIGN_64 volatile int16_t dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 10;
        src2[i] = 1000 + i * 20;
    }
    
    __m512i v1 = _mm512_load_si512(src1);
    __m512i v2 = _mm512_load_si512(src2);
    
    /* Create mask using comparison */
    __mmask32 mask = _mm512_cmp_epi16_mask(v1, _mm512_set1_epi16(160), _MM_CMPINT_LT);
    
    /* Blend with mask - should trigger gen_avx512bw_blendmv32hi */
    __m512i blended = _mm512_mask_blend_epi16(mask, v2, v1);
    
    /* Blend with multiplied result */
    __m512i multiplied = _mm512_mullo_epi16(blended, _mm512_set1_epi16(2));
    __m512i final = _mm512_mask_blend_epi16(mask ^ 0xFFFFFFFF, multiplied, blended);
    
    _mm512_store_si512((void*)dst, final);
    
    int16_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    
    return sum;
}

/* V32HF - 32 half-precision floats */
static __m512h test_v32hf_blend(void) {
    ALIGN_64 uint16_t src1_data[32];
    ALIGN_64 uint16_t src2_data[32];
    ALIGN_64 volatile uint16_t dst_data[32];
    
    /* Initialize half-precision values (pattern) */
    for (int i = 0; i < 32; i++) {
        src1_data[i] = 0x3C00 | (i & 0x1F); /* ~1.0 with variations */
        src2_data[i] = 0x4000 | (i & 0x1F); /* ~2.0 with variations */
    }
    
    __m512h v1 = _mm512_load_ph(src1_data);
    __m512h v2 = _mm512_load_ph(src2_data);
    
    /* Create mask: blend even elements */
    __mmask32 mask = 0xAAAAAAAA; /* 0b10101010... */
    
    /* Blend with mask - should trigger gen_avx512bw_blendmv32hf */
    __m512h blended = _mm512_mask_blend_ph(mask, v2, v1);
    
    /* Additional operation and blend */
    __m512h added = _mm512_add_ph(blended, _mm512_set1_ph(1.0));
    __m512h final = _mm512_mask_blend_ph(mask, blended, added);
    
    _mm512_store_ph((void*)dst_data, final);
    
    /* Return as vector to avoid conversion issues */
    return final;
}

/* V32BF - 32 bfloat16 values */
static __m512bh test_v32bf_blend(void) {
    ALIGN_64 uint16_t src1_data[32];
    ALIGN_64 uint16_t src2_data[32];
    ALIGN_64 volatile uint16_t dst_data[32];
    
    /* Initialize bfloat16 values */
    for (int i = 0; i < 32; i++) {
        src1_data[i] = 0x3F80 | (i & 0x7); /* ~1.0f in bfloat16 */
        src2_data[i] = 0x4000 | (i & 0x7); /* ~2.0f in bfloat16 */
    }
    
    /* Load as integer vectors for blending */
    __m512i v1 = _mm512_load_si512(src1_data);
    __m512i v2 = _mm512_load_si512(src2_data);
    
    /* Create mask */
    __mmask32 mask = 0x55555555; /* 0b01010101... */
    
    /* Blend using epi16 intrinsic - should trigger gen_avx512bw_blendmv32bf */
    __m512i blended = _mm512_mask_blend_epi16(mask, v2, v1);
    
    /* Cast back to bfloat16 vector for potential use */
    __m512bh result = _mm512_castsi512_bh(blended);
    
    _mm512_store_si512((void*)dst_data, blended);
    
    return result;
}

#endif /* __AVX512BW__ */

int main(int argc, char *argv[]) {
    uint64_t checksum = 0;
    
    /* Use argc to create runtime variability */
    int iterations = (argc > 1) ? (argv[1][0] % 4) + 1 : 2;
    
#ifdef __AVX512F__
    printf("Testing AVX-512F blend patterns...\n");
    
    for (int i = 0; i < iterations; i++) {
        g_volatile_counter = i;
        
        /* V16SF */
        float fsum = test_v16sf_blend();
        checksum += *(uint32_t*)&fsum;
        
        /* V8DF */
        double dsum = test_v8df_blend();
        checksum += *(uint64_t*)&dsum;
        
        /* V16SI */
        int32_t isum = test_v16si_blend();
        checksum += (uint32_t)isum;
        
        /* V8DI */
        int64_t lsum = test_v8di_blend();
        checksum += (uint64_t)lsum;
    }
#else
    printf("AVX-512F not supported at compile time\n");
#endif

#ifdef __AVX512BW__
    printf("Testing AVX-512BW blend patterns...\n");
    
    for (int i = 0; i < iterations; i++) {
        /* V64QI */
        int8_t bsum = test_v64qi_blend();
        checksum += (uint8_t)bsum;
        
        /* V32HI */
        int16_t ssum = test_v32hi_blend();
        checksum += (uint16_t)ssum;
        
        /* V32HF */
        __m512h hvec = test_v32hf_blend();
        /* Access elements to ensure computation */
        ALIGN_64 uint16_t htemp[32];
        _mm512_store_ph(htemp, hvec);
        for (int j = 0; j < 32; j++) {
            checksum += htemp[j];
        }
        
        /* V32BF */
        __m512bh bfvec = test_v32bf_blend();
        ALIGN_64 uint16_t bftemp[32];
        _mm512_store_si512(bftemp, _mm512_castbh_si512(bfvec));
        for (int j = 0; j < 32; j++) {
            checksum += bftemp[j];
        }
    }
#else
    printf("AVX-512BW not supported at compile time\n");
#endif

    printf("Final checksum: 0x%016llx\n", (unsigned long long)checksum);
    
    /* Use checksum to affect return value */
    return (checksum & 0xFF) == 0 ? 0 : 1;
}
