/* test_condition_codes.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <string.h>

/* Global volatile checksum to prevent optimization */
static volatile unsigned long g_checksum = 0;

/* Helper to mix bits into checksum */
static void update_checksum(unsigned long val) {
    g_checksum = (g_checksum * 131 + val) & 0xFFFFFFFFUL;
}

/* ========== Scalar floating-point tests ========== */
__attribute__((optimize("O0")))
void test_scalar_conditions(float f1, float f2, double d1, double d2) {
    volatile int res;
    
    /* UNORDERED (unord) - using isunordered macro */
    res = isunordered(f1, f2);
    update_checksum(res);
    
    /* ORDERED (ord) - using !isunordered */
    res = !isunordered(d1, d2);
    update_checksum(res);
    
    /* UNEQ (ueq) - unordered or equal */
    res = (f1 != f1) || (f2 != f2) || (f1 == f2);
    update_checksum(res);
    
    /* UNGE (nlt) - unordered or not less than */
    res = isunordered(f1, f2) || !(f1 < f2);
    update_checksum(res);
    
    /* UNGT (nle) - unordered or not less than or equal */
    res = isunordered(d1, d2) || !(d1 <= d2);
    update_checksum(res);
    
    /* UNLE (ule) - unordered or less than or equal */
    res = isunordered(f1, f2) || (f1 <= f2);
    update_checksum(res);
    
    /* UNLT (ult) - unordered or less than */
    res = isunordered(d1, d2) || (d1 < d2);
    update_checksum(res);
    
    /* LTGT (une) - less than or greater than (ordered and not equal) */
    res = !isunordered(f1, f2) && (f1 != f2);
    update_checksum(res);
}

/* ========== Vector SSE tests ========== */
__attribute__((target("sse2"), optimize("O2")))
void test_vector_conditions(__m128 v1, __m128 v2, __m128d d1, __m128d d2) {
    __m128 res_ps;
    __m128d res_pd;
    volatile float store_f[4];
    volatile double store_d[2];
    
    /* UNORDERED - _CMP_UNORD_Q */
    res_ps = _mm_cmp_ps(v1, v2, _CMP_UNORD_Q);
    _mm_storeu_ps((float*)store_f, res_ps);
    update_checksum(*(unsigned*)&store_f[0]);
    
    /* ORDERED - _CMP_ORD_Q */
    res_ps = _mm_cmp_ps(v1, v2, _CMP_ORD_Q);
    _mm_storeu_ps((float*)store_f, res_ps);
    update_checksum(*(unsigned*)&store_f[1]);
    
    /* UNEQ - _CMP_EQ_UQ */
    res_pd = _mm_cmp_pd(d1, d2, _CMP_EQ_UQ);
    _mm_storeu_pd((double*)store_d, res_pd);
    update_checksum(*(unsigned long*)&store_d[0]);
    
    /* UNGE - _CMP_NLT_UQ */
    res_ps = _mm_cmp_ps(v1, v2, _CMP_NLT_UQ);
    _mm_storeu_ps((float*)store_f, res_ps);
    update_checksum(*(unsigned*)&store_f[2]);
    
    /* UNGT - _CMP_NLE_UQ */
    res_pd = _mm_cmp_pd(d1, d2, _CMP_NLE_UQ);
    _mm_storeu_pd((double*)store_d, res_pd);
    update_checksum(*(unsigned long*)&store_d[1]);
    
    /* UNLE - _CMP_LE_UQ */
    res_ps = _mm_cmp_ps(v1, v2, _CMP_LE_UQ);
    _mm_storeu_ps((float*)store_f, res_ps);
    update_checksum(*(unsigned*)&store_f[3]);
    
    /* UNLT - _CMP_LT_UQ */
    res_pd = _mm_cmp_pd(d1, d2, _CMP_LT_UQ);
    _mm_storeu_pd((double*)store_d, res_pd);
    update_checksum(*(unsigned long*)&store_d[0] ^ *(unsigned long*)&store_d[1]);
    
    /* LTGT - _CMP_NEQ_OQ */
    res_ps = _mm_cmp_ps(v1, v2, _CMP_NEQ_OQ);
    _mm_storeu_ps((float*)store_f, res_ps);
    update_checksum(*(unsigned*)&store_f[0] + *(unsigned*)&store_f[1]);
}

