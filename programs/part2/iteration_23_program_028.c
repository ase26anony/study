/* Test program for x86 condition code mnemonics coverage */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Global volatile checksum to prevent optimization */
volatile unsigned long g_checksum = 0;

/* Test scalar floating-point conditions */
__attribute__((optimize("O0")))
void test_scalar_conditions(double a, double b, float fa, float fb) {
    volatile int result;
    
    /* Test UNORDERED (unord) */
    result = isunordered(a, b);
    g_checksum += result;
    
    /* Test ORDERED (ord) */
    result = !isunordered(a, b);
    g_checksum += result;
    
    /* Test UNEQ (ueq) - unordered or equal */
    result = (isunordered(a, b) || (a == b));
    g_checksum += result;
    
    /* Test UNGE (nlt) - unordered or greater-or-equal */
    result = (isunordered(a, b) || (a >= b));
    g_checksum += result;
    
    /* Test UNGT (nle) - unordered or greater */
    result = (isunordered(a, b) || (a > b));
    g_checksum += result;
    
    /* Test UNLE (ule) - unordered or less-or-equal */
    result = (isunordered(a, b) || (a <= b));
    g_checksum += result;
    
    /* Test UNLT (ult) - unordered or less */
    result = (isunordered(a, b) || (a < b));
    g_checksum += result;
    
    /* Test LTGT (une) - less or greater (ordered and not equal) */
    result = (!isunordered(a, b) && (a != b));
    g_checksum += result;
    
    /* Complex branching to force condition code generation */
    volatile double x = a;
    volatile double y = b;
    
    if (isunordered(x, y)) {
        g_checksum += 1;
    } else if (x > y && !isunordered(x, y)) {
        g_checksum += 2;
    } else if (x < y && !isunordered(x, y)) {
        g_checksum += 3;
    } else if (x == y && !isunordered(x, y)) {
        g_checksum += 4;
    }
}

/* Test with SSE2 vector operations */
__attribute__((target("sse2"), optimize("O2")))
void test_vector_conditions(__m128d va, __m128d vb, __m128 vfa, __m128 vfb) {
    __m128d cmp_result;
    __m128 cmp_resultf;
    volatile double dresult[2];
    volatile float fresult[4];
    
    /* Test UNORDERED for vectors */
    cmp_result = _mm_cmp_pd(va, vb, _CMP_UNORD_Q);
    _mm_storeu_pd((double*)dresult, cmp_result);
    g_checksum += (int)dresult[0] + (int)dresult[1];
    
    /* Test ORDERED for vectors */
    cmp_result = _mm_cmp_pd(va, vb, _CMP_ORD_Q);
    _mm_storeu_pd((double*)dresult, cmp_result);
    g_checksum += (int)dresult[0] + (int)dresult[1];
    
    /* Test UNEQ for vectors */
    cmp_result = _mm_cmp_pd(va, vb, _CMP_EQ_UQ);
    _mm_storeu_pd((double*)dresult, cmp_result);
    g_checksum += (int)dresult[0] + (int)dresult[1];
    
    /* Test UNGE for vectors */
    cmp_result = _mm_cmp_pd(va, vb, _CMP_GE_UQ);
    _mm_storeu_pd((double*)dresult, cmp_result);
    g_checksum += (int)dresult[0] + (int)dresult[1];
    
    /* Test UNGT for vectors */
    cmp_result = _mm_cmp_pd(va, vb, _CMP_GT_UQ);
    _mm_storeu_pd((double*)dresult, cmp_result);
    g_checksum += (int)dresult[0] + (int)dresult[1];
    
    /* Test UNLE for vectors */
    cmp_result = _mm_cmp_pd(va, vb, _CMP_LE_UQ);
    _mm_storeu_pd((double*)dresult, cmp_result);
    g_checksum += (int)dresult[0] + (int)dresult[1];
    
    /* Test UNLT for vectors */
    cmp_result = _mm_cmp_pd(va, vb, _CMP_LT_UQ);
    _mm_storeu_pd((double*)dresult, cmp_result);
    g_checksum += (int)dresult[0] + (int)dresult[1];
    
    /* Test LTGT for vectors */
    cmp_result = _mm_cmp_pd(va, vb, _CMP_NEQ_OQ);
    _mm_storeu_pd((double*)dresult, cmp_result);
    g_checksum += (int)dresult[0] + (int)dresult[1];
    
    /* Test float vectors as well */
    cmp_resultf = _mm_cmp_ps(vfa, vfb, _CMP_UNORD_Q);
    _mm_storeu_ps((float*)fresult, cmp_resultf);
    g_checksum += (int)fresult[0] + (int)fresult[1] + (int)fresult[2] + (int)fresult[3];
    
    cmp_resultf = _mm_cmp_ps(vfa, vfb, _CMP_ORD_Q);
    _mm_storeu_ps((float*)fresult, cmp_resultf);
    g_checksum += (int)fresult[0] + (int)fresult[1] + (int)fresult[2] + (int)fresult[3];
}

