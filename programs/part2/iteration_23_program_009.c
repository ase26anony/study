/* Test program for x86 condition code mnemonics coverage */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Volatile variables to prevent optimization */
volatile double vd1, vd2, vd3, vd4;
volatile float vf1, vf2, vf3, vf4;
volatile int vi1, vi2, vi3, vi4;
volatile unsigned long long checksum = 0;

/* Function with different optimization attributes */
__attribute__((optimize("O0"), target("sse2")))
void test_scalar_conditions_O0(double d1, double d2, float f1, float f2) {
    /* Test UNORDERED (unord) */
    if (isunordered(d1, d2)) {
        vi1 = 1;
        checksum += 1;
    }
    
    /* Test ORDERED (ord) */
    if (!isunordered(d1, d2)) {
        vi2 = 2;
        checksum += 2;
    }
    
    /* Test UNEQ (ueq) - unordered or equal */
    if (isunordered(f1, f2) || f1 == f2) {
        vi3 = 3;
        checksum += 3;
    }
    
    /* Test UNGE (nlt) - unordered or not less than */
    if (isunordered(d1, d2) || d1 >= d2) {
        vi4 = 4;
        checksum += 4;
    }
    
    /* Test UNGT (nle) - unordered or not less than or equal */
    if (isunordered(d1, d2) || d1 > d2) {
        vi1 = 5;
        checksum += 5;
    }
    
    /* Test UNLE (ule) - unordered or less than or equal */
    if (isunordered(f1, f2) || f1 <= f2) {
        vi2 = 6;
        checksum += 6;
    }
    
    /* Test UNLT (ult) - unordered or less than */
    if (isunordered(f1, f2) || f1 < f2) {
        vi3 = 7;
        checksum += 7;
    }
    
    /* Test LTGT (une) - less than or greater than (ordered and not equal) */
    if ((!isunordered(d1, d2)) && d1 != d2) {
        vi4 = 8;
        checksum += 8;
    }
}

__attribute__((optimize("O2"), target("sse2")))
void test_scalar_conditions_O2(double d1, double d2, float f1, float f2) {
    /* Use ternary operators to force conditional moves */
    vi1 = isunordered(d1, d2) ? 100 : 200;
    checksum += vi1;
    
    vi2 = !isunordered(d1, d2) ? 300 : 400;
    checksum += vi2;
    
    vi3 = (isunordered(f1, f2) || f1 == f2) ? 500 : 600;
    checksum += vi3;
    
    vi4 = (isunordered(d1, d2) || d1 >= d2) ? 700 : 800;
    checksum += vi4;
}

__attribute__((optimize("O3"), target("sse2")))
void test_vector_conditions_sse2(__m128d a, __m128d b, __m128 c, __m128 d) {
    __m128d cmp_result;
    __m128 cmp_result_f;
    __m128i mask;
    
    /* Test UNORDERED (unord) with _CMP_UNORD_Q */
    cmp_result = _mm_cmp_pd(a, b, _CMP_UNORD_Q);
    mask = _mm_castpd_si128(cmp_result);
    vi1 = _mm_extract_epi16(mask, 0);
    checksum += vi1;
    
    /* Test ORDERED (ord) with _CMP_ORD_Q */
    cmp_result = _mm_cmp_pd(a, b, _CMP_ORD_Q);
    mask = _mm_castpd_si128(cmp_result);
    vi2 = _mm_extract_epi16(mask, 1);
    checksum += vi2;
    
    /* Test UNEQ (ueq) with _CMP_EQ_UQ */
    cmp_result = _mm_cmp_pd(a, b, _CMP_EQ_UQ);
    mask = _mm_castpd_si128(cmp_result);
    vi3 = _mm_extract_epi16(mask, 2);
    checksum += vi3;
    
    /* Test UNGE (nlt) with _CMP_NLT_UQ */
    cmp_result = _mm_cmp_pd(a, b, _CMP_NLT_UQ);
    mask = _mm_castpd_si128(cmp_result);
    vi4 = _mm_extract_epi16(mask, 3);
    checksum += vi4;
    
    /* Test UNGT (nle) with _CMP_NLE_UQ */
    cmp_result_f = _mm_cmp_ps(c, d, _CMP_NLE_UQ);
    mask = _mm_castps_si128(cmp_result_f);
    vi1 = _mm_extract_epi16(mask, 0);
    checksum += vi1;
    
    /* Test UNLE (ule) with _CMP_LE_UQ */
    cmp_result_f = _mm_cmp_ps(c, d, _CMP_LE_UQ);
    mask = _mm_castps_si128(cmp_result_f);
    vi2 = _mm_extract_epi16(mask, 1);
    checksum += vi2;
    
    /* Test UNLT (ult) with _CMP_LT_UQ */
    cmp_result_f = _mm_cmp_ps(c, d, _CMP_LT_UQ);
    mask = _mm_castps_si128(cmp_result_f);
    vi3 = _mm_extract_epi16(mask, 2);
    checksum += vi3;
    
    /* Test LTGT (une) with _CMP_NEQ_UQ */
    cmp_result = _mm_cmp_pd(a, b, _CMP_NEQ_UQ);
    mask = _mm_castpd_si128(cmp_result);
    vi4 = _mm_extract_epi16(mask, 3);
    checksum += vi4;
}

