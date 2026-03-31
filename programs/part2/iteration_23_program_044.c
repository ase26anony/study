/* Test program to cover x86 condition code mnemonics in i386.cc */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Volatile variables to prevent optimization */
volatile int global_checksum = 0;
volatile int volatile_control = 1;

/* Function with different optimization levels */
__attribute__((optimize("O0")))
void test_scalar_conditions_O0(double a, double b, float fa, float fb) {
    volatile int result = 0;
    
    /* Test UNORDERED (unord) */
    if (isunordered(a, b)) {
        result |= 1;
    }
    
    /* Test ORDERED (ord) */
    if (!isunordered(a, b)) {
        result |= 2;
    }
    
    /* Test UNEQ (ueq) - unordered or equal */
    if (isunordered(a, b) || a == b) {
        result |= 4;
    }
    
    /* Test UNGE (nlt) - unordered or not less than */
    if (isunordered(a, b) || !(a < b)) {
        result |= 8;
    }
    
    /* Test UNGT (nle) - unordered or not less than or equal */
    if (isunordered(a, b) || !(a <= b)) {
        result |= 16;
    }
    
    /* Test UNLE (ule) - unordered or less than or equal */
    if (isunordered(a, b) || a <= b) {
        result |= 32;
    }
    
    /* Test UNLT (ult) - unordered or less than */
    if (isunordered(a, b) || a < b) {
        result |= 64;
    }
    
    /* Test LTGT (une) - less than or greater than (ordered and not equal) */
    if (!isunordered(a, b) && a != b) {
        result |= 128;
    }
    
    global_checksum += result;
}

__attribute__((optimize("O2"), target("sse2")))
void test_vector_conditions_sse2(__m128d va, __m128d vb, __m128 vfa, __m128 vfb) {
    volatile __m128d mask_result;
    volatile __m128 mask_resultf;
    
    /* Test UNORDERED (unord) - _CMP_UNORD_Q */
    mask_result = _mm_cmp_pd(va, vb, _CMP_UNORD_Q);
    global_checksum += _mm_movemask_pd(mask_result);
    
    /* Test ORDERED (ord) - _CMP_ORD_Q */
    mask_result = _mm_cmp_pd(va, vb, _CMP_ORD_Q);
    global_checksum += _mm_movemask_pd(mask_result);
    
    /* Test UNEQ (ueq) - _CMP_EQ_UQ */
    mask_result = _mm_cmp_pd(va, vb, _CMP_EQ_UQ);
    global_checksum += _mm_movemask_pd(mask_result);
    
    /* Test UNGE (nlt) - _CMP_NLT_UQ */
    mask_result = _mm_cmp_pd(va, vb, _CMP_NLT_UQ);
    global_checksum += _mm_movemask_pd(mask_result);
    
    /* Test UNGT (nle) - _CMP_NLE_UQ */
    mask_result = _mm_cmp_pd(va, vb, _CMP_NLE_UQ);
    global_checksum += _mm_movemask_pd(mask_result);
    
    /* Test UNLE (ule) - _CMP_LE_UQ */
    mask_result = _mm_cmp_pd(va, vb, _CMP_LE_UQ);
    global_checksum += _mm_movemask_pd(mask_result);
    
    /* Test UNLT (ult) - _CMP_LT_UQ */
    mask_result = _mm_cmp_pd(va, vb, _CMP_LT_UQ);
    global_checksum += _mm_movemask_pd(mask_result);
    
    /* Test LTGT (une) - _CMP_NEQ_OQ */
    mask_result = _mm_cmp_pd(va, vb, _CMP_NEQ_OQ);
    global_checksum += _mm_movemask_pd(mask_result);
    
    /* Float versions */
    mask_resultf = _mm_cmp_ps(vfa, vfb, _CMP_UNORD_Q);
    global_checksum += _mm_movemask_ps(mask_resultf);
    
    mask_resultf = _mm_cmp_ps(vfa, vfb, _CMP_ORD_Q);
    global_checksum += _mm_movemask_ps(mask_resultf);
}