/* ========== Inline assembly tests ========== */
__attribute__((optimize("O1")))
void test_inline_asm_conditions(float f1, float f2, double d1, double d2) {
    volatile int result_int = 0;
    volatile double result_double = 0.0;
    volatile __m128d vres;
    __m128d va = _mm_set_pd(d1, d2);
    __m128d vb = _mm_set_pd(d2, d1);
    
    /* Test each condition code mnemonic in inline assembly */
    /* Using both AT&T and Intel syntax patterns via %{...|...} */
    
    /* unord */
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "set{%0|unord} %0"
        : "=r"(result_int)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    update_checksum(result_int);
    
    /* ord */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set{%0|ord} %0"
        : "=r"(result_int)
        : "x"(d1), "x"(d2)
        : "cc"
    );
    update_checksum(result_int);
    
    /* ueq */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|ueq}\n\t"
        "movq %1, %0"
        : "=m"(result_double)
        : "x"(vres), "x"(va), "x"(vb)
        : "memory"
    );
    update_checksum(*(unsigned long*)&result_double);
    
    /* nlt */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|nlt}\n\t"
        "movq %1, %0"
        : "=m"(result_double)
        : "x"(vres), "x"(va), "x"(vb)
        : "memory"
    );
    update_checksum(*(unsigned long*)&result_double);
    
    /* nle */
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "set{%0|nle} %0"
        : "=r"(result_int)
        : "x"(f1), "x"(f2)
        : "cc"
    );
    update_checksum(result_int);
    
    /* ule */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|ule}\n\t"
        "movq %1, %0"
        : "=m"(result_double)
        : "x"(vres), "x"(va), "x"(vb)
        : "memory"
    );
    update_checksum(*(unsigned long*)&result_double);
    
    /* ult */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set{%0|ult} %0"
        : "=r"(result_int)
        : "x"(d1), "x"(d2)
        : "cc"
    );
    update_checksum(result_int);
    
    /* une */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|une}\n\t"
        "movq %1, %0"
        : "=m"(result_double)
        : "x"(vres), "x"(va), "x"(vb)
        : "memory"
    );
    update_checksum(*(unsigned long*)&result_double);
}

/* ========== Complex branching tests ========== */
__attribute__((optimize("O3"), noinline))
void test_branching_conditions(float f1, float f2, double d1, double d2) {
    volatile int counter = 0;
    volatile float fv = f1;
    volatile double dv = d1;
    
    /* Complex nested ifs with uncommon conditions */
    for (int i = 0; i < 3; i++) {
        /* UNORDERED branch */
        if (isunordered(fv, f2)) {
            counter += 1;
            fv += 1.0f;
        }
        
        /* ORDERED branch */
        if (!isunordered(dv, d2)) {
            counter += 2;
            dv *= 1.1;
        }
        
        /* UNEQ branch */
        if (isunordered(fv, f2) || fv == f2) {
            counter += 4;
            fv = f2 + (float)i;
        }
        
        /* UNGE branch */
        if (isunordered(dv, d2) || !(dv < d2)) {
            counter += 8;
            dv = d2 - (double)i;
        }
        
        /* UNGT branch */
        if (isunordered(fv, f2) || !(fv <= f2)) {
            counter += 16;
            fv = f2 * (float)(i + 1);
        }
        
        /* UNLE branch */
        if (isunordered(dv, d2) || dv <= d2) {
            counter += 32;
            dv = d2 / (double)(i + 2);
        }
        
        /* UNLT branch */
        if (isunordered(fv, f2) || fv < f2) {
            counter += 64;
            fv = f2 - (float)(i * 2);
        }
        
        /* LTGT branch */
        if (!isunordered(dv, d2) && dv != d2) {
            counter += 128;
            dv = d2 + (double)(i * 3);
        }
    }
    
    update_checksum(counter);
    update_checksum(*(unsigned*)&fv);
    update_checksum(*(unsigned long*)&dv);
}

/* ========== Main driver ========== */
int main(int argc, char *argv[]) {
    /* Initialize with non-uniform, non-special values */
    unsigned seed = (argc > 1) ? (unsigned)atoi(argv[1]) : 12345;
    srand(seed);
    
    /* Create test data with NaNs, normals, and special values */
    float farr[4];
    double darr[4];
    
    for (int i = 0; i < 4; i++) {
        farr[i] = (rand() % 1000) / 100.0f - 5.0f;
        darr[i] = (rand() % 1000) / 100.0 - 5.0;
    }
    
    /* Insert some NaN values to trigger unordered conditions */
    farr[1] = 0.0f / 0.0f;  /* NaN */
    darr[2] = 0.0 / 0.0;    /* NaN */
    
    /* Test scalar conditions */
    test_scalar_conditions(farr[0], farr[1], darr[0], darr[1]);
    test_scalar_conditions(farr[2], farr[3], darr[2], darr[3]);
    
    /* Test vector conditions */
    __m128 v1 = _mm_set_ps(farr[3], farr[2], farr[1], farr[0]);
    __m128 v2 = _mm_set_ps(farr[0], farr[1], farr[2], farr[3]);
    __m128d d1 = _mm_set_pd(darr[1], darr[0]);
    __m128d d2 = _mm_set_pd(darr[3], darr[2]);
    test_vector_conditions(v1, v2, d1, d2);
    
    /* Test inline assembly */
    test_inline_asm_conditions(farr[0], farr[2], darr[0], darr[2]);
    
    /* Test complex branching */
    test_branching_conditions(farr[1], farr[0], darr[1], darr[0]);
    test_branching_conditions(farr[3], farr[2], darr[3], darr[2]);
    
    /* Print final checksum */
    printf("Final checksum: %lu\n", (unsigned long)g_checksum);
    
    return 0;
}
