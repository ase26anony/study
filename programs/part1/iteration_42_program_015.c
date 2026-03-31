#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>
#include <x86intrin.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* ==================== V64QImode (64-byte integers) ==================== */
static int test_v64qi_blend(void) {
    char a[64], b[64], result[64];
    __m512i va, vb, vresult;
    __mmask64 mask;
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        a[i] = i;
        b[i] = 64 - i;
    }
    
    va = _mm512_loadu_si512((__m512i*)a);
    vb = _mm512_loadu_si512((__m512i*)b);
    
    /* Generate dynamic mask: select a[i] if i % 2 == 0, else b[i] */
    __m512i pattern = _mm512_set_epi8(
        63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
        47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    __m512i mod_mask = _mm512_set1_epi8(1);
    __m512i mod_result = _mm512_and_si512(pattern, mod_mask);
    __m512i zero = _mm512_setzero_si512();
    mask = _mm512_cmpeq_epi8_mask(mod_result, zero);
    
    /* Critical blend operation for V64QImode */
    vresult = _mm512_mask_blend_epi8(mask, vb, va);
    
    _mm512_storeu_si512((__m512i*)result, vresult);
    
    /* Compute checksum to prevent elimination */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== V32HImode (32-halfword integers) ==================== */
static int test_v32hi_blend(void) {
    short a[32], b[32], result[32];
    __m512i va, vb, vresult;
    __mmask32 mask;
    
    for (int i = 0; i < 32; i++) {
        a[i] = i * 2;
        b[i] = i * 3;
    }
    
    va = _mm512_loadu_si512((__m512i*)a);
    vb = _mm512_loadu_si512((__m512i*)b);
    
    /* Generate mask based on comparison */
    __m512i threshold = _mm512_set1_epi16(32);
    mask = _mm512_cmpgt_epi16_mask(va, threshold);
    
    /* Critical blend operation for V32HImode */
    vresult = _mm512_mask_blend_epi16(mask, vb, va);
    
    _mm512_storeu_si512((__m512i*)result, vresult);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== V32HFmode (32-half-precision floats) ==================== */
#ifdef __AVX512FP16__
static int test_v32hf_blend(void) {
    _Float16 a[32], b[32], result[32];
    __m512h va, vb, vresult;
    __mmask32 mask;
    
    for (int i = 0; i < 32; i++) {
        a[i] = (_Float16)(i * 0.5f);
        b[i] = (_Float16)(i * 0.75f);
    }
    
    va = _mm512_loadu_ph(a);
    vb = _mm512_loadu_ph(b);
    
    /* Generate mask: select a where a > 8.0 */
    __m512h threshold = _mm512_set1_ph((_Float16)8.0f);
    mask = _mm512_cmp_ph_mask(va, threshold, _CMP_GT_OQ);
    
    /* Critical blend operation for V32HFmode */
    vresult = _mm512_mask_blend_ph(mask, vb, va);
    
    _mm512_storeu_ph(result, vresult);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (int)result[i];
    }
    return sum;
}
#endif

/* ==================== V32BFmode (32-bfloat16) ==================== */
#ifdef __AVX512BF16__
static int test_v32bf_blend(void) {
    __bfloat16 a[32], b[32], result[32];
    __m512bh va, vb, vresult;
    __mmask32 mask;
    
    for (int i = 0; i < 32; i++) {
        a[i] = bfloat16_from_float(i * 1.5f);
        b[i] = bfloat16_from_float(i * 2.0f);
    }
    
    va = _mm512_loadu_bf16(a);
    vb = _mm512_loadu_bf16(b);
    
    /* For bfloat16, we need to use integer blend since there's no direct bfloat16 blend */
    __m512i vai = _mm512_castsi512_si512(_mm512_loadu_si512((__m512i*)a));
    __m512i vbi = _mm512_castsi512_si512(_mm512_loadu_si512((__m512i*)b));
    
    /* Generate mask using comparison of integer representation */
    __m512i pattern = _mm512_set_epi16(
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    __m512i mod_mask = _mm512_set1_epi16(1);
    __m512i mod_result = _mm512_and_si512(pattern, mod_mask);
    mask = _mm512_cmpeq_epi16_mask(mod_result, _mm512_setzero_si512());
    
    /* Critical blend operation for V32BFmode (using epi16 blend) */
    __m512i vresulti = _mm512_mask_blend_epi16(mask, vbi, vai);
    
    _mm512_storeu_si512((__m512i*)result, vresulti);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += bfloat16_to_float(result[i]);
    }
    return sum;
}
#endif

/* ==================== V16SFmode (16-single-precision floats) ==================== */
static int test_v16sf_blend(void) {
    float a[16], b[16], result[16];
    __m512 va, vb, vresult;
    __mmask16 mask;
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 1.1f;
        b[i] = i * 2.2f;
    }
    
    va = _mm512_loadu_ps(a);
    vb = _mm512_loadu_ps(b);
    
    /* Generate mask: select a where a > 8.0 */
    __m512 threshold = _mm512_set1_ps(8.0f);
    mask = _mm512_cmp_ps_mask(va, threshold, _CMP_GT_OQ);
    
    /* Critical blend operation for V16SFmode */
    vresult = _mm512_mask_blend_ps(mask, vb, va);
    
    _mm512_storeu_ps(result, vresult);
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += (int)result[i];
    }
    return sum;
}