/* Inline assembly tests with explicit condition codes */
__attribute__((optimize("O1"), target("sse2")))
void test_inline_asm_conditions(double d1, double d2, float f1, float f2) {
    double result_d;
    float result_f;
    int result_i;
    
    /* Test UNORDERED (unord) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result_i)
        : "x" (d1), "x" (d2)
        : "al", "cc"
    );
    checksum += result_i;
    
    /* Test ORDERED (ord) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (result_i)
        : "x" (d1), "x" (d2)
        : "al", "cc"
    );
    checksum += result_i;
    
    /* Test UNEQ (ueq) using cmppd */
    {
        __m128d a = _mm_set_sd(d1);
        __m128d b = _mm_set_sd(d2);
        __m128d res;
        __asm__ volatile (
            "cmppd %3, %2, %{%1|ueq}\n\t"
            "movq %q1, %0"
            : "=m" (result_d)
            : "x" (res), "x" (a), "x" (b)
            : "memory"
        );
        checksum += (unsigned long long)result_d;
    }
    
    /* Test UNGE (nlt) */
    {
        __m128d a = _mm_set_sd(d1);
        __m128d b = _mm_set_sd(d2);
        __m128d res;
        __asm__ volatile (
            "cmppd %3, %2, %{%1|nlt}\n\t"
            "movq %q1, %0"
            : "=m" (result_d)
            : "x" (res), "x" (a), "x" (b)
            : "memory"
        );
        checksum += (unsigned long long)result_d;
    }
    
    /* Test UNGT (nle) */
    {
        __m128 c = _mm_set_ss(f1);
        __m128 d = _mm_set_ss(f2);
        __m128 res;
        __asm__ volatile (
            "cmpps %3, %2, %{%1|nle}\n\t"
            "movss %1, %0"
            : "=m" (result_f)
            : "x" (res), "x" (c), "x" (d)
            : "memory"
        );
        checksum += (unsigned)result_f;
    }
    
    /* Test UNLE (ule) */
    {
        __m128 c = _mm_set_ss(f1);
        __m128 d = _mm_set_ss(f2);
        __m128 res;
        __asm__ volatile (
            "cmpps %3, %2, %{%1|ule}\n\t"
            "movss %1, %0"
            : "=m" (result_f)
            : "x" (res), "x" (c), "x" (d)
            : "memory"
        );
        checksum += (unsigned)result_f;
    }
    
    /* Test UNLT (ult) */
    {
        __m128 c = _mm_set_ss(f1);
        __m128 d = _mm_set_ss(f2);
        __m128 res;
        __asm__ volatile (
            "cmpps %3, %2, %{%1|ult}\n\t"
            "movss %1, %0"
            : "=m" (result_f)
            : "x" (res), "x" (c), "x" (d)
            : "memory"
        );
        checksum += (unsigned)result_f;
    }
    
    /* Test LTGT (une) */
    {
        __m128d a = _mm_set_sd(d1);
        __m128d b = _mm_set_sd(d2);
        __m128d res;
        __asm__ volatile (
            "cmppd %3, %2, %{%1|une}\n\t"
            "movq %q1, %0"
            : "=m" (result_d)
            : "x" (res), "x" (a), "x" (b)
            : "memory"
        );
        checksum += (unsigned long long)result_d;
    }
}