__attribute__((optimize("O1")))
void test_inline_asm_conditions(double a, double b) {
    volatile double result;
    volatile int cc_result;
    
    /* Test UNORDERED (unord) with inline assembly */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%{p|unord} %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    global_checksum += cc_result;
    
    /* Test ORDERED (ord) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%{np|ord} %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    global_checksum += cc_result;
    
    /* Test UNEQ (ueq) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%{e|ueq} %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    global_checksum += cc_result;
    
    /* Test UNGE (nlt) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%{ae|nlt} %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    global_checksum += cc_result;
    
    /* Test UNGT (nle) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%{a|nle} %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    global_checksum += cc_result;
    
    /* Test UNLE (ule) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%{be|ule} %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    global_checksum += cc_result;
    
    /* Test UNLT (ult) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%{b|ult} %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    global_checksum += cc_result;
    
    /* Test LTGT (une) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%{ne|une} %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    global_checksum += cc_result;
}

__attribute__((optimize("O3"), target("avx")))
void test_avx_conditions(__m256d va, __m256d vb) {
    volatile __m256d mask_result;
    
    /* AVX versions of the comparisons */
    mask_result = _mm256_cmp_pd(va, vb, _CMP_UNORD_Q);
    global_checksum += _mm256_movemask_pd(mask_result);
    
    mask_result = _mm256_cmp_pd(va, vb, _CMP_ORD_Q);
    global_checksum += _mm256_movemask_pd(mask_result);
    
    mask_result = _mm256_cmp_pd(va, vb, _CMP_EQ_UQ);
    global_checksum += _mm256_movemask_pd(mask_result);
    
    mask_result = _mm256_cmp_pd(va, vb, _CMP_NLT_UQ);
    global_checksum += _mm256_movemask_pd(mask_result);
    
    mask_result = _mm256_cmp_pd(va, vb, _CMP_NLE_UQ);
    global_checksum += _mm256_movemask_pd(mask_result);
    
    mask_result = _mm256_cmp_pd(va, vb, _CMP_LE_UQ);
    global_checksum += _mm256_movemask_pd(mask_result);
    
    mask_result = _mm256_cmp_pd(va, vb, _CMP_LT_UQ);
    global_checksum += _mm256_movemask_pd(mask_result);
    
    mask_result = _mm256_cmp_pd(va, vb, _CMP_NEQ_OQ);
    global_checksum += _mm256_movemask_pd(mask_result);
}

void test_complex_branches(double* arr, int n) {
    volatile int i;
    volatile double sum = 0.0;
    
    for (i = 0; i < n; i++) {
        /* Complex branching with multiple conditions */
        if (isunordered(arr[i], arr[(i+1)%n])) {
            sum += 1.0;
        } else if (!isunordered(arr[i], arr[(i+2)%n]) && arr[i] != arr[(i+2)%n]) {
            sum += 2.0;  /* LTGT (une) */
        } else if (isunordered(arr[i], arr[(i+3)%n]) || arr[i] <= arr[(i+3)%n]) {
            sum += 3.0;  /* UNLE (ule) */
        } else if (isunordered(arr[i], arr[(i+4)%n]) || !(arr[i] < arr[(i+4)%n])) {
            sum += 4.0;  /* UNGE (nlt) */
        }
    }
    
    global_checksum += (int)sum;
}

int main(int argc, char** argv) {
    /* Initialize with non-uniform values */
    unsigned int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    srand(seed);
    
    /* Create arrays with mixed values including NaN, infinity, normal numbers */
    double darr[16];
    float farr[16];
    
    for (int i = 0; i < 16; i++) {
        switch (i % 5) {
            case 0: darr[i] = 1.0 / (i + 1); break;
            case 1: darr[i] = -2.0 * i; break;
            case 2: darr[i] = 0.0 / 0.0; break;  /* NaN */
            case 3: darr[i] = 1.0 / 0.0; break;  /* Infinity */
            case 4: darr[i] = -1.0 / 0.0; break; /* -Infinity */
        }
        farr[i] = (float)darr[i];
    }
    
    /* Test scalar conditions with different optimization levels */
    for (int i = 0; i < 8; i++) {
        test_scalar_conditions_O0(darr[i], darr[i+1], farr[i], farr[i+1]);
    }
    
    /* Test vector conditions */
    __m128d vd1 = _mm_set_pd(darr[0], darr[1]);
    __m128d vd2 = _mm_set_pd(darr[2], darr[3]);
    __m128 vf1 = _mm_set_ps(farr[0], farr[1], farr[2], farr[3]);
    __m128 vf2 = _mm_set_ps(farr[4], farr[5], farr[6], farr[7]);
    
    test_vector_conditions_sse2(vd1, vd2, vf1, vf2);
    
    /* Test inline assembly conditions */
    for (int i = 0; i < 4; i++) {
        test_inline_asm_conditions(darr[i*2], darr[i*2+1]);
    }
    
    /* Test AVX conditions if available */
#ifdef __AVX__
    __m256d avx_vd1 = _mm256_set_pd(darr[0], darr[1], darr[2], darr[3]);
    __m256d avx_vd2 = _mm256_set_pd(darr[4], darr[5], darr[6], darr[7]);
    test_avx_conditions(avx_vd1, avx_vd2);
#endif
    
    /* Test complex branching patterns */
    test_complex_branches(darr, 16);
    
    printf("Final checksum: %d\n", global_checksum);
    return global_checksum != 0 ? 0 : 1;
}