/* ==================== V8DFmode (8-double-precision floats) ==================== */
static int test_v8df_blend(void) {
    double a[8], b[8], result[8];
    __m512d va, vb, vresult;
    __mmask8 mask;
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.5;
        b[i] = i * 2.5;
    }
    
    va = _mm512_loadu_pd(a);
    vb = _mm512_loadu_pd(b);
    
    /* Generate mask: select a where a > 4.0 */
    __m512d threshold = _mm512_set1_pd(4.0);
    mask = _mm512_cmp_pd_mask(va, threshold, _CMP_GT_OQ);
    
    /* Critical blend operation for V8DFmode */
    vresult = _mm512_mask_blend_pd(mask, vb, va);
    
    _mm512_storeu_pd(result, vresult);
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (int)result[i];
    }
    return sum;
}

/* ==================== V16SImode (16-dword integers) ==================== */
static int test_v16si_blend(void) {
    int a[16], b[16], result[16];
    __m512i va, vb, vresult;
    __mmask16 mask;
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 10;
        b[i] = i * 20;
    }
    
    va = _mm512_loadu_si512((__m512i*)a);
    vb = _mm512_loadu_si512((__m512i*)b);
    
    /* Generate mask: select a where a > 80 */
    __m512i threshold = _mm512_set1_epi32(80);
    mask = _mm512_cmpgt_epi32_mask(va, threshold);
    
    /* Critical blend operation for V16SImode */
    vresult = _mm512_mask_blend_epi32(mask, vb, va);
    
    _mm512_storeu_si512((__m512i*)result, vresult);
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== V8DImode (8-qword integers) ==================== */
static int test_v8di_blend(void) {
    long long a[8], b[8], result[8];
    __m512i va, vb, vresult;
    __mmask8 mask;
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 100LL;
        b[i] = i * 200LL;
    }
    
    va = _mm512_loadu_si512((__m512i*)a);
    vb = _mm512_loadu_si512((__m512i*)b);
    
    /* Generate mask: select a where a > 400 */
    __m512i threshold = _mm512_set1_epi64(400);
    mask = _mm512_cmpgt_epi64_mask(va, threshold);
    
    /* Critical blend operation for V8DImode */
    vresult = _mm512_mask_blend_epi64(mask, vb, va);
    
    _mm512_storeu_si512((__m512i*)result, vresult);
    
    long long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return (int)sum;
}