/* Test inline assembly with condition codes */
__attribute__((optimize("O1")))
void test_inline_asm_conditions(double a, double b) {
    volatile double result;
    volatile int cc_result;
    
    /* Test UNORDERED (unord) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setp %b0\n\t"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    g_checksum += cc_result;
    
    /* Test ORDERED (ord) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnp %b0\n\t"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    g_checksum += cc_result;
    
    /* Test UNEQ (ueq) using cmppd */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|ueq}\n\t"
        "movq %1, %0\n\t"
        : "=m"(result)
        : "x"(_mm_setzero_pd()), "x"(_mm_set1_pd(a)), "x"(_mm_set1_pd(b))
        : "memory"
    );
    g_checksum += (int)result;
    
    /* Test UNGE (nlt) */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|nlt}\n\t"
        "movq %1, %0\n\t"
        : "=m"(result)
        : "x"(_mm_setzero_pd()), "x"(_mm_set1_pd(a)), "x"(_mm_set1_pd(b))
        : "memory"
    );
    g_checksum += (int)result;
    
    /* Test UNGT (nle) */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|nle}\n\t"
        "movq %1, %0\n\t"
        : "=m"(result)
        : "x"(_mm_setzero_pd()), "x"(_mm_set1_pd(a)), "x"(_mm_set1_pd(b))
        : "memory"
    );
    g_checksum += (int)result;
    
    /* Test UNLE (ule) */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|ule}\n\t"
        "movq %1, %0\n\t"
        : "=m"(result)
        : "x"(_mm_setzero_pd()), "x"(_mm_set1_pd(a)), "x"(_mm_set1_pd(b))
        : "memory"
    );
    g_checksum += (int)result;
    
    /* Test UNLT (ult) */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|ult}\n\t"
        "movq %1, %0\n\t"
        : "=m"(result)
        : "x"(_mm_setzero_pd()), "x"(_mm_set1_pd(a)), "x"(_mm_set1_pd(b))
        : "memory"
    );
    g_checksum += (int)result;
    
    /* Test LTGT (une) */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|une}\n\t"
        "movq %1, %0\n\t"
        : "=m"(result)
        : "x"(_mm_setzero_pd()), "x"(_mm_set1_pd(a)), "x"(_mm_set1_pd(b))
        : "memory"
    );
    g_checksum += (int)result;
}

/* Test with mixed optimization levels */
__attribute__((optimize("O3"), target("avx")))
void test_avx_conditions(__m256d va, __m256d vb) {
    __m256d cmp_result;
    volatile double dresult[4];
    
    /* Test various conditions with AVX */
    cmp_result = _mm256_cmp_pd(va, vb, _CMP_UNORD_Q);
    _mm256_storeu_pd(dresult, cmp_result);
    g_checksum += (int)dresult[0] + (int)dresult[1] + (int)dresult[2] + (int)dresult[3];
    
    cmp_result = _mm256_cmp_pd(va, vb, _CMP_ORD_Q);
    _mm256_storeu_pd(dresult, cmp_result);
    g_checksum += (int)dresult[0] + (int)dresult[1] + (int)dresult[2] + (int)dresult[3];
    
    cmp_result = _mm256_cmp_pd(va, vb, _CMP_EQ_UQ);
    _mm256_storeu_pd(dresult, cmp_result);
    g_checksum += (int)dresult[0] + (int)dresult[1] + (int)dresult[2] + (int)dresult[3];
    
    cmp_result = _mm256_cmp_pd(va, vb, _CMP_GE_UQ);
    _mm256_storeu_pd(dresult, cmp_result);
    g_checksum += (int)dresult[0] + (int)dresult[1] + (int)dresult[2] + (int)dresult[3];
}

int main(int argc, char *argv[]) {
    /* Initialize with non-uniform values */
    unsigned long seed = (argc > 1) ? atoi(argv[1]) : 12345;
    srand(seed);
    
    /* Create test data with special values (NaN, infinity, normal numbers) */
    double a = 1.5 + (rand() % 100) * 0.01;
    double b = 2.5 + (rand() % 100) * 0.01;
    double nan_val = 0.0 / 0.0;  /* Generate NaN */
    double inf_val = 1.0 / 0.0;  /* Generate infinity */
    
    float fa = 1.5f + (rand() % 100) * 0.01f;
    float fb = 2.5f + (rand() % 100) * 0.01f;
    
    /* Test with normal numbers */
    printf("Testing with normal numbers...\n");
    test_scalar_conditions(a, b, fa, fb);
    
    /* Test with NaN (triggers unordered conditions) */
    printf("Testing with NaN...\n");
    test_scalar_conditions(nan_val, b, nan_val, fb);
    
    /* Test vector conditions */
    __m128d va = _mm_set_pd(a, nan_val);
    __m128d vb = _mm_set_pd(b, inf_val);
    __m128 vfa = _mm_set_ps(fa, fb, nan_val, fa);
    __m128 vfb = _mm_set_ps(fb, fa, fb, nan_val);
    
    test_vector_conditions(va, vb, vfa, vfb);
    
    /* Test inline assembly */
    test_inline_asm_conditions(a, b);
    test_inline_asm_conditions(nan_val, b);
    
    /* Test AVX if available */
    #ifdef __AVX__
    __m256d va256 = _mm256_set_pd(a, b, nan_val, inf_val);
    __m256d vb256 = _mm256_set_pd(b, a, inf_val, nan_val);
    test_avx_conditions(va256, vb256);
    #endif
    
    /* Print final checksum */
    printf("Final checksum: %lu\n", g_checksum);
    
    return 0;
}