/* Complex branching to force condition code generation */
__attribute__((optimize("O2"), target("sse2")))
void test_complex_branching(double* darray, float* farray, int n) {
    int i;
    volatile int count_unord = 0, count_ord = 0, count_ueq = 0;
    volatile int count_nlt = 0, count_nle = 0, count_ule = 0;
    volatile int count_ult = 0, count_une = 0;
    
    for (i = 0; i < n - 1; i++) {
        /* UNORDERED */
        if (isunordered(darray[i], darray[i+1])) {
            count_unord++;
            checksum += i * 10;
        }
        
        /* ORDERED */
        if (!isunordered(darray[i], darray[i+1])) {
            count_ord++;
            checksum += i * 20;
        }
        
        /* UNEQ */
        if (isunordered(farray[i], farray[i+1]) || farray[i] == farray[i+1]) {
            count_ueq++;
            checksum += i * 30;
        }
        
        /* UNGE (nlt) */
        if (isunordered(darray[i], darray[i+1]) || darray[i] >= darray[i+1]) {
            count_nlt++;
            checksum += i * 40;
        }
        
        /* UNGT (nle) */
        if (isunordered(darray[i], darray[i+1]) || darray[i] > darray[i+1]) {
            count_nle++;
            checksum += i * 50;
        }
        
        /* UNLE (ule) */
        if (isunordered(farray[i], farray[i+1]) || farray[i] <= farray[i+1]) {
            count_ule++;
            checksum += i * 60;
        }
        
        /* UNLT (ult) */
        if (isunordered(farray[i], farray[i+1]) || farray[i] < farray[i+1]) {
            count_ult++;
            checksum += i * 70;
        }
        
        /* LTGT (une) */
        if ((!isunordered(darray[i], darray[i+1])) && darray[i] != darray[i+1]) {
            count_une++;
            checksum += i * 80;
        }
    }
    
    /* Use results to prevent dead code elimination */
    checksum += count_unord + count_ord + count_ueq + count_nlt +
                count_nle + count_ule + count_ult + count_une;
}

int main(int argc, char* argv[]) {
    /* Initialize with non-uniform values */
    unsigned seed = (argc > 1) ? (unsigned)atoi(argv[1]) : 12345;
    srand(seed);
    
    /* Create arrays with mixed values including NaN, infinity, normal numbers */
    double darray[20];
    float farray[20];
    
    for (int i = 0; i < 20; i++) {
        switch (i % 7) {
            case 0: darray[i] = 1.0 / (i + 1); break;
            case 1: darray[i] = (double)(i * i); break;
            case 2: darray[i] = -3.14159 * i; break;
            case 3: darray[i] = 0.0; break;
            case 4: darray[i] = -0.0; break;
            case 5: darray[i] = (i % 3 == 0) ? 0.0 / 0.0 : darray[i-1]; /* NaN */ break;
            case 6: darray[i] = 1.0 / 0.0; /* Inf */ break;
        }
        farray[i] = (float)darray[i];
    }
    
    /* Initialize volatile variables */
    vd1 = darray[0]; vd2 = darray[1]; vd3 = darray[2]; vd4 = darray[3];
    vf1 = farray[4]; vf2 = farray[5]; vf3 = farray[6]; vf4 = farray[7];
    
    /* Test scalar conditions with different optimization levels */
    test_scalar_conditions_O0(vd1, vd2, vf1, vf2);
    test_scalar_conditions_O2(vd3, vd4, vf3, vf4);
    
    /* Test vector conditions */
    __m128d vec_d1 = _mm_set_pd(darray[8], darray[9]);
    __m128d vec_d2 = _mm_set_pd(darray[10], darray[11]);
    __m128 vec_f1 = _mm_set_ps(farray[12], farray[13], farray[14], farray[15]);
    __m128 vec_f2 = _mm_set_ps(farray[16], farray[17], farray[18], farray[19]);
    test_vector_conditions_sse2(vec_d1, vec_d2, vec_f1, vec_f2);
    
    /* Test inline assembly */
    test_inline_asm_conditions(vd1, vd2, vf1, vf2);
    
    /* Test complex branching */
    test_complex_branching(darray, farray, 20);
    
    /* Print final checksum */
    printf("Final checksum: %llu\n", checksum);
    
    return 0;
}