/* ==================== Mixed Data Types in Loop ==================== */
static int test_mixed_blend_loop(void) {
    const int N = 1024;
    float fa[N], fb[N], fr[N];
    double da[N], db[N], dr[N];
    int ia[N], ib[N], ir[N];
    short sa[N], sb[N], sr[N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        fa[i] = i * 0.1f;
        fb[i] = i * 0.2f;
        da[i] = i * 0.3;
        db[i] = i * 0.4;
        ia[i] = i * 5;
        ib[i] = i * 10;
        sa[i] = i * 2;
        sb[i] = i * 3;
    }
    
    int total_sum = 0;
    
    /* Process in chunks of vector size */
    for (int i = 0; i < N; i += 16) {
        /* Float blend (V16SFmode) */
        __m512 vfa = _mm512_loadu_ps(&fa[i]);
        __m512 vfb = _mm512_loadu_ps(&fb[i]);
        __mmask16 fmask = _mm512_cmp_ps_mask(vfa, _mm512_set1_ps(50.0f), _CMP_GT_OQ);
        __m512 vfr = _mm512_mask_blend_ps(fmask, vfb, vfa);
        _mm512_storeu_ps(&fr[i], vfr);
        
        /* Integer blend (V16SImode) */
        __m512i via = _mm512_loadu_si512((__m512i*)&ia[i]);
        __m512i vib = _mm512_loadu_si512((__m512i*)&ib[i]);
        __mmask16 imask = _mm512_cmpgt_epi32_mask(via, _mm512_set1_epi32(250));
        __m512i vir = _mm512_mask_blend_epi32(imask, vib, via);
        _mm512_storeu_si512((__m512i*)&ir[i], vir);
    }
    
    for (int i = 0; i < N; i += 8) {
        /* Double blend (V8DFmode) */
        __m512d vda = _mm512_loadu_pd(&da[i]);
        __m512d vdb = _mm512_loadu_pd(&db[i]);
        __mmask8 dmask = _mm512_cmp_pd_mask(vda, _mm512_set1_pd(150.0), _CMP_GT_OQ);
        __m512d vdr = _mm512_mask_blend_pd(dmask, vdb, vda);
        _mm512_storeu_pd(&dr[i], vdr);
    }
    
    for (int i = 0; i < N; i += 32) {
        /* Short blend (V32HImode) */
        __m512i vsa = _mm512_loadu_si512((__m512i*)&sa[i]);
        __m512i vsb = _mm512_loadu_si512((__m512i*)&sb[i]);
        __mmask32 smask = _mm512_cmpgt_epi16_mask(vsa, _mm512_set1_epi16(100));
        __m512i vsr = _mm512_mask_blend_epi16(smask, vsb, vsa);
        _mm512_storeu_si512((__m512i*)&sr[i], vsr);
    }
    
    /* Compute final checksum */
    for (int i = 0; i < N; i++) {
        total_sum += (int)fr[i] + (int)dr[i] + ir[i] + sr[i];
    }
    
    return total_sum;
}

#endif /* __AVX512BW__ */
#endif /* __AVX512F__ */

/* ==================== Scalar Fallbacks ==================== */
static int scalar_test_v64qi_blend(void) {
    char a[64], b[64], result[64];
    
    for (int i = 0; i < 64; i++) {
        a[i] = i;
        b[i] = 64 - i;
        result[i] = (i % 2 == 0) ? a[i] : b[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += result[i];
    }
    return sum;
}

static int scalar_test_v16sf_blend(void) {
    float a[16], b[16], result[16];
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 1.1f;
        b[i] = i * 2.2f;
        result[i] = (a[i] > 8.0f) ? a[i] : b[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += (int)result[i];
    }
    return sum;
}

/* ==================== Main Function ==================== */
int main(void) {
    int total_checksum = 0;
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("AVX-512 support detected. Running vectorized tests...\n");
    
    total_checksum += test_v64qi_blend();
    total_checksum += test_v32hi_blend();
    
#ifdef __AVX512FP16__
    total_checksum += test_v32hf_blend();
#endif
    
#ifdef __AVX512BF16__
    total_checksum += test_v32bf_blend();
#endif
    
    total_checksum += test_v16sf_blend();
    total_checksum += test_v8df_blend();
    total_checksum += test_v16si_blend();
    total_checksum += test_v8di_blend();
    total_checksum += test_mixed_blend_loop();
    
#else
    printf("AVX-512BW not available. Using scalar fallbacks...\n");
    total_checksum += scalar_test_v64qi_blend();
    total_checksum += scalar_test_v16sf_blend();
#endif
#else
    printf("AVX-512 not available. Using scalar fallbacks...\n");
    total_checksum += scalar_test_v64qi_blend();
    total_checksum += scalar_test_v16sf_blend();
#endif
    
    printf("Total checksum: %d\n", total_checksum);
    return total_checksum % 256;
}
